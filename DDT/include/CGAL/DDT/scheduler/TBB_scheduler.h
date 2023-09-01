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

#ifndef CGAL_DDT_SCHEDULER_TBB_SCHEDULER_H
#define CGAL_DDT_SCHEDULER_TBB_SCHEDULER_H

#ifndef CGAL_LINKED_WITH_TBB
#error TBB not properly setup with CGAL
#endif

#include <map>
#include <set>
#include <vector>
#include <mutex>
#include <functional>
#include <tbb/blocked_range.h>
#include <tbb/parallel_reduce.h>
#include <tbb/parallel_for_each.h>
#include <tbb/task_arena.h>

#include <CGAL/DDT/IO/trace_logger.h>

namespace CGAL {
namespace DDT {

template<typename T, typename Keys>
std::string to_summary(const tbb::blocked_range<T>& r, const Keys& keys) {
    std::ostringstream out;
    for (T i=r.begin(); i<r.end(); ++i)
        out << ", " << std::to_string(keys.at(i));
    out << " ]";
    std::string s = out.str();
    s[0] = '[';
    return s;
}

/// \ingroup PkgDDTSchedulerClasses
/// \cgalModels Scheduler
struct TBB_scheduler
{

    TBB_scheduler(int max_concurrency = 0)
        : arena(max_concurrency ? max_concurrency : tbb::task_arena::automatic)
    {
        arena.initialize();
    }

    inline int max_concurrency() const { return arena.max_concurrency(); }

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
             typename OutputIterator,
             typename Transform>
    OutputIterator ranges_transform(Container& c, Transform transform, OutputIterator out)
    {
        CGAL_DDT_TRACE0(*this, "PERF", "transform", "generic_work", "B");
        typedef typename Container::key_type key_type;
        std::vector<key_type> keys;
        get_unique_keys(c, keys);

        arena.execute([this, &c, &out, &transform, &keys]{
            tbb::parallel_for_each(
                keys,
                [this, &c, &out, &transform](key_type k)
                {
                    auto range = c.equal_range(k);
                    std::vector<OutputValue> output;
                    CGAL_DDT_TRACE1_LOCK(*this, "PERF", "transform", 0, "B", k, to_string(k));
                    transform(range.first, range.second, std::back_inserter(output));
                    std::unique_lock<std::mutex> lock(mutex);
                    CGAL_DDT_TRACE1(*this, "PERF", "transform", 0, "E", sizes, to_summary(output.begin(), output.end()));
                    CGAL_DDT_TRACE1(*this, "LOCK", "mutex", "bad", "B", k, to_string(k));
                    out = std::move(output.begin(), output.end(), out);
                    CGAL_DDT_TRACE0(*this, "LOCK", "mutex", "bad", "E");
                }
            );
        });
        CGAL_DDT_TRACE0(*this, "PERF", "transform", "generic_work", "E");
        return out;
    }

    template<typename OutputValue,
             typename Container,
             typename OutputIterator,
             typename V,
             typename Reduce,
             typename Transform>
    std::pair<V,OutputIterator>
    ranges_transform_reduce(Container& c, Transform transform, V value, Reduce reduce, OutputIterator out)
    {
        CGAL_DDT_TRACE0(*this, "PERF", "transform_reduce", "generic_work", "B");
        typedef typename Container::key_type key_type;
        std::vector<key_type> keys;
        get_unique_keys(c, keys);

        value = arena.execute([this, &c, &out, &value, &reduce, &transform, &keys]{
            return tbb::parallel_reduce(
                tbb::blocked_range<int>(0,keys.size()),
                value,
                [this, &c, &out, &value, &reduce, &transform, &keys](tbb::blocked_range<int> r, V val)
                {
                    CGAL_DDT_TRACE1_LOCK(*this, "PERF", "range", 0, "B", range, to_summary(r, keys));
                    for (int i=r.begin(); i<r.end(); ++i)
                    {
                        key_type k = keys[i];
                        auto range = c.equal_range(k);
                        std::vector<OutputValue> output;
                        CGAL_DDT_TRACE1_LOCK(*this, "PERF", "transform", 0, "B", k, to_string(k));
                        auto res = transform(range.first, range.second, std::back_inserter(output));
                        val = reduce(val, res.first);
                        std::unique_lock<std::mutex> lock(mutex);
                        CGAL_DDT_TRACE2(*this, "PERF", "transform", 0, "E", value, res.first, sizes, to_summary(output.begin(), output.end()));
                        CGAL_DDT_TRACE1(*this, "LOCK", "mutex", "bad", "B", k, to_string(k));
                        out = std::move(output.begin(), output.end(), out);
                        CGAL_DDT_TRACE0(*this, "LOCK", "mutex", "bad", "E");
                    }

                    CGAL_DDT_TRACE1_LOCK(*this, "PERF", "range", 0, "E", value, val);
                    return val;
                },
                reduce
            );
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

        value = arena.execute([this, &c, &value, &transform, &reduce, &keys]{
            return tbb::parallel_reduce(
                tbb::blocked_range<int>(0,keys.size()),
                value,
                [this, &c, &transform, &reduce, &keys](tbb::blocked_range<int> r, V val)
                {
                    CGAL_DDT_TRACE1_LOCK(*this, "PERF", "range", 0, "B", range, to_summary(r, keys));
                    for (int i=r.begin(); i<r.end(); ++i) {
                        key_type k = keys[i];
                        auto range = c.equal_range(k);
                        CGAL_DDT_TRACE1_LOCK(*this, "PERF", "transform", 0, "B", k, to_string(k));
                        V v = transform(range.first, range.second);
                        CGAL_DDT_TRACE1_LOCK(*this, "PERF", "transform", 0, "E", value, v);
                        val = reduce(val, v);
                    }
                    CGAL_DDT_TRACE1_LOCK(*this, "PERF", "range", 0, "E", value, val);
                    return val;
                },
                reduce
            );
        });
        CGAL_DDT_TRACE1(*this, "PERF", "reduce", "generic_work", "E", value, value);
        return value;
    }

    template<typename OutputValue3,
             typename Container1,
             typename Container2,
             typename OutputIterator3,
             typename Transform,
             typename... Args2>
    OutputIterator3
    ranges_transform(Container1& c1, Container2& c2, Transform transform, OutputIterator3 out3, Args2&&... args2)
    {
        CGAL_DDT_TRACE0(*this, "PERF", "transform", "generic_work", "B");
        typedef typename Container2::key_type key_type;
        std::vector<key_type> keys;
        get_unique_keys(c1, keys);

        arena.execute([this, &c1, &c2, &transform, &out3, &args2..., &keys]{
            tbb::parallel_for_each(
                keys,
                [this, &c1, &c2, &transform, &out3, &args2...](key_type k)
                {
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
                }
            );
        });
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
        typedef typename Container1::mapped_type mapped_type1;
        typedef typename Container2::mapped_type mapped_type2;
        typedef std::pair<key_type, mapped_type1> OutputValue1;
        std::set<key_type> keys; // maintains keys from c1 that have not yet finished processing
        get_unique_keys(c1, keys);
        std::multimap<key_type, mapped_type1> m1; // extra values to be processed (beyond those of c1)

        arena.execute([this, &c1, &c2, &transform, &m1, &args2..., &keys]{
            tbb::parallel_for_each(keys, [this, &c1, &c2, &transform, &m1, &args2..., &keys](key_type k, tbb::feeder<key_type>& feeder){
                std::unique_lock<std::mutex> lock(mutex);
                CGAL_DDT_TRACE1(*this, "PERF", "item", "generic_work", "B", k, to_string(k));
                CGAL_DDT_TRACE1(*this, "LOCK", "mutex", "bad", "B", k, to_string(k));
                typename Container2::iterator it2 = c2.find(k);
                if (keys.find(k) != keys.end()) {
                    // process c1.equal_range(k)
                    it2 = c2.emplace(std::piecewise_construct,
                        std::forward_as_tuple(k),
                        std::forward_as_tuple(k, std::forward<Args2>(args2)...)).first;
                    auto range1 = c1.equal_range(k);
                    CGAL_DDT_TRACE0(*this, "LOCK", "mutex", "bad", "E");
                    CGAL_DDT_TRACE2(*this, "PERF", "transform", 0, "B", k, to_string(k), in, to_summary(range1.first, range1.second));
                    lock.unlock();

                    std::vector<OutputValue1> output1;
                    transform(range1.first, range1.second, it2->second, std::back_inserter(output1));

                    lock.lock();
                    CGAL_DDT_TRACE2(*this, "PERF", "transform", 0, "E", inout, to_summary(range1.first, range1.second), out, to_summary(output1.begin(), output1.end()));
                    CGAL_DDT_TRACE1(*this, "LOCK", "mutex", "bad", "B", k, to_string(k));
                    for (const auto& kv : output1)
                    {
                        key_type k1 = kv.first;
                        if (keys.find(k1) == keys.end() && m1.find(k1) == m1.end())
                            feeder.add(k1);
                        m1.insert(std::move(kv));
                    }
                }
                mapped_type2& v2 = it2->second;
                while(!m1.empty())
                {
                    auto range1 = m1.equal_range(k);
                    if (range1.first == range1.second) break;
                    std::vector<value_type1> input1;
                    for(auto it1 = range1.first; it1 != range1.second; ++it1)
                        input1.emplace_back(it1->first, std::move(it1->second));
                    m1.erase(range1.first, range1.second);
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
                        key_type k1 = kv.first;
                        if (keys.find(k1) == keys.end() && m1.find(k1) == m1.end())
                            feeder.add(k1);
                        m1.insert(std::move(kv));
                    }
                }
                keys.erase(k);
                CGAL_DDT_TRACE0(*this, "LOCK", "mutex", "bad", "E");
                CGAL_DDT_TRACE0(*this, "PERF", "item", "generic_work", "E");
            });
        });
        CGAL_DDT_TRACE0(*this, "PERF", "for_each", "generic_work", "E");
        */
    }

private:
    tbb::task_arena arena;
    std::mutex mutex;

#ifdef CGAL_DDT_TRACING
public:
    typedef tbb::tick_count clock_type;
    static constexpr int process_index() { return 0; }
    int thread_index() const { return tbb::this_task_arena::current_thread_index(); }
    std::size_t clock_microsec() const { return 1e6*(clock_now() - trace.t0).seconds(); }
    clock_type clock_now() const { return tbb::tick_count::now(); }
    trace_logger<clock_type> trace;
#endif
};

}
}

#endif // CGAL_DDT_SCHEDULER_TBB_SCHEDULER_H
