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
V transform_reduce_id(TileContainer& tc, Transform transform, Reduce reduce, V value, Tile_index id, std::mutex& mutex)
{
    typedef typename TileContainer::iterator Tile_iterator;
    Tile_iterator tile;
    {
        std::unique_lock<std::mutex> lock(mutex);
        tile = tc.emplace(id).first;
        tile->locked = true;
        tc.prepare_load(*tile);
    }
    if (tc.safe_load(*tile)) value = reduce(value, transform(*tile));
    {
        std::unique_lock<std::mutex> lock(mutex);
        tile->locked = false;
    }
    return value;
}

template<typename TileContainer, typename MessagingContainer, typename Transform, typename Reduce, typename V, typename Tile_index>
V transform_reduce_id(TileContainer& tc, MessagingContainer& messagings, Transform transform, Reduce reduce, V value, Tile_index id, std::mutex& mutex)
{
    typedef typename TileContainer::iterator Tile_iterator;
    Tile_iterator tile;
    {
        std::unique_lock<std::mutex> lock(mutex);
        tile = tc.emplace(id).first;
        tile->locked = true;
        tc.prepare_load(*tile);
    }
    if (tc.safe_load(*tile)) value = reduce(value, transform(*tile, messagings[id]));
    {
        std::unique_lock<std::mutex> lock(mutex);
        messagings.send_points(id);
        tile->locked = false;
    }
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
    V for_each(TileContainer& tc, Transform transform, Reduce reduce = {}, V init = {})
    {
        typedef typename TileContainer::Tile_index Tile_index;
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
             typename MessagingContainer,
             typename Transform,
             typename Reduce = std::plus<>,
             typename Tile = typename TileContainer::Tile,
             typename Messaging = typename MessagingContainer::mapped_type,
             typename V = std::invoke_result_t<Reduce,
                                               std::invoke_result_t<Transform, Tile&, Messaging&>,
                                               std::invoke_result_t<Transform, Tile&, Messaging&> > >
    V for_each_zip(TileContainer& tc, MessagingContainer& messagings, Transform transform, Reduce reduce = {}, V init = {})
    {
        typedef typename TileContainer::Tile_index Tile_index;
        std::vector<Tile_index> ids;
        for (const auto& msg : messagings) ids.push_back(msg.first);
        return arena.execute([&]{
            return tbb::parallel_reduce(
                  tbb::blocked_range<int>(0,ids.size()),
                  init, [&](tbb::blocked_range<int> r, double running_total)
            {
                V value = init;
                for (int i=r.begin(); i<r.end(); ++i)
                    value = Impl::transform_reduce_id(tc, messagings, transform, reduce, value, ids[i], mutex);
                return value;
            }, reduce);
        });
    }

    template<typename TileContainer,
             typename MessagingContainer,
             typename Transform,
             typename Reduce = std::plus<>,
             typename Tile = typename TileContainer::Tile,
             typename Messaging = typename MessagingContainer::mapped_type,
             typename V = std::invoke_result_t<Reduce,
                                               std::invoke_result_t<Transform, Tile&, Messaging&>,
                                               std::invoke_result_t<Transform, Tile&, Messaging&> > >
    V for_each_rec(TileContainer& tc, MessagingContainer& messagings, Transform transform, Reduce reduce = {}, V init = {})
    {
        V value = init, v;
        do {
            v = for_each_zip(tc, messagings, transform, reduce, init);
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
