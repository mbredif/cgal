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
#include <CGAL/DDT/Tile.h>

namespace CGAL {
namespace DDT {

namespace Impl {
template< typename TileContainer, typename Transform, typename V, typename Tile_index >
V transform_id(TileContainer& tiles, Transform transform, V init, Tile_index id, std::mutex& mutex)
{
    typedef typename TileContainer::Tile Tile;
    std::unique_lock<std::mutex> lock(mutex);
    Tile& tile = tiles.emplace(id).first->second;
    tile.locked = true;
    tiles.prepare_load(id, tile);

    lock.unlock();
    V value = (tiles.safe_load(id, tile)) ? transform(tile) : init;

    lock.lock();
    tile.locked = false;
    return value;
}


template< typename TileContainer, typename PointSetContainer, typename Transform, typename V, typename Tile_index >
V transform_zip_id(TileContainer& tiles, PointSetContainer& point_sets, Transform transform, V init, Tile_index id, std::mutex& mutex)
{
    typedef typename TileContainer::Tile Tile;
    std::unique_lock<std::mutex> lock(mutex);
    Tile& tile = tiles.emplace(id).first->second;
    tile.locked = true;
    tiles.prepare_load(id, tile);

    lock.unlock();
    V value = (tiles.safe_load(id, tile)) ? transform(tile, point_sets[id]) : init;

    lock.lock();
    point_sets.send_points(id);
    tile.locked = false;
    return value;
}

}

// \ingroup PkgDDTSchedulerClasses
// \cgalModels Scheduler
struct Multithread_scheduler
{
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
         typename Tile = typename TileContainer::Tile,
         typename T = std::invoke_result_t<Transform, Tile&>,
         typename V = std::invoke_result_t<Reduce, T, T> >
    V for_each(TileContainer& tiles, Transform transform, Reduce reduce = {}, V init_v = {}, T init_t = {})
    {
        typedef typename Tile::value_type::Tile_index Tile_index;
        std::vector<std::future<T>> futures;
        for(const auto& [tid, tile] : tiles)
            futures.push_back(pool.submit([this, &tiles, &transform, &init_t](Tile_index id){
                return Impl::transform_id(tiles, transform, init_t, id, mutex);
            }, tid));
        V value = init_v;
        for(auto& f: futures) value = reduce(value, f.get());
        return value;
    }

    template<typename TileContainer,
             typename PointSetContainer,
             typename Transform,
             typename Reduce = std::plus<>,
             typename Tile = typename TileContainer::Tile,
             typename PointSet = typename PointSetContainer::mapped_type,
             typename T = std::invoke_result_t<Transform, Tile&, PointSet&>,
             typename V = std::invoke_result_t<Reduce, T, T> >
    V for_each_zip(TileContainer& tiles, PointSetContainer& point_sets, Transform transform, Reduce reduce = {}, V init = {})
    {
        typedef typename Tile::value_type::Tile_index Tile_index;
        std::vector<std::future<V>> futures;
        {
            std::unique_lock<std::mutex> lock(mutex);
            for(const auto& [id, point_set] : point_sets)
                futures.push_back(pool.submit([this, &tiles, &point_sets, &transform, &init](Tile_index id){
                    return Impl::transform_zip_id(tiles, point_sets, transform, init, id, mutex);
                }, id));
        }
        V value = init;
        for(auto& f: futures) value = reduce(value, f.get());
        return value;
    }

    template<typename TileContainer,
             typename PointSetContainer,
             typename Transform,
             typename Reduce = std::plus<>,
             typename Tile = typename TileContainer::Tile,
             typename PointSet = typename PointSetContainer::mapped_type,
             typename T = std::invoke_result_t<Transform, Tile&, PointSet&>,
             typename V = std::invoke_result_t<Reduce, T, T> >
    V for_each_rec(TileContainer& tiles, PointSetContainer& point_sets, Transform transform, Reduce reduce = {}, V init = {})
    {
        typedef typename Tile::value_type::Tile_index Tile_index;
        std::map<Tile_index, std::future<V>> futures;
        {
            std::unique_lock<std::mutex> lock(mutex);
            for(const auto& [id, point_set] : point_sets)
                futures.emplace(id, pool.submit([this, &tiles, &point_sets, &transform, &init](Tile_index id){
                    return Impl::transform_zip_id(tiles, point_sets, transform, init, id, mutex);
                }, id));
        }

        V value = init;
        while (!futures.empty()) {
            auto fit = futures.begin();
            while(fit!=futures.end())
            {
                if (fit->second.wait_for(timeout_) != std::future_status::ready) {
                    ++fit;
                } else {
                    std::unique_lock<std::mutex> lock(mutex);
                    value = reduce(value, fit->second.get());
                    futures.erase(fit++);
                    std::vector<Tile_index> ids;
                    for(const auto& point_set : point_sets)
                        if (!point_set.second.points().at(point_set.first).empty())
                            ids.push_back(point_set.first);
                    for(auto id : ids)
                        if (futures.count(id) == 0)
                            futures.emplace(id, pool.submit([this, &tiles, &point_sets, &transform, &init](Tile_index id){
                                return Impl::transform_zip_id(tiles, point_sets, transform, init, id, mutex);
                            }, id));
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
