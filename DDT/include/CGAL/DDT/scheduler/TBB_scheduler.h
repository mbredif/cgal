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

#ifndef CGAL_DDT_SCHEDULER_TBB_SCHEDULER_H
#define CGAL_DDT_SCHEDULER_TBB_SCHEDULER_H

#ifndef CGAL_LINKED_WITH_TBB
#error TBB not properly setup with CGAL
#endif

#include <map>
#include <set>
#include <vector>
#include <mutex>
#include <tbb/concurrent_vector.h>
#include <tbb/parallel_reduce.h>
#include <tbb/task_arena.h>
#include <CGAL/DDT/Tile.h>

namespace CGAL {
namespace DDT {

namespace Impl {
template<typename TileContainer, typename Transform, typename Reduce, typename V, typename Tile_index>
V transform_reduce_id(TileContainer& tiles, Transform transform, Reduce reduce, V value, Tile_index id, std::mutex& mutex)
{
    typedef typename TileContainer::Tile Tile;
    std::unique_lock<std::mutex> lock(mutex);
    Tile& tile = tiles.emplace(id).first->second;
    tile.locked = true;
    tiles.prepare_load(id, tile);

    lock.unlock();
    if (tiles.safe_load(id, tile)) value = reduce(value, transform(tile));

    lock.lock();
    tile.locked = false;
    return value;
}

template<typename TileContainer, typename PointSetContainer, typename Transform, typename Reduce, typename V, typename Tile_index>
V transform_reduce_id(TileContainer& tiles, PointSetContainer& point_sets, Transform transform, Reduce reduce, V value, Tile_index id, std::mutex& mutex)
{
    typedef typename TileContainer::Tile Tile;
    std::unique_lock<std::mutex> lock(mutex);
    Tile& tile = tiles.emplace(id).first->second;
    tile.locked = true;
    tiles.prepare_load(id, tile);

    lock.unlock();
    if (tiles.safe_load(id, tile)) value = reduce(value, transform(tile, point_sets[id]));

    lock.lock();
    point_sets.send_points(id);
    tile.locked = false;
    return value;
}
}

/// \ingroup PkgDDTSchedulerClasses
/// \cgalModels Scheduler
struct TBB_scheduler
{
    TBB_scheduler(int max_concurrency = 0) : arena(max_concurrency ? max_concurrency : tbb::task_arena::automatic) {}

    inline int max_concurrency() const { return arena.max_concurrency(); }

    template<typename TileContainer,
         typename Transform,
         typename Reduce = std::plus<>,
         typename Tile = typename TileContainer::Tile,
         typename V = std::invoke_result_t<Reduce,
                                           std::invoke_result_t<Transform, Tile&>,
                                           std::invoke_result_t<Transform, Tile&> > >
    V for_each(TileContainer& tiles, Transform transform, Reduce reduce = {}, V init = {})
    {
        typedef typename TileContainer::Tile_index Tile_index;
        std::vector<Tile_index> ids(tiles.ids_begin(), tiles.ids_end());
        return arena.execute([&]{
            return tbb::parallel_reduce(
                  tbb::blocked_range<int>(0,ids.size()),
                  init, [&](tbb::blocked_range<int> r, V value)
            {
                for (int i=r.begin(); i<r.end(); ++i)
                    value = Impl::transform_reduce_id(tiles, transform, reduce, value, ids[i], mutex);
                return value;
            }, reduce);
        });
    }

    template<typename TileContainer,
             typename PointSetContainer,
             typename Transform,
             typename Reduce = std::plus<>,
             typename Tile = typename TileContainer::Tile,
             typename PointSet = typename PointSetContainer::mapped_type,
             typename V = std::invoke_result_t<Reduce,
                                               std::invoke_result_t<Transform, Tile&, PointSet&>,
                                               std::invoke_result_t<Transform, Tile&, PointSet&> > >
    V for_each_zip(TileContainer& tiles, PointSetContainer& point_sets, Transform transform, Reduce reduce = {}, V init = {})
    {
        typedef typename TileContainer::Tile_index Tile_index;
        std::vector<Tile_index> ids;
        for (const auto& msg : point_sets) ids.push_back(msg.first);
        return arena.execute([&]{
            return tbb::parallel_reduce(
                  tbb::blocked_range<int>(0,ids.size()),
                  init, [&](tbb::blocked_range<int> r, V value)
            {
                for (int i=r.begin(); i<r.end(); ++i)
                    value = Impl::transform_reduce_id(tiles, point_sets, transform, reduce, value, ids[i], mutex);
                return value;
            }, reduce);
        });
    }

    template<typename TileContainer,
             typename PointSetContainer,
             typename Transform,
             typename Reduce = std::plus<>,
             typename Tile = typename TileContainer::Tile,
             typename PointSet = typename PointSetContainer::mapped_type,
             typename V = std::invoke_result_t<Reduce,
                                               std::invoke_result_t<Transform, Tile&, PointSet&>,
                                               std::invoke_result_t<Transform, Tile&, PointSet&> > >
    V for_each_rec(TileContainer& tiles, PointSetContainer& point_sets, Transform transform, Reduce reduce = {}, V init = {})
    {
        V value = init, v;
        do {
            v = for_each_zip(tiles, point_sets, transform, reduce, init);
            value = reduce(value, v);
        } while (v != init);
        return value;
    }

private:
    tbb::task_arena arena;
    std::mutex mutex;
};

}
}

#endif // CGAL_DDT_SCHEDULER_TBB_SCHEDULER_H
