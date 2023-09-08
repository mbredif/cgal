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

#ifndef CGAL_DDT_SCHEDULER_MULTITHREAD_SCHEDULER_H
#define CGAL_DDT_SCHEDULER_MULTITHREAD_SCHEDULER_H

#include <CGAL/DDT/scheduler/multithread_scheduler/thread_pool.h>
#include <functional>
#include <chrono>

#include <CGAL/DDT/IO/trace_logger.h>

namespace CGAL {
namespace DDT {

// \ingroup PkgDDTSchedulerClasses
// \cgalModels Scheduler
struct Multithread_scheduler
{
    /// constructor
    Multithread_scheduler(int max_concurrency = 0) : pool(max_concurrency), timeout_(1)
    {
        pool.init();
    }

    template<class Duration>
    Multithread_scheduler(int max_concurrency, Duration timeout) : pool(max_concurrency), timeout_(timeout)
    {
        pool.init();
    }

    ~Multithread_scheduler()
    {
        pool.shutdown();
    }

    inline int max_concurrency() const
    {
        return pool.max_concurrency();
    }

    template<typename Container, typename Key>
    void get_unique_keys(Container& c, std::vector<Key>& keys) const { // TODO const Container&
        for(const auto& [k,v] : c)
            keys.push_back(k);
        keys.erase(std::unique(keys.begin(), keys.end()), keys.end());
    }

    template<typename Container, typename Key>
    void get_unique_keys(Container& c, std::set<Key>& keys) const { // TODO const Container&
        for(const auto& [k,v] : c)
            keys.insert(k);
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

        std::vector<std::future<void>> futures;
        {
            std::unique_lock<std::mutex> lock(mutex);
            for(key_type k : keys)
                futures.push_back(pool.submit([this, &c, &out, &transform](key_type k) {
                    auto range = c.equal_range(k);
                    std::vector<OutputValue> output;
                    CGAL_DDT_TRACE1_LOCK(*this, "PERF", "transform", 0, "B", k, to_string(k));
                    transform(range.first, range.second, std::back_inserter(output));
                    std::unique_lock<std::mutex> lock(mutex);
                    CGAL_DDT_TRACE1(*this, "PERF", "transform", 0, "E", sizes, to_summary(output.begin(), output.end()));
                    CGAL_DDT_TRACE1(*this, "LOCK", "mutex", "bad", "B", k, to_string(k));
                    out = std::move(output.begin(), output.end(), out);
                    CGAL_DDT_TRACE0(*this, "LOCK", "mutex", "bad", "E");
                }, k));
        }
        for(auto& f: futures) f.get();
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

        std::vector<std::future<V>> futures;
        {
            std::unique_lock<std::mutex> lock(mutex);
            for(key_type k : keys)
                futures.push_back(pool.submit([this, &c, &transform, &out](key_type k) {
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
                }, k));
        }
        for(auto& f: futures) value = reduce(value, f.get());

        CGAL_DDT_TRACE1(*this, "PERF", "transform_reduce", "generic_work", "E", value, value);
        return { value, out };
    }

    template<typename Container,
             typename Transform,
             typename V,
             typename Reduce>
    V ranges_reduce(Container& c, Transform transform, V value, Reduce reduce)
    {
        CGAL_DDT_TRACE0(*this, "PERF", "reduce", "generic_work", "B");
        typedef typename Container::key_type key_type;
        std::vector<key_type> keys;
        get_unique_keys(c, keys);

        std::vector<std::future<V>> futures;
        {
            std::unique_lock<std::mutex> lock(mutex);
            for(key_type k : keys)
                futures.push_back(pool.submit([this, &c, &transform](key_type k){
                    auto range = c.equal_range(k);
                    CGAL_DDT_TRACE1_LOCK(*this, "PERF", "transform", 0, "B", k, to_string(k));
                    V val = transform(range.first, range.second);
                    CGAL_DDT_TRACE1_LOCK(*this, "PERF", "transform", 0, "E", value, val);
                    return val;
                }, k));
        }
        for(auto& f: futures) value = reduce(value, f.get());

        CGAL_DDT_TRACE1(*this, "PERF", "reduce", "generic_work", "E", value, value);
        return value;
    }

    template<typename OutputValue3,
             typename Container1,
             typename Container2,
             typename Transform,
             typename OutputIterator3,
             typename... Args2>
    OutputIterator3
    ranges_transform(Container1& c1, Container2& c2, Transform transform, OutputIterator3 out3, Args2&&... args2)
    {
        CGAL_DDT_TRACE0(*this, "PERF", "transform", "generic_work", "B");
        typedef typename Container1::key_type key_type;
        std::vector<key_type> keys;
        get_unique_keys(c1, keys);

        std::vector<std::future<void>> futures;
        {
            std::unique_lock<std::mutex> lock(mutex);
            for(key_type k : keys)
                futures.push_back(pool.submit([this, &c1, &c2, &transform, &out3, &args2...](key_type k){
                    std::unique_lock<std::mutex> lock(mutex);
                    CGAL_DDT_TRACE1(*this, "LOCK", "mutex", "bad", "B", k, to_string(k));
                    typename Container2::iterator it2 = c2.emplace(std::piecewise_construct,
                        std::forward_as_tuple(k),
                        std::forward_as_tuple(k, std::forward<Args2>(args2)...)).first;
                    auto range1 = c1.equal_range(k);
                    CGAL_DDT_TRACE0(*this, "LOCK", "mutex", "bad", "E");
                    CGAL_DDT_TRACE2(*this, "PERF", "transform", 0, "B", k, to_string(k), in, to_summary(range1.first, range1.second));
                    lock.unlock();

                    std::vector<OutputValue3> v3;
                    transform(range1.first, range1.second, it2->second, std::back_inserter(v3));

                    lock.lock();
                    CGAL_DDT_TRACE2(*this, "PERF", "transform", 0, "E", inout, to_summary(range1.first, range1.second), out, to_summary(v3.begin(), v3.end()));
                    CGAL_DDT_TRACE1(*this, "LOCK", "mutex", "bad", "B", k, to_string(k));
                    out3 = std::move(v3.begin(), v3.end(), out3);
                    CGAL_DDT_TRACE0(*this, "LOCK", "mutex", "bad", "E");
                }, k));
        }
        for(auto& f: futures) f.get();
        CGAL_DDT_TRACE0(*this, "PERF", "transform", "generic_work", "E");
        return out3;
    }

    template<typename Container1,
             typename Container2,
             typename Transform,
             typename... Args2>
    void ranges_for_each(Container1& c1, Container2& c2, Transform transform, Args2&&... args2)
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
        CGAL_DDT_TRACE0(*this, "PERF", "for_each", "generic_work", "E");
        /*
        CGAL_DDT_TRACE0(*this, "PERF", "for_each", "generic_work", "B");
        typedef typename Container1::key_type    key_type;
        typedef typename Container1::value_type  value_type1;
        typedef typename Container2::mapped_type mapped_type2;
        std::set<key_type> keys;
        get_unique_keys(c1, keys);

        std::map<key_type, std::future<void>> futures;
        auto task = [this, &c1, &c2, &transform, &out1, &args2..., &keys](key_type k){
            std::unique_lock<std::mutex> lock(mutex);
            CGAL_DDT_TRACE1(*this, "PERF", "item", "generic_work", "B", k, to_string(k));
            CGAL_DDT_TRACE1(*this, "LOCK", "mutex", "bad", "B", k, to_string(k));
            typename Container2::iterator it2 = c2.emplace(std::piecewise_construct,
                        std::forward_as_tuple(k),
                        std::forward_as_tuple(k, std::forward<Args2>(args2)...)).first;

            mapped_type2& v2 = it2->second;
            while(true)
            {
                auto range1 = c1.equal_range(k);
                if (range1.first == range1.second) break;
                std::vector<value_type1> input1;
                for(auto it1 = range1.first; it1 != range1.second; ++it1)
                    input1.emplace_back(it1->first, std::move(it1->second));
                c1.erase(range1.first, range1.second);
                CGAL_DDT_TRACE0(*this, "LOCK", "mutex", "bad", "E");
                CGAL_DDT_TRACE2(*this, "PERF", "transform", 0, "B", k, to_string(k), in, to_summary(input1.begin(), input1.end()));
                lock.unlock();

                std::vector<value_type1> output1;
                transform(input1.begin(), input1.end(), v2, std::back_inserter(output1));

                lock.lock();
                CGAL_DDT_TRACE2(*this, "PERF", "transform", 0, "E", inout, to_summary(input1.begin(), input1.end()), out, to_summary(output1.begin(), output1.end()));
                CGAL_DDT_TRACE1(*this, "LOCK", "mutex", "bad", "B", k, to_string(k));
                for (const auto& kv : output1)
                {
                    keys.insert(kv.first);
                    *out1++ = std::move(kv);
                }
            }
            keys.erase(k);
            CGAL_DDT_TRACE0(*this, "LOCK", "mutex", "bad", "E");
            CGAL_DDT_TRACE0(*this, "PERF", "item", "generic_work", "E");
        };

        {
            std::unique_lock<std::mutex> lock(mutex);
            for(key_type k : keys)
                futures.emplace(k, pool.submit(task, k));
        }

        while (!futures.empty()) {
            auto fit = futures.begin();
            while(fit!=futures.end())
            {
                if (fit->second.wait_for(timeout_) != std::future_status::ready) {
                    ++fit;
                } else {
                    std::unique_lock<std::mutex> lock(mutex);
                    fit->second.get();
                    futures.erase(fit++);
                    for(auto k : keys)
                        if (futures.count(k) == 0)
                            futures.emplace(k, pool.submit(task, k));
                }
            }
        }

        CGAL_DDT_TRACE0(*this, "PERF", "for_each", "generic_work", "E");
        */
    }
    int thread_index() { return pool.thread_index(); }

private:
    thread_pool pool;
    std::chrono::milliseconds timeout_;
    std::mutex mutex;

#ifdef CGAL_DDT_TRACING
public:
    typedef std::chrono::time_point<std::chrono::high_resolution_clock> clock_type;
    std::size_t clock_microsec() const { return std::chrono::duration<double, std::micro>(clock_now() - trace.t0).count(); }
    clock_type clock_now() const { return std::chrono::high_resolution_clock::now(); }
    trace_logger<clock_type> trace = {"perf_mt.json", clock_now()};
    int process_index() const { return 0; }
#endif
};

}
}

#endif // CGAL_DDT_SCHEDULER_MULTITHREAD_SCHEDULER_H
