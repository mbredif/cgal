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

#define CGAL_DDT_TRACE
#include <CGAL/DDT/IO/trace_logger.h>

namespace CGAL {
namespace DDT {

template<typename T, typename Keys>
std::string to_string(const tbb::blocked_range<T>& r, const Keys& keys) {
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

    template<typename OutputValue,
             typename Container,
             typename OutputIterator,
             typename Transform>
    OutputIterator flat_map(Container& c, OutputIterator out, Transform transform)
    {
        CGAL_DDT_TRACE0("PERF", "flat_map", "generic_work", "B");
        typedef typename Container::key_type key_type;
        std::vector<key_type> keys;
        for(const auto& [k,v] : c) keys.push_back(k);

        arena.execute([&, &c, &out, &transform, &keys]{
            return tbb::parallel_for_each(
                keys,
                [&, &c, &out, &transform](key_type k)
                {
                    auto it = c.find(k);
                    std::vector<std::pair<key_type,OutputValue>> tmp;
                    CGAL_DDT_TRACE1_LOCK("PERF", "transform", 0, "B", k, to_string(k));
                    transform(it->first, it->second, std::back_inserter(tmp));
                    std::unique_lock<std::mutex> lock(mutex);
                    CGAL_DDT_TRACE0("PERF", "transform", 0, "E");
                    CGAL_DDT_TRACE1("LOCK", "mutex", "bad", "B", k, to_string(k));
                    out = std::move(tmp.begin(), tmp.end(), out);
                    CGAL_DDT_TRACE0("LOCK", "mutex", "bad", "E");
                }
            );
        });
        CGAL_DDT_TRACE0("PERF", "flat_map", "generic_work", "E");
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
        CGAL_DDT_TRACE0("PERF", "reduce_by_key", "generic_work", "B");
        typedef typename Container::key_type key_type;
        std::vector<key_type> keys;
        for(const auto& [k,v] : c) keys.push_back(k);
        keys.erase(std::unique(keys.begin(), keys.end()), keys.end());

        value = arena.execute([&, &c, &out, &value, &reduce, &transform, &keys]{
            return tbb::parallel_reduce(
                tbb::blocked_range<int>(0,keys.size()),
                value,
                [&, &c, &out, &value, &reduce, &transform, &keys](tbb::blocked_range<int> r, V val)
                {
                    CGAL_DDT_TRACE1_LOCK("PERF", "range", 0, "B", range, CGAL::DDT::to_string(r, keys));
                    for (int i=r.begin(); i<r.end(); ++i)
                    {
                        key_type k = keys[i];
                        auto range = c.equal_range(k);
                        std::vector<std::pair<key_type,OutputValue>> tmp;
                        CGAL_DDT_TRACE1_LOCK("PERF", "transform", 0, "B", k, to_string(k));
                        auto res = transform(range, std::back_inserter(tmp));
                        val = reduce(val, res.first);
                        std::unique_lock<std::mutex> lock(mutex);
                        CGAL_DDT_TRACE1("PERF", "transform", 0, "E", value, res.first);
                        CGAL_DDT_TRACE1("LOCK", "mutex", "bad", "B", k, to_string(k));
                        out = std::move(tmp.begin(), tmp.end(), out);
                        CGAL_DDT_TRACE0("LOCK", "mutex", "bad", "E");
                    }

                    CGAL_DDT_TRACE1_LOCK("PERF", "range", 0, "E", value, val);
                    return val;
                },
                reduce
            );
        });
        CGAL_DDT_TRACE1("PERF", "reduce_by_key", "generic_work", "E", value, value);
        return { value, out };
    }

    template<typename Container,
             typename V,
             typename Reduce,
             typename Transform>
    V transform_reduce(Container& c, V value, Reduce reduce, Transform transform)
    {
        CGAL_DDT_TRACE0("PERF", "transform_reduce", "generic_work", "B");
        typedef typename Container::key_type key_type;
        std::vector<key_type> keys;
        for(const auto& [k,v] : c) keys.push_back(k);

        value = arena.execute([&, &c, &value, &transform, &reduce, &keys]{
            return tbb::parallel_reduce(
                tbb::blocked_range<int>(0,keys.size()),
                value,
                [&, &c, &transform, &reduce, &keys](tbb::blocked_range<int> r, V val)
                {
                    CGAL_DDT_TRACE1_LOCK("PERF", "range", 0, "B", range, CGAL::DDT::to_string(r, keys));
                    for (int i=r.begin(); i<r.end(); ++i) {
                        key_type k = keys[i];
                        auto it = c.find(k);
                        CGAL_DDT_TRACE1_LOCK("PERF", "transform", 0, "B", k, to_string(k));
                        V v = transform(it->first, it->second);
                        CGAL_DDT_TRACE1_LOCK("PERF", "transform", 0, "E", value, v);
                        val = reduce(val, v);
                    }
                    CGAL_DDT_TRACE1_LOCK("PERF", "range", 0, "E", value, val);
                    return val;
                },
                reduce
            );
        });
        CGAL_DDT_TRACE1("PERF", "transform_reduce", "generic_work", "E", value, value);
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
        CGAL_DDT_TRACE0("PERF", "join_transform_reduce", "generic_work", "B");
        typedef typename Container1::key_type key_type;
        std::vector<key_type> keys;
        for(const auto& [k,v] : c2) keys.push_back(k);
        arena.execute([&, &c1, &c2, &transform, &args..., &keys, &out]{
            return tbb::parallel_for(
                tbb::blocked_range<int>(0,keys.size()),
                [&, &c1, &c2, &transform, &args..., &keys, &out](tbb::blocked_range<int> r)
                {
                    std::unique_lock<std::mutex> lock(mutex);
                    CGAL_DDT_TRACE1("PERF", "range", 0, "B", range, CGAL::DDT::to_string(r, keys));
                    for (int i=r.begin(); i<r.end(); ++i)
                    {
                        key_type k = keys[i];
                        CGAL_DDT_TRACE1("LOCK", "mutex", "bad", "B", k, to_string(k));
                        typename Container1::iterator it1 = c1.emplace(std::piecewise_construct,
                            std::forward_as_tuple(k),
                            std::forward_as_tuple(k, std::forward<Args>(args)...)).first;
                        typename Container2::iterator it2 = c2.find(k);
                        CGAL_DDT_TRACE0("LOCK", "mutex", "bad", "E");
                        CGAL_DDT_TRACE1("PERF", "transform", 0, "B", k, to_string(k));
                        lock.unlock();

                        std::vector<typename Container2::value_type> c3;
                        transform(k, it1->second, it2->second, std::back_inserter(c3));

                        lock.lock();
                        CGAL_DDT_TRACE0("PERF", "transform", 0, "E");
                        CGAL_DDT_TRACE1("LOCK", "mutex", "bad", "B", k, to_string(k));
                        out = std::move(c3.begin(), c3.end(), out);
                        CGAL_DDT_TRACE0("LOCK", "mutex", "bad", "E");
                    }
                    CGAL_DDT_TRACE0("PERF", "range", 0, "E");
                }
            );
        });
        CGAL_DDT_TRACE0("PERF", "join_transform_reduce", "generic_work", "E");
        return out;
    }


    template<typename Container1,
             typename Container2,
             typename OutputIterator2,
             typename Transform,
             typename... Args>
    void join_transform_reduce_loop(Container1& c1, Container2& c2, OutputIterator2 out2, Transform transform, Args&&... args)
    {
        CGAL_DDT_TRACE0("PERF", "join_transform_reduce_loop", "generic_work", "B");
        typedef typename Container2::key_type    key_type;
        typedef typename Container2::value_type  value_type2;
        typedef typename Container2::mapped_type mapped_type2;
        typedef typename Container1::mapped_type mapped_type1;
        std::set<key_type> keys;
        for (const auto& [k,v] : c2) keys.insert(k);

        arena.execute([&, &c1, &c2, &out2, &transform, &args..., &keys]{
            tbb::parallel_for_each(keys, [&, &c1, &c2, &out2, &transform, &args..., &keys](key_type k, tbb::feeder<key_type>& feeder){
                std::unique_lock<std::mutex> lock(mutex);
                CGAL_DDT_TRACE1("PERF", "item", "generic_work", "B", k, to_string(k));
                CGAL_DDT_TRACE1("LOCK", "mutex", "bad", "B", k, to_string(k));
                typename Container1::iterator it1 = c1.emplace(std::piecewise_construct,
                            std::forward_as_tuple(k),
                            std::forward_as_tuple(k, std::forward<Args>(args)...)).first;

                mapped_type1& v1 = it1->second;
                while(true)
                {
                    typename Container2::iterator it2 = c2.find(k);
                    if (it2 == c2.end()) break;
                    mapped_type2 v2;
                    std::swap(it2->second, v2);
                    c2.erase(it2);
                    CGAL_DDT_TRACE0("LOCK", "mutex", "bad", "E");
                    CGAL_DDT_TRACE1("PERF", "transform", 0, "B", k, to_string(k));
                    lock.unlock();

                    std::vector<value_type2> c3;
                    transform(k, v1, v2, std::back_inserter(c3));

                    lock.lock();
                    CGAL_DDT_TRACE0("PERF", "transform", 0, "E");
                    CGAL_DDT_TRACE1("LOCK", "mutex", "bad", "B", k, to_string(k));
                    for (const auto& kv : c3)
                    {
                        key_type k3 = kv.first;
                        *out2++ = std::move(kv);
                        if (keys.find(k3) == keys.end()) {
                            keys.insert(k3);
                            feeder.add(k3);
                        }
                    }
                }
                keys.erase(k);
                CGAL_DDT_TRACE0("LOCK", "mutex", "bad", "E");
                CGAL_DDT_TRACE0("PERF", "item", "generic_work", "E");
            });
        });
        CGAL_DDT_TRACE0("PERF", "join_transform_reduce_loop", "generic_work", "E");
    }

private:
    tbb::task_arena arena;
    std::mutex mutex;
#ifdef CGAL_DDT_TRACE
    trace_logger trace;
#endif
};

}
}

#endif // CGAL_DDT_SCHEDULER_TBB_SCHEDULER_H
