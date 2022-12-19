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

namespace CGAL {
namespace DDT {

namespace Impl {
template<typename TileContainer, typename Transform, typename Reduce, typename V, typename Tile_index>
V transform_reduce_id(TileContainer& tc, Transform transform, Reduce reduce, V value, Tile_index id, std::mutex& mutex)
{
    typedef typename TileContainer::iterator Tile_const_iterator;
    Tile_const_iterator tile;
    {
        std::unique_lock<std::mutex> lock(mutex);
        tile = tc.find(id);
        tile->locked = true;
        tc.prepare_load(*tile);
    }
    if (tc.safe_load(*tile)) value = reduce(value, transform(*tile));
    {
        std::unique_lock<std::mutex> lock(mutex);
        tc.send_points(*tile);
        tile->locked = false;
    }
    return value;
}
}

/// \ingroup PkgDDTSchedulerClasses
/// \cgalModels Scheduler
template<typename T>
struct TBB_scheduler
{
    typedef T Tile;
    typedef typename Tile::Tile_index Tile_index;

    TBB_scheduler(int max_concurrency = 0) : arena(max_concurrency ? max_concurrency : tbb::task_arena::automatic) {}

    inline int max_concurrency() const { return arena.max_concurrency(); }

    template<typename TileContainer,
         typename Transform,
         typename Reduce = std::plus<>,
         typename V = std::invoke_result_t<Reduce,
                                           std::invoke_result_t<Transform, Tile&>,
                                           std::invoke_result_t<Transform, Tile&> > >
    V for_each(TileContainer& tc, Transform transform, Reduce reduce = {}, V init = {})
    {
        std::vector<Tile_index> ids(tc.ids_begin(), tc.ids_end());
        return arena.execute([&]{
            return tbb::parallel_reduce(
                  tbb::blocked_range<int>(0,ids.size()),
                  init, [&](tbb::blocked_range<int> r, double running_total)
            {
                V value = init;
                for (int i=r.begin(); i<r.end(); ++i)
                    value = Impl::transform_reduce_id(tc, transform, reduce, value, ids[i], mutex);
                return value;
            }, reduce);
        });
    }

    template<typename TileContainer,
         typename Transform,
         typename Reduce = std::plus<>,
         typename V = std::invoke_result_t<Reduce,
                                           std::invoke_result_t<Transform, Tile&>,
                                           std::invoke_result_t<Transform, Tile&> > >
    V for_each_rec(TileContainer& tc, Transform transform, Reduce reduce = {}, V init = {})
    {
        V value = init, v;
        do {
            v = for_each(tc, transform, reduce, init);
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
