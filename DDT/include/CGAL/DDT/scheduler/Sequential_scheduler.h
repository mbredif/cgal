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
#include <chrono>
#include <CGAL/DDT/IO/trace_logger.h>

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTSchedulerClasses
/// \cgalModels Scheduler
struct Sequential_scheduler
{
    /// constructor
    Sequential_scheduler(int max_concurrency = 0) {
        assert(max_concurrency==0 || max_concurrency==1);
#ifdef CGAL_DDT_TRACING
        trace.t0 = clock_now();
#endif
    }

    inline int max_concurrency() const { return 1; }

    template<typename OutputValue,
             typename Container,
             typename OutputIterator,
             typename Transform>
    OutputIterator for_each(Container& c, Transform transform, OutputIterator out)
    {
        CGAL_DDT_TRACE0(*this, "PERF", "for_each", "generic_work", "B");
        auto first = std::begin(c), end = std::end(c), last = first;
        while(first != end) {
            ++last;
            if (last != end && first->first == last->first) continue;
            CGAL_DDT_TRACE1(*this, "PERF", "transform", 0, "B", k, to_string(first->first));
            out = transform(first, last, out);
            CGAL_DDT_TRACE0(*this, "PERF", "transform", 0, "E");
            first = last;
        }
        CGAL_DDT_TRACE0(*this, "PERF", "for_each", "generic_work", "E");
        return out;
    }

    template<typename Container,
             typename Transform,
             typename V,
             typename Reduce>
    V for_each(Container& c, Transform transform, V value, Reduce reduce)
    {
        CGAL_DDT_TRACE0(*this, "PERF", "for_each", "generic_work", "B");
        auto first = std::begin(c), end = std::end(c), last = first;
        while(first != end) {
            ++last;
            if (last != end && first->first == last->first) continue;
            CGAL_DDT_TRACE1(*this, "PERF", "transform", 0, "B", k, to_string(first->first));
            V val = transform(first, last);
            CGAL_DDT_TRACE1(*this, "PERF", "transform", 0, "E", value, val);
            value = reduce(value, val);
            first = last;
        }
        CGAL_DDT_TRACE1(*this, "PERF", "for_each", "generic_work", "E", value, value);
        return value;
    }

    template<typename OutputValue,
             typename Container,
             typename Transform,
             typename V,
             typename Reduce,
             typename OutputIterator>
    std::pair<V,OutputIterator>
    for_each(Container& c, Transform transform, V value, Reduce reduce, OutputIterator out)
    {
        CGAL_DDT_TRACE0(*this, "PERF", "for_each", "generic_work", "B");
        auto first = std::begin(c), end = std::end(c), last = first;
        while(first != end) {
            ++last;
            if (last != end && first->first == last->first) continue;
            CGAL_DDT_TRACE1(*this, "PERF", "transform", 0, "B", k, to_string(first->first));
            auto res = transform(first, last, out);
            CGAL_DDT_TRACE1(*this, "PERF", "transform", 0, "E", value, to_string(res.first));
            value = reduce(value, res.first);
            out = res.second;
            first = last;
        }
        CGAL_DDT_TRACE1(*this, "PERF", "for_each", "generic_work", "E", value, value);
        return { value, out };
    }

    template<typename OutputValue,
             typename Container1,
             typename Container2,
             typename Transform,
             typename OutputIterator3,
             typename... Args2>
    OutputIterator3 left_join(Container1& c1, Container2& c2, Transform transform, OutputIterator3 out3, Args2&&... args2)
    {
        CGAL_DDT_TRACE0(*this, "PERF", "left_join", "generic_work", "B");
        auto first1 = std::begin(c1), end1 = std::end(c1), last1 = first1;
        while(first1 != end1) {
            ++last1;
            if (last1 != end1 && first1->first == last1->first) continue;
            auto k = first1->first;
            auto it2 = c2.emplace(std::piecewise_construct,
                std::forward_as_tuple(k),
                std::forward_as_tuple(k, std::forward<Args2>(args2)...)).first;
            CGAL_DDT_TRACE1(*this, "PERF", "transform", 0, "B", k, to_string(k));
            out3 = transform(first1, last1, it2->second, out3);
            CGAL_DDT_TRACE0(*this, "PERF", "transform", 0, "E");
            first1 = last1;
        }
        CGAL_DDT_TRACE0(*this, "PERF", "left_join", "generic_work", "E");
        return out3;
    }

    template<typename Container1,
             typename Container2,
             typename OutputIterator1,
             typename Transform,
             typename... Args2>
    void left_join_loop(Container1& c1, Container2& c2, Transform transform, OutputIterator1 out1, Args2&&... args2)
    {
        CGAL_DDT_TRACE0(*this, "PERF", "left_join_loop", "generic_work", "B");
        while(!c1.empty())
        {
            auto first1 = std::begin(c1), end1 = std::end(c1), last1 = first1;
            auto k = last1->first;
            while(last1 != end1 && last1->first == k) ++last1;
            auto it2 = c2.emplace(std::piecewise_construct,
                std::forward_as_tuple(k),
                std::forward_as_tuple(k, std::forward<Args2>(args2)...)).first;
            CGAL_DDT_TRACE1(*this, "PERF", "transform", 0, "B", k, to_string(k));
            std::vector<typename Container1::value_type> output1;
            transform(first1, last1, it2->second, std::back_inserter(output1));
            CGAL_DDT_TRACE0(*this, "PERF", "transform", 0, "E");
            c1.erase(first1, last1);
            out1 = std::move(output1.begin(), output1.end(), out1);
        }
        CGAL_DDT_TRACE0(*this, "PERF", "left_join_loop", "generic_work", "E");
    }

#ifdef CGAL_DDT_TRACING
public:
    typedef std::chrono::time_point<std::chrono::high_resolution_clock> clock_type;
    static constexpr int thread_index() { return 0; }
    std::size_t clock_microsec() const { return std::chrono::duration<double, std::micro>(clock_now() - trace.t0).count(); }
    clock_type clock_now() const { return std::chrono::high_resolution_clock::now(); }
    trace_logger<clock_type> trace;
#endif
};

}
}

#endif // CGAL_DDT_SCHEDULER_SEQUENTIAL_SCHEDULER_H
