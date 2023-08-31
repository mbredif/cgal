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
#include <functional>
#include <chrono>

#include <CGAL/DDT/IO/trace_logger.h>

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

    template<typename Container, typename Key>
    void get_unique_keys(Container& c, std::vector<Key>& keys) const { // TODO const Container&
        for(const auto& [k,v] : c)
            keys.push_back(k);
        keys.erase(std::unique(keys.begin(), keys.end()), keys.end());
    }

    template<typename OutputValue,
             typename Container,
             typename Transform,
             typename OutputIterator>
    OutputIterator ranges_transform(Container& c, Transform transform, OutputIterator out)
    {
        CGAL_DDT_TRACE0(*this, "PERF", "transform", "generic_work", "B");
        typedef typename Container::key_type key_type;
        std::vector<key_type> keys;
        get_unique_keys(c, keys);

        std::for_each(CGAL_DDT_SCHEDULER_STD_SCHEDULER_PAR
            keys.begin(), keys.end(), [this, &c, &transform, &out](key_type k){
                auto range = c.equal_range(k);
                std::vector<OutputValue> output;
                CGAL_DDT_TRACE1_LOCK(*this, "PERF", "transform", 0, "B", k, to_string(k));
                transform(range.first, range.second, std::back_inserter(output));
                std::unique_lock<std::mutex> lock(mutex);
                CGAL_DDT_TRACE1(*this, "PERF", "transform", 0, "E", sizes, to_summary(output.begin(), output.end()));
                CGAL_DDT_TRACE1(*this, "LOCK", "mutex", "bad", "B", k, to_string(k));
                out = std::move(output.begin(), output.end(), out);
                CGAL_DDT_TRACE0(*this, "LOCK", "mutex", "bad", "E");
        });

        CGAL_DDT_TRACE0(*this, "PERF", "transform", "generic_work", "E");
        return out;
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
        typedef typename Container::key_type key_type;
        std::vector<key_type> keys;
        get_unique_keys(c, keys);

        value = std::transform_reduce(CGAL_DDT_SCHEDULER_STD_SCHEDULER_PAR
            keys.begin(), keys.end(), value, reduce, [this, &c, &transform, &out](key_type k){
                auto range = c.equal_range(k);
                std::vector<OutputValue> output;
                CGAL_DDT_TRACE1_LOCK(*this, "PERF", "transform", 0, "B", k, to_string(k));
                auto res = transform(range.first, range.second, std::back_inserter(output));
                std::unique_lock<std::mutex> lock(mutex);
                CGAL_DDT_TRACE2(*this, "PERF", "transform", 0, "E", value, res.first, sizes, to_summary(output.begin(), output.end()));
                CGAL_DDT_TRACE1(*this, "LOCK", "mutex", "bad", "B", k, to_string(k));
                out = std::move(output.begin(), output.end(), out);
                CGAL_DDT_TRACE0(*this, "LOCK", "mutex", "bad", "E");
                return res.first;
        });

        CGAL_DDT_TRACE1(*this, "PERF", "transform_reduce", "generic_work", "E", value, value);
        return { value, out };
    }

    template<typename Container,
             typename V,
             typename Reduce,
             typename Transform>
    V ranges_reduce(Container& c, Transform transform, V value, Reduce reduce)
    {
        CGAL_DDT_TRACE0(*this, "PERF", "reduce", "generic_work", "B");
        typedef typename Container::key_type key_type;
        std::vector<key_type> keys;
        get_unique_keys(c, keys);

        value = std::transform_reduce(CGAL_DDT_SCHEDULER_STD_SCHEDULER_PAR
            keys.begin(), keys.end(), value, reduce, [this, &c, &transform](key_type k){
                auto range = c.equal_range(k);
                CGAL_DDT_TRACE1_LOCK(*this, "PERF", "transform", 0, "B", k, to_string(k));
                V val = transform(range.first, range.second);
                CGAL_DDT_TRACE1_LOCK(*this, "PERF", "transform", 0, "E", value, val);
                return val;
        });

        CGAL_DDT_TRACE1(*this, "PERF", "reduce", "generic_work", "E", value, value);
        return value;
    }

    template<typename OutputValue,
             typename Container1,
             typename Container2,
             typename Transform,
             typename OutputIterator,
             typename... Args2>
    OutputIterator
    ranges_transform(Container1& c1, Container2& c2, Transform transform, OutputIterator out, Args2&&... args2)
    {
        CGAL_DDT_TRACE0(*this, "PERF", "transform", "generic_work", "B");
        typedef typename Container2::key_type key_type;
        std::vector<key_type> keys;
        get_unique_keys(c1, keys);

        std::for_each(CGAL_DDT_SCHEDULER_STD_SCHEDULER_PAR
            keys.begin(), keys.end(), [this, &c1, &c2, &transform, &out, &args2...](key_type k){
                std::unique_lock<std::mutex> lock(mutex);
                CGAL_DDT_TRACE1(*this, "LOCK", "mutex", "bad", "B", k, to_string(k));
                typename Container2::iterator it2 = c2.emplace(std::piecewise_construct,
                    std::forward_as_tuple(k),
                    std::forward_as_tuple(k, std::forward<Args2>(args2)...)).first;
                auto range1 = c1.equal_range(k);
                CGAL_DDT_TRACE0(*this, "LOCK", "mutex", "bad", "E");
                CGAL_DDT_TRACE2(*this, "PERF", "transform", 0, "B", k, to_string(k), in, to_summary(range1.first, range1.second));
                lock.unlock();

                std::vector<OutputValue> output;
                transform(range1.first, range1.second, it2->second, std::back_inserter(output));

                lock.lock();
                CGAL_DDT_TRACE2(*this, "PERF", "transform", 0, "E", inout, to_summary(range1.first, range1.second), out, to_summary(output.begin(), output.end()));
                CGAL_DDT_TRACE1(*this, "LOCK", "mutex", "bad", "B", k, to_string(k));
                out = std::move(output.begin(), output.end(), out);
                CGAL_DDT_TRACE0(*this, "LOCK", "mutex", "bad", "E");
        });

        CGAL_DDT_TRACE0(*this, "PERF", "transform", "generic_work", "E");
        return out;
    }

    template<typename Container1,
             typename Container2,
             typename Transform,
             typename OutputIterator1,
             typename... Args2>
    void ranges_for_each(Container1& c1, Container2& c2, Transform transform, OutputIterator1, Args2&&... args2)
    {
        CGAL_DDT_TRACE0(*this, "PERF", "for_each", "generic_work", "B");
        typedef typename Container1::key_type    key_type;
        typedef typename Container1::mapped_type mapped_type1;
        typedef typename Container1::value_type  value_type1;
        std::multimap<key_type, mapped_type1> m1[2];
        ranges_transform<value_type1>(c1, c2, transform, std::inserter(m1[0], m1[0].begin()), std::forward<Args2>(args2)...);
        for(int i = 0, j = 1; !m1[i].empty(); i = j, j = 1-i) {
            ranges_transform<value_type1>(m1[i], c2, transform, std::inserter(m1[j], m1[j].begin()), std::forward<Args2>(args2)...);
            m1[i].clear();
        }
        CGAL_DDT_TRACE0(*this, "PERF", "for_each", "generic_work", "B");
    }

#if __cplusplus >= 201703L
    ExecutionPolicy policy;
#endif

private:
    std::mutex mutex;

#ifdef CGAL_DDT_TRACING
public:
    typedef std::chrono::time_point<std::chrono::high_resolution_clock> clock_type;
    static constexpr int process_index() { return 0; }
    std::thread::id thread_index() const { return std::this_thread::get_id(); }
    std::size_t clock_microsec() const { return std::chrono::duration<double, std::micro>(clock_now() - trace.t0).count(); }
    clock_type clock_now() const { return std::chrono::high_resolution_clock::now(); }
    trace_logger<clock_type> trace;
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
