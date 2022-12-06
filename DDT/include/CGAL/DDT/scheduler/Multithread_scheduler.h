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
V for_each_function(TileContainer& tc, UnaryOp op1, std::mutex& mutex, Id id)
{
    // ensure tile is loaded
    std::pair<typename TileContainer::Tile_iterator, bool> insertion;
    do {
        std::unique_lock<std::mutex> lock(mutex);
        insertion = tc.insert(id);
    } while (!tc.load(insertion));
    typename TileContainer::Tile_iterator tile = insertion.first;

    // acquire the tile and extract a copy of tc in tc2
    size_t number_of_extreme_points_received1;
    size_t number_of_extreme_points_received2;
    TileContainer tc2(tc.maximal_dimension());
    {
        std::unique_lock<std::mutex> lock(mutex);
        // "swap" moves the incoming points from tc to tc2, which had no points
        tc2.points()[id].swap(tc.points()[id]);

        // moves the unreceived axis extreme points
        tc2.extreme_points().insert(tc2.extreme_points().end(),
                                    tc.extreme_points().begin() + tile->number_of_extreme_points_received,
                                    tc.extreme_points().end());
        number_of_extreme_points_received1 = tc.extreme_points().size();
        number_of_extreme_points_received2 = number_of_extreme_points_received1 - tile->number_of_extreme_points_received;
        tile->number_of_extreme_points_received = 0;
    }

    // process the tile on the extracted copy tc2
    V value = op1(tc2, *tile);

    // release the tile and merge the extracted copy tc2 back to tc
    {
        tile->number_of_extreme_points_received = number_of_extreme_points_received1;
        std::unique_lock<std::mutex> lock(mutex);
        tc.unload(tile);
        // moves the points emitted in tc2 to tc
        for(auto& p : tc2.points()) {
            auto& points = tc.points()[p.first];
            points.insert(points.end(), p.second.begin(), p.second.end());
        }

        // moves the axis extreme points emitted in tc2 to tc
        tc.extreme_points().insert(tc.extreme_points().end(),
                                   tc2.extreme_points().begin() + number_of_extreme_points_received2,
                                   tc2.extreme_points().end());
        for(auto b : tc2.bboxes())
            tc.bboxes()[b.first] += b.second;
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

    template<typename TileContainer, typename UnaryOp, typename V = int, typename BinaryOp = std::plus<>>
    V for_each(TileContainer& tc, UnaryOp op1, BinaryOp op2 = {}, V init = {})
    {
        std::vector<std::future<V>> futures;
        for(auto it = tc.tile_ids_begin(); it != tc.tile_ids_end(); ++it)
            futures.push_back(pool.submit([this, &tc, &op1](Id id){ return for_each_function<V>(tc, op1, mutex, id); }, *it));

        V value = init;
        for(auto& f: futures) value = op2(value, f.get());
        return value;
    }

    template<typename TileContainer, typename UnaryOp, typename V = int, typename BinaryOp = std::plus<>>
    V for_each_rec(TileContainer& tc, UnaryOp op1, BinaryOp op2 = {}, V init = {})
    {
        std::map<Id, std::future<V>> futures;
        for(auto it = tc.tile_ids_begin(); it != tc.tile_ids_end(); ++it)
            futures.emplace(*it, pool.submit([this, &tc, &op1](Id id){ return for_each_function<V>(tc, op1, mutex, id); }, *it));

        V value = init;
        while (!futures.empty()) {
            auto fit = futures.begin();
            while(fit!=futures.end())
            {
                if (fit->second.wait_for(timeout_) != std::future_status::ready) {
                    ++fit;
                } else {
                    value = op2(value, fit->second.get());
                    futures.erase(fit++);
                    std::vector<Id> ids;
                    {
                        std::unique_lock<std::mutex> lock(mutex);
                        for(auto& p : tc.points())
                            if (!p.second.empty())
                                ids.push_back(p.first);
                    }
                    for(auto id : ids)
                        if (futures.count(id) == 0)
                            futures.emplace(id, pool.submit([this, &tc, &op1](Id id){ return for_each_function<V>(tc, op1, mutex, id); }, id));
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
