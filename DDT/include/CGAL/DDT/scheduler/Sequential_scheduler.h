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
    V transform_reduce(Container& c, V value, Transform transform, Reduce reduce = {})
    {
        typedef typename Container::iterator iterator;
        for(iterator it = c.begin(); it != c.end(); ++it)
            value = reduce(value, transform(it->first, it->second));
        return value;
    }

    template<typename Container,
             typename Iterator,
             typename Transform>
    void flat_map(Container& c, Iterator out, Transform transform)
    {
        for(auto it = c.begin(); it != c.end(); ++it)
            transform(it->first, it->second, out);
    }

    template<typename Container,
             typename Iterator,
             typename Transform,
             typename V,
             typename Reduce = std::plus<>>
    V reduce_by_key(Container& c, Iterator out, V value, Transform transform, Reduce reduce = {})
    {
        for(auto it = c.begin(); it != c.end();)
        {
            auto range = c.equal_range(it->first);
            value = reduce(value, transform(range, out));
            it = range.second;
        }
        return value;
    }

    template<typename Container1,
             typename Container2,
             typename V,
             typename Transform,
             typename Reduce = std::plus<>,
             typename... Args>
    V join_transform_reduce(Container1& c1, Container2& c2, V value, Transform transform, Reduce reduce = {}, Args&&... args)
    {
        typedef typename Container1::iterator iterator1;
        typedef typename Container2::iterator iterator2;
        typedef typename Container1::key_type key_type;
        for(iterator2 it2 = c2.begin(); it2 != c2.end(); ++it2) {
            key_type k = it2->first;
            iterator1 it1 = c1.try_emplace(k, k, std::forward<Args>(args)...).first;
            value = reduce(value, transform(k, it1->second, it2->second));
            c2.send_points(k);
        }
        return value;
    }

    template<typename Container1,
             typename Container2,
             typename V,
             typename Transform,
             typename Reduce = std::plus<>,
             typename... Args>
    V join_transform_reduce_loop(Container1& c1, Container2& c2, V init, Transform transform, Reduce reduce = {}, Args&&... args)
    {
        V value = init, v;
        do {
            v = join_transform_reduce(c1, c2, init, transform, reduce, std::forward<Args>(args)...);
            value = reduce(value, v);
        } while (v != init);
        return value;
    }
};

}
}

#endif // CGAL_DDT_SCHEDULER_SEQUENTIAL_SCHEDULER_H
