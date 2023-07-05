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

#include <assert.h>
#include <CGAL/DDT/Tile.h>

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTSchedulerClasses
/// \cgalModels Scheduler
struct Sequential_scheduler
{
    /// constructor
    Sequential_scheduler(int max_concurrency = 0) { assert(max_concurrency==0 || max_concurrency==1); }

    inline int max_concurrency() const { return 1; }

    template<typename Container,
             typename Transform,
             typename V,
             typename Reduce = std::plus<>>
    V transform_reduce(Container& c, V init_v, Transform transform, Reduce reduce = {})
    {
        V value = init_v;
        for(auto& [k, v] : c) {
            v.locked = true;
            if (c.load(k, v)) value = reduce(value, transform(k, v));
            v.locked = false;
        }
        return value;
    }

    template<typename Container1,
             typename Container2,
             typename V,
             typename Transform,
             typename Reduce = std::plus<>,
             typename... Args>
    V join_transform_reduce(Container1& c1, Container2& c2, V init, Transform transform, Reduce reduce = {}, Args... args)
    {
        V value = init;
        for(auto& [k, v2] : c2) {
            typedef typename Container1::iterator iterator1;
            typedef typename Container1::mapped_type T1;
            iterator1 it = c1.try_emplace(k, k, std::forward<Args>(args)...).first;
            it->second.locked = true;

            T1& v1 = it->second;
            if (c1.load(k, it->second)) value = reduce(value, transform(k, v1, v2));

            c2.send_points(k);
            it->second.locked = false;
        }
        return value;
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
};

}
}

#endif // CGAL_DDT_SCHEDULER_SEQUENTIAL_SCHEDULER_H
