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
#include <CGAL/DDT/Tile.h>

namespace CGAL {
namespace DDT {

namespace Impl {
template< typename Container, typename Transform, typename V, typename Key>
V transform_id(Container& c, V init, Transform transform, Key k, std::mutex& mutex)
{
    typedef typename Container::iterator iterator;
    typedef typename Container::mapped_type T;
    std::unique_lock<std::mutex> lock(mutex);
    iterator it = c.find(k);
    it->second.locked = true;
    c.prepare_load(k, it->second);
    lock.unlock();

    T& v = it->second;
    V value = (c.safe_load(k, it->second)) ? transform(v) : init;

    lock.lock();
    it->second.locked = false;
    return value;
}


template< typename Container1, typename Container2, typename Transform, typename V, typename Key, typename... Args>
V transform_zip_id(Container1& c1, Container2& c2, V init, Transform transform, Key k, std::mutex& mutex, Args... args)
{
    typedef typename Container1::iterator iterator1;
    typedef typename Container1::mapped_type T1;
    typedef typename Container2::mapped_type T2;
    std::unique_lock<std::mutex> lock(mutex);
    iterator1 it = c1.emplace(k, std::move(T1(k, args...))).first;
    it->second.locked = true;
    c1.prepare_load(k, it->second);
    lock.unlock();

    T1& v1 = it->second;
    T2& v2 = c2[k];
    V value = (c1.safe_load(k, it->second)) ? transform(v1, v2) : init;

    lock.lock();
    c2.send_points(k);
    it->second.locked = false;
    return value;
}

}

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
    inline int max_concurrency() const
    {
        return pool.max_concurrency();
    }
    ~Multithread_scheduler()
    {
        pool.shutdown();
    }

    template<typename Container,
             typename V,
             typename Transform,
             typename Reduce = std::plus<>>
    V transform_reduce(Container& c, V init, Transform transform, Reduce reduce = {})
    {
        typedef typename Container::key_type key_type;
        std::vector<std::future<V>> futures;
        for(const auto& [k, v] : c)
            futures.push_back(pool.submit([this, &c, &init, &transform](key_type k){
                return Impl::transform_id(c, init, transform, k, mutex);
            }, k));
        V value = init;
        for(auto& f: futures) value = reduce(value, f.get());
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
        typedef typename Container1::key_type key_type;
        std::vector<std::future<V>> futures;
        {
            std::unique_lock<std::mutex> lock(mutex);
            for(const auto& [k, v2] : c2)
                futures.push_back(pool.submit([this, &c1, &c2, &init, &transform, &args...](key_type k){
                    return Impl::transform_zip_id(c1, c2, init, transform, k, mutex, args...);
                }, k));
        }
        V value = init;
        for(auto& f: futures) value = reduce(value, f.get());
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
        typedef typename Container1::key_type key_type;
        std::map<key_type, std::future<V>> futures;
        {
            std::unique_lock<std::mutex> lock(mutex);
            for(const auto& [k, v2] : c2)
                futures.emplace(k, pool.submit([this, &c1, &c2, &init, &transform, &args...](key_type k){
                    return Impl::transform_zip_id(c1, c2, init, transform, k, mutex, args...);
                }, k));
        }

        V value = init;
        while (!futures.empty()) {
            auto fit = futures.begin();
            while(fit!=futures.end())
            {
                if (fit->second.wait_for(timeout_) != std::future_status::ready) {
                    ++fit;
                } else {
                    std::unique_lock<std::mutex> lock(mutex);
                    value = reduce(value, fit->second.get());
                    futures.erase(fit++);
                    std::vector<key_type> keys;
                    for(const auto& v2 : c2)
                        if (!v2.second.points().at(v2.first).empty())
                            keys.push_back(v2.first);
                    for(auto k : keys)
                        if (futures.count(k) == 0)
                            futures.emplace(k, pool.submit([this, &c1, &c2, &init, &transform, &args...](key_type k){
                                return Impl::transform_zip_id(c1, c2, init, transform, k, mutex, args...);
                            }, k));
                }
            }
        }
        return value;
    }

private:
    thread_pool pool;
    std::chrono::milliseconds timeout_;
    std::mutex mutex;
};

}
}

#endif // CGAL_DDT_SCHEDULER_MULTITHREAD_SCHEDULER_H
