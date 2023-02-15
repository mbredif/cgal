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

#ifndef CGAL_DDT_SCHEDULER_STD_SCHEDULER_H
#define CGAL_DDT_SCHEDULER_STD_SCHEDULER_H

#if __cplusplus >= 201703L
#include <execution>
#define CGAL_DDT_SCHEDULER_STD_SCHEDULER_PAR policy,
#else
#define CGAL_DDT_SCHEDULER_STD_SCHEDULER_PAR
#endif
#include <CGAL/DDT/Tile.h>

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTSchedulerClasses
/// \cgalModels Scheduler
template<typename TriangulationTraits
#if __cplusplus >= 201703L
       , typename ExecutionPolicy
#endif
>
struct STD_scheduler
{
    typedef TriangulationTraits Traits;
    typedef CGAL::DDT::Tile<Traits> Tile;
    typedef typename Tile::Tile_index Tile_index;

    STD_scheduler(int max_concurrency = 0) {}

    inline int max_concurrency() const { return 0; }

    template<typename TileContainer,
         typename Transform,
         typename Reduce = std::plus<>,
         typename V = std::invoke_result_t<Reduce,
                                           std::invoke_result_t<Transform, Tile&>,
                                           std::invoke_result_t<Transform, Tile&> > >
    V for_each(TileContainer& tc, Transform transform, Reduce reduce = {}, V init = {})
    {
        return std::transform_reduce(CGAL_DDT_SCHEDULER_STD_SCHEDULER_PAR
                                     tc.begin(), tc.end(), init, reduce, [&tc, &transform, &init](Tile& tile){
            tile.locked = true;
            V value = init;
            if (tc.load(tile)) value = transform(tile);
            tc.send_points(tile);
            tile.locked = false;
            return value;
        } );
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
#if __cplusplus >= 201703L
    ExecutionPolicy policy;
#endif
};

#if __cplusplus >= 201703L
template <typename T> using STD_scheduler_par = CGAL::DDT::STD_scheduler<T, std::execution::parallel_policy>;
template <typename T> using STD_scheduler_seq = CGAL::DDT::STD_scheduler<T, std::execution::sequenced_policy>;
#else
template <typename T> using STD_scheduler_par = CGAL::DDT::STD_scheduler<T>;
template <typename T> using STD_scheduler_seq = CGAL::DDT::STD_scheduler<T>;
#endif

}
}

#endif // CGAL_DDT_SCHEDULER_SEQUENTIAL_SCHEDULER_H
