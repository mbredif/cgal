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
/// \cgalModels{Scheduler}
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
    OutputIterator ranges_transform(Container& c, Transform transform, OutputIterator out)
    {
        CGAL_DDT_TRACE0(*this, "PERF", Type<Transform>::name, "generic_work", "B");
        auto first = std::begin(c), end = std::end(c), last = first;
        while(first != end) {
            if (++last == end || first->first != last->first) {
                CGAL_DDT_TRACE2(*this, "PERF", Type<Transform>::name, 0, "B", k, to_string(first->first), in, to_summary(first, last));
                out = transform(first, last, out);
                CGAL_DDT_TRACE0(*this, "PERF", Type<Transform>::name, 0, "E");
                first = last;
            }
        }
        CGAL_DDT_TRACE0(*this, "PERF", Type<Transform>::name, "generic_work", "E");
        return out;
    }

    template<typename Container,
             typename Transform,
             typename V,
             typename Reduce>
    V ranges_reduce(Container& c, Transform transform, V value, Reduce reduce)
    {
        CGAL_DDT_TRACE0(*this, "PERF", "reduce", "generic_work", "B");
        auto first = std::begin(c), end = std::end(c), last = first;
        while(first != end) {
            if (++last == end || first->first != last->first) {
                CGAL_DDT_TRACE2(*this, "PERF", Type<Transform>::name, 0, "B", k, to_string(first->first), in, to_summary(first, last));
                V val = transform(first, last);
                CGAL_DDT_TRACE1(*this, "PERF", Type<Transform>::name, 0, "E", value, val);
                value = reduce(value, val);
                first = last;
            }
        }
        CGAL_DDT_TRACE1(*this, "PERF", "reduce", "generic_work", "E", value, value);
        return value;
    }

    template<typename OutputValue,
             typename Container,
             typename Transform,
             typename V,
             typename Reduce,
             typename OutputIterator>
    std::pair<V,OutputIterator>
    ranges_transform_reduce(Container& c, Transform transform, V value, Reduce reduce, OutputIterator out)
    {
        CGAL_DDT_TRACE0(*this, "PERF", "transform_reduce", "generic_work", "B");
        auto first = std::begin(c), end = std::end(c), last = first;
        while(first != end) {
            if (++last == end || first->first != last->first) {
                CGAL_DDT_TRACE2(*this, "PERF", Type<Transform>::name, 0, "B", k, to_string(first->first), in, to_summary(first, last));
                auto res = transform(first, last, out);
                CGAL_DDT_TRACE1(*this, "PERF", Type<Transform>::name, 0, "E", value, to_string(res.first));
                value = reduce(value, res.first);
                out = res.second;
                first = last;
            }
        }
        CGAL_DDT_TRACE1(*this, "PERF", "transform_reduce", "generic_work", "E", value, value);
        return { value, out };
    }

    template<typename OutputValue3,
             typename InputIterator1,
             typename Container2,
             typename Transform,
             typename OutputIterator3,
             typename... Args2>
    OutputIterator3 transform_range(InputIterator1 first1, InputIterator1 last1, Container2& c2, Transform transform, OutputIterator3 out3, Args2&&... args2)
    {
        typedef typename Container2::key_type key_type;
        key_type k = first1->first;
        auto it2 = c2.emplace(std::piecewise_construct,
            std::forward_as_tuple(k),
            std::forward_as_tuple(k, std::forward<Args2>(args2)...)).first;
        std::vector<OutputValue3> v3;
        CGAL_DDT_TRACE2(*this, "PERF", Type<Transform>::name, 0, "B", k, to_string(k), in, to_summary(first1, last1));
        transform(first1, last1, it2->second, std::back_inserter(v3));
        CGAL_DDT_TRACE2(*this, "PERF", Type<Transform>::name, 0, "E", inout, to_summary(first1, last1), out, to_summary(v3.begin(), v3.end()));
        return std::move(v3.begin(), v3.end(), out3);
    }

    template<typename OutputValue3,
             typename Container1,
             typename Container2,
             typename Transform,
             typename OutputIterator3,
             typename... Args2>
    OutputIterator3 ranges_transform(Container1& c1, Container2& c2, Transform transform, OutputIterator3 out3, Args2&&... args2)
    {
        CGAL_DDT_TRACE0(*this, "PERF", Type<Transform>::name, "generic_work", "B");
        auto first1 = std::begin(c1), end1 = std::end(c1), last1 = first1;
        while(first1 != end1) {
            if (++last1 == end1 || first1->first != last1->first) {
                out3 = transform_range<OutputValue3>(first1, last1, c2, transform, out3, std::forward<Args2>(args2)...);
                first1 = last1;
            }
        }
        CGAL_DDT_TRACE0(*this, "PERF", Type<Transform>::name, "generic_work", "E");
        return out3;
    }

    template<typename Container1,
             typename Container2,
             typename Container3,
             typename Transform,
             typename... Args2>
    void ranges_for_each(Container1& c1, Container2& c2, Container3& c3, Transform transform, Args2&&... args2)
    {
        typedef typename Container3::value_type value_type3;
        typedef typename Container3::key_type   key_type;
        CGAL_DDT_TRACE0(*this, "PERF", "for_each", "generic_work", "B");
        auto first1 = std::begin(c1), end1 = std::end(c1), last1 = first1;
        while(first1 != end1) {
            if (++last1 == end1 || first1->first != last1->first) {
                CGAL_DDT_TRACE1(*this, "PERF", "item", "generic_work", "B", k, to_string(first1->first));
                transform_range<value_type3>(first1, last1, c2, transform, std::inserter(c3, c3.begin()), std::forward<Args2>(args2)...);
                first1 = last1;

                while(!c3.empty()) {
                    auto first3 = c3.begin(), end3 = c3.end(), last3 = first3;
                    do { ++last3; } while (last3!=end3 && last3->first == first3->first);

                    key_type k = first3->first;
                    auto it2 = c2.emplace(std::piecewise_construct,
                        std::forward_as_tuple(k),
                        std::forward_as_tuple(k, std::forward<Args2>(args2)...)).first;
                    std::vector<value_type3> v3;
                    CGAL_DDT_TRACE2(*this, "PERF", Type<Transform>::name, 0, "B", k, to_string(k), in, to_summary(first3, last3));
                    transform(first3, last3, it2->second, std::back_inserter(v3));
                    CGAL_DDT_TRACE2(*this, "PERF", Type<Transform>::name, 0, "E", inout, to_summary(first3, last3), out, to_summary(v3.begin(), v3.end()));
                    c3.erase(first3, last3);
                    std::move(v3.begin(), v3.end(), std::inserter(c3, c3.begin()));
                }
                CGAL_DDT_TRACE0(*this, "PERF", "item", "generic_work", "E");
            }
        }
        CGAL_DDT_TRACE0(*this, "PERF", "for_each", "generic_work", "E");
    }

    static constexpr int thread_index() { return 0; }

#ifdef CGAL_DDT_TRACING
public:
    typedef std::chrono::time_point<std::chrono::high_resolution_clock> clock_type;
    std::size_t clock_microsec() const { return std::chrono::duration<double, std::micro>(clock_now() - trace.t0).count(); }
    clock_type clock_now() const { return std::chrono::high_resolution_clock::now(); }
    trace_logger<clock_type> trace = {"perf_seq.json", clock_now()};
    int process_index() const { return 0; }
#endif
};

}
}

#endif // CGAL_DDT_SCHEDULER_SEQUENTIAL_SCHEDULER_H
