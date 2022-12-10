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

#ifndef CGAL_DDT_SCHEDULER_SEQUENTIAL_SCHEDULER_H
#define CGAL_DDT_SCHEDULER_SEQUENTIAL_SCHEDULER_H

#if __cplusplus >= 201703L
#include <execution>
#define CGAL_DDT_SCHEDULER_SEQUENTIAL_SCHEDULER_PAR policy,
#else
#define CGAL_DDT_SCHEDULER_SEQUENTIAL_SCHEDULER_PAR
#endif

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTSchedulerClasses
/// \cgalModels Scheduler
template<typename T
#if __cplusplus >= 201703L
       , typename ExecutionPolicy
#endif
>
struct STD_scheduler
{
    typedef T Tile;
    typedef typename Tile::Id Id;

    STD_scheduler(int max_concurrency = 0) {}

    inline int max_concurrency() const { return 0; }

    template<typename TileContainer, typename UnaryOp, typename V = int, typename BinaryOp = std::plus<>>
    V for_each(TileContainer& tc, UnaryOp transform, BinaryOp reduce = {}, V init = {})
    {
        return std::transform_reduce(CGAL_DDT_SCHEDULER_SEQUENTIAL_SCHEDULER_PAR
                                     tc.begin(), tc.end(), init, reduce, [&tc, &transform](Tile& t){
            tc.lock(t);
            tc.load(t);
            V v = transform(tc, t);
            tc.unlock(t);
            return v;
        } );
    }

    template<typename TileContainer, typename UnaryOp, typename V = int, typename BinaryOp = std::plus<>>
    V for_each_rec(TileContainer& tc, UnaryOp transform, BinaryOp reduce = {}, V init = {})
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
template <typename T> using STD_scheduler_par_unseq = CGAL::DDT::STD_scheduler<T, std::execution::parallel_unsequenced_policy>;
template <typename T> using STD_scheduler_seq = CGAL::DDT::STD_scheduler<T, std::execution::sequenced_policy>;
template <typename T> using STD_scheduler_unseq = CGAL::DDT::STD_scheduler<T, std::execution::unsequenced_policy>;
#endif

}
}

#endif // CGAL_DDT_SCHEDULER_SEQUENTIAL_SCHEDULER_H
