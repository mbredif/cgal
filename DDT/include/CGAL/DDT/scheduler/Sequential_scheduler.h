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
#include <functional>

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTSchedulerClasses
/// \cgalModels Scheduler
struct Sequential_scheduler
{
    /// constructor
    Sequential_scheduler(int max_concurrency = 0) { assert(max_concurrency==0 || max_concurrency==1); }

    inline int max_concurrency() const { return 1; }

    template<typename OutputValue,
             typename Container,
             typename OutputIterator,
             typename Transform>
    OutputIterator flat_map(Container& c, OutputIterator out, Transform transform)
    {
        for(auto it = c.begin(); it != c.end(); ++it)
            out = transform(it->first, it->second, out);
        return out;
    }

    template<typename OutputValue,
             typename Container,
             typename OutputIterator,
             typename V,
             typename Reduce,
             typename Transform>
    std::pair<V,OutputIterator>
    reduce_by_key(Container& c, OutputIterator out, V value, Reduce reduce, Transform transform)
    {
        for(auto it = c.begin(); it != c.end();)
        {
            auto range = c.equal_range(it->first);
            auto res = transform(range, out);
            out = res.second;
            value = reduce(value, res.first);
            it = range.second;
        }
        return { value, out };
    }

    template<typename Container,
             typename V,
             typename Reduce,
             typename Transform>
    V transform_reduce(Container& c, V value, Reduce reduce, Transform transform)
    {
        for(auto it = c.begin(); it != c.end(); ++it)
            value = reduce(value, transform(it->first, it->second));
        return value;
    }
    template<typename Container1,
             typename Container2,
             typename OutputIterator,
             typename Transform,
             typename... Args>
    OutputIterator
    join_transform_reduce(Container1& c1, Container2& c2, OutputIterator out, Transform transform, Args&&... args)
    {
        typedef typename Container1::iterator iterator1;
        typedef typename Container2::iterator iterator2;
        typedef typename Container1::key_type key_type;
        for(iterator2 it2 = c2.begin(); it2 != c2.end(); ++it2) {
            key_type k = it2->first;
            iterator1 it1 = c1.emplace(std::piecewise_construct,
                std::forward_as_tuple(k),
                std::forward_as_tuple(k, std::forward<Args>(args)...)).first;
            out = transform(k, it1->second, it2->second, out);
        }
        return out;
    }

    template<typename Container1,
             typename Container2,
             typename OutputIterator2,
             typename Transform,
             typename... Args>
    void join_transform_reduce_loop(Container1& c1, Container2& c2, OutputIterator2 out2, Transform transform, Args&&... args)
    {
        typedef typename Container1::iterator iterator1;
        typedef typename Container2::iterator iterator2;
        typedef typename Container1::key_type key_type;
        while(!c2.empty()) {
            iterator2 it2 = c2.begin();
            key_type k = it2->first;
            iterator1 it1 = c1.emplace(std::piecewise_construct,
                std::forward_as_tuple(k),
                std::forward_as_tuple(k, std::forward<Args>(args)...)).first;
            out2 = transform(k, it1->second, it2->second, out2);
            c2.erase(it2);
        }
    }
};

}
}

#endif // CGAL_DDT_SCHEDULER_SEQUENTIAL_SCHEDULER_H
