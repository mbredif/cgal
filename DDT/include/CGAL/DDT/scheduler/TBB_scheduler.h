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
    }
    return value;
}

/// \ingroup PkgDDTSchedulerClasses
/// \cgalModels Scheduler
template<typename T>
struct TBB_scheduler
{
    typedef T Tile;
    typedef typename Tile::Id Id;

    TBB_scheduler(int num_threads = 0) : arena(num_threads ? num_threads : tbb::task_arena::automatic) {}

    inline int max_concurrency() const { return arena.max_concurrency(); }

    template<typename TileContainer, typename UnaryOp, typename V = int, typename BinaryOp = std::plus<>>
    V for_each(TileContainer& tc, UnaryOp op1, BinaryOp op2 = {}, V init = {})
    {
        std::vector<Id> ids(tc.tile_ids_begin(), tc.tile_ids_end());
        return arena.execute([&]{
            return tbb::parallel_reduce(
                  tbb::blocked_range<int>(0,ids.size()),
                  init, [&](tbb::blocked_range<int> r, double running_total)
            {
                V c = init;
                for (int i=r.begin(); i<r.end(); ++i)
                    c = op2(c, for_each_function<V>(tc, op1, mutex, ids[i]));
                return c;
            }, op2 );
        });
    }

    template<typename TileContainer, typename UnaryOp, typename V = int, typename BinaryOp = std::plus<>>
    V for_each_rec(TileContainer& tc, UnaryOp op1, BinaryOp op2 = {}, V init = {})
    {
        V value = init, v;
        do {
            v = for_each(tc, op1, op2, init);
            value = op2(value, v);
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
