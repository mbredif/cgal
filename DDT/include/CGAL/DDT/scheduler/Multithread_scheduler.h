// Copyright (c) 2022 Institut Géographique National - IGN (France)
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Mathieu Brédif and Laurent Caraffa

#ifndef CGAL_DDT_SCHEDULER_MULTITHREAD_SCHEDULER_H
#define CGAL_DDT_SCHEDULER_MULTITHREAD_SCHEDULER_H

#include <CGAL/DDT/scheduler/multithread_scheduler/thread_pool.h>

namespace CGAL {
namespace DDT {


template< typename V, typename TileContainer, typename UnaryOp, typename Id>
V transform_id(TileContainer& tc, UnaryOp transform, std::mutex& mutex, Id id)
{
    typedef typename TileContainer::Tile Tile;
    Tile& tile = tc.at(id);
    // ensure tile is loaded
    {
        std::unique_lock<std::mutex> lock(mutex);
        tc.lock(tile);
        tc.load(tile);
    }

    // acquire the tile and extract a copy of tc in tc2
    size_t number_of_extreme_points_received1;
    size_t number_of_extreme_points_received2;
    TileContainer tc2(tc.maximal_dimension());

    {
        std::unique_lock<std::mutex> lock(mutex);
        // "swap" moves the incoming points from tc to the tile, which had no points
        tile.points().swap(tc[id].points());
        // moves the unreceived axis extreme points
        number_of_extreme_points_received1 = tc.extreme_points().size();
        tc2.extreme_points().insert(tc2.extreme_points().end(),
                                    tc.extreme_points().begin() + tile.number_of_extreme_points_received,
                                    tc.extreme_points().end());
                                 // tc.extreme_points().begin() + number_of_extreme_points_received1); // == end
    }
    number_of_extreme_points_received2 = number_of_extreme_points_received1 - tile.number_of_extreme_points_received;
    tile.number_of_extreme_points_received = 0;
    // process the tile on the extracted copy tc2
    V value = transform(tc2, tile);
    tile.number_of_extreme_points_received = number_of_extreme_points_received1;

    // unlock the tile and merge the extracted copy tc2 back to tc
    {
        std::unique_lock<std::mutex> lock(mutex);
        tc.unlock(tile);
        // moves the points emitted in tc2 to tc
        for(auto& t : tc2) {
            auto& points = tc[t.id()].points();
            points.insert(points.end(), t.points().begin(), t.points().end());
        }
        // moves the axis extreme points emitted in tc2 to tc
        tc.extreme_points().insert(tc.extreme_points().end(),
                                   tc2.extreme_points().begin() + number_of_extreme_points_received2,
                                   tc2.extreme_points().end());
    }

    return value;
}


/// \ingroup PkgDDTSchedulerClasses
/// \cgalModels Scheduler
template<typename T>
struct Multithread_scheduler
{
    typedef T Tile;
    typedef typename Tile::Id Id;

    /// constructor
    Multithread_scheduler(int max_concurrency = 0) : pool(max_concurrency), timeout_(1)
    {
        pool.init();
    }
    template<class Duration>
    Multithread_scheduler(int max_concurrency, Duration timeout) : pool(max_concurrency), timeout_(timeout)
    {
        pool.init();
    }
    inline int max_concurrency() const
    {
        return pool.max_concurrency();
    }
    ~Multithread_scheduler()
    {
        pool.shutdown();
    }

    template<typename TileContainer,
         typename Transform,
         typename Reduce = std::plus<>,
         typename V = std::invoke_result_t<Reduce,
                                           std::invoke_result_t<Transform, TileContainer&, Tile&>,
                                           std::invoke_result_t<Transform, TileContainer&, Tile&> > >
    V for_each(TileContainer& tc, Transform transform, Reduce reduce = {}, V init = {})
    {
        std::vector<std::future<V>> futures;        
        for(const Tile& tile : tc)
            futures.push_back(pool.submit([this, &tc, &transform](Id id){ return transform_id<V>(tc, transform, mutex, id); }, tile.id()));

        V value = init;
        for(auto& f: futures) value = reduce(value, f.get());
        return value;
    }

    template<typename TileContainer,
         typename Transform,
         typename Reduce = std::plus<>,
         typename V = std::invoke_result_t<Reduce,
                                           std::invoke_result_t<Transform, TileContainer&, Tile&>,
                                           std::invoke_result_t<Transform, TileContainer&, Tile&> > >
    V for_each_rec(TileContainer& tc, Transform transform, Reduce reduce = {}, V init = {})
    {
        std::map<Id, std::future<V>> futures;
        for(const Tile& tile : tc)
            futures.emplace(tile.id(), pool.submit([this, &tc, &transform](Id id){ return transform_id<V>(tc, transform, mutex, id); }, tile.id()));

        V value = init;
        while (!futures.empty()) {
            auto fit = futures.begin();
            while(fit!=futures.end())
            {
                if (fit->second.wait_for(timeout_) != std::future_status::ready) {
                    ++fit;
                } else {
                    value = reduce(value, fit->second.get());
                    futures.erase(fit++);
                    std::vector<Id> ids;
                    {
                        std::unique_lock<std::mutex> lock(mutex);
                        for(const Tile& tile : tc)
                            if (!tile.points().empty())
                                ids.push_back(tile.id());
                    }
                    for(auto id : ids)
                        if (futures.count(id) == 0)
                            futures.emplace(id, pool.submit([this, &tc, &transform](Id id){ return transform_id<V>(tc, transform, mutex, id); }, id));
                }
            }
        }
        return value;
    }

private:
    thread_pool pool;
    std::chrono::milliseconds timeout_;
    std::mutex mutex;
};

}
}

#endif // CGAL_DDT_SCHEDULER_MULTITHREAD_SCHEDULER_H
