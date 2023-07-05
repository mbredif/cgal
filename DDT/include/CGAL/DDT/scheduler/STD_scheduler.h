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

// \ingroup PkgDDTSchedulerClasses
// \cgalModels Scheduler
#if __cplusplus >= 201703L
template<typename ExecutionPolicy
>
#endif
struct STD_scheduler
{

    STD_scheduler(int max_concurrency = 0) {}

    inline int max_concurrency() const { return 0; }

    template<typename Container,
             typename Transform,
             typename V,
             typename Reduce = std::plus<>>
    V transform_reduce(Container& c, V init, Transform transform, Reduce reduce = {})
    {
        return std::transform_reduce(CGAL_DDT_SCHEDULER_STD_SCHEDULER_PAR
                                     c.begin(), c.end(), init, reduce, [&c, &init, &transform](auto& p){
            p.second.locked = true;
            V value = init;
            if (c.load(p.first, p.second)) value = transform(p.first, p.second);
            p.second.locked = false;
            return value;
        } );
    }

    template<typename Container1,
             typename Container2,
             typename V,
             typename Transform,
             typename Reduce = std::plus<>,
             typename... Args>
    V join_transform_reduce(Container1& c1, Container2& c2, V init, Transform transform, Reduce reduce = {}, Args... args)
    {
        return std::transform_reduce(CGAL_DDT_SCHEDULER_STD_SCHEDULER_PAR
                                     c2.begin(), c2.end(), init, reduce, [&c1, &c2, &init, &transform, &args...](auto& p){
            typedef typename Container2::key_type key_type;
            typedef typename Container1::iterator iterator;
            typedef typename Container1::mapped_type mapped_type1;
            key_type k = p.first;
            iterator it = c1.emplace(k, std::move(mapped_type1(k, args...))).first;
            it->second.locked = true;
            V value = init;
            if (c1.load(k, it->second)) value = transform(k, it->second, p.second);
            c2.send_points(k);
            it->second.locked = false;
            return value;
        } );
    }

    template<typename Container1,
             typename Container2,
             typename V,
             typename Transform,
             typename Reduce = std::plus<>,
             typename... Args>
    V join_transform_reduce_loop(Container1& c1, Container2& c2, V init, Transform transform, Reduce reduce = {}, Args... args)
    {
        V value = init, v;
        do {
            v = join_transform_reduce(c1, c2, init, transform, reduce, args...);
            value = reduce(value, v);
        } while (v != init);
        return value;
    }
#if __cplusplus >= 201703L
    ExecutionPolicy policy;
#endif
};

#if __cplusplus >= 201703L
using STD_scheduler_par = CGAL::DDT::STD_scheduler<std::execution::parallel_policy>;
using STD_scheduler_seq = CGAL::DDT::STD_scheduler<std::execution::sequenced_policy>;
#else
using STD_scheduler_seq = CGAL::DDT::STD_scheduler;
#endif

}
}

#endif // CGAL_DDT_SCHEDULER_SEQUENTIAL_SCHEDULER_H
