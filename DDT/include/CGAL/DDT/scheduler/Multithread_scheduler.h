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
V transform_id(Container& c, V value, Transform transform, Key k, std::mutex& mutex)
{
    std::unique_lock<std::mutex> lock(mutex);
    typename Container::iterator it = c.find(k);
    lock.unlock();

    return transform(k, it->second);
}


template< typename Container1, typename Container2, typename Transform, typename V, typename Key, typename... Args>
V transform_zip_id(Container1& c1, Container2& c2, V value, Transform transform, Key k, std::mutex& mutex, Args&&... args)
{
    std::unique_lock<std::mutex> lock(mutex);
    typename Container1::iterator it1 = c1.try_emplace(k, k, std::forward<Args>(args)...).first;
    typename Container2::iterator it2 = c2.find(k);
    lock.unlock();

    value = transform(k, it1->second, it2->second);

    lock.lock();
    c2.send_points(k);

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
        {
            std::unique_lock<std::mutex> lock(mutex);
            for(const auto& [k, _] : c)
                futures.push_back(pool.submit([this, &c, &init, &transform](key_type k){
                    return Impl::transform_id(c, init, transform, k, mutex);
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
    V join_transform_reduce(Container1& c1, Container2& c2, V init, Transform transform, Reduce reduce = {}, Args&&... args)
    {
        typedef typename Container1::key_type key_type;
        std::vector<std::future<V>> futures;
        {
            std::unique_lock<std::mutex> lock(mutex);
            for(const auto& [k, _] : c2)
                futures.push_back(pool.submit([this, &c1, &c2, &init, &transform, &args...](key_type k){
                    return Impl::transform_zip_id(c1, c2, init, transform, k, mutex, std::forward<Args>(args)...);
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
    V join_transform_reduce_loop(Container1& c1, Container2& c2, V init, Transform transform, Reduce reduce = {}, Args&&... args)
    {
        typedef typename Container1::key_type key_type;
        std::map<key_type, std::future<V>> futures;
        {
            std::unique_lock<std::mutex> lock(mutex);
            for(const auto& [k, _] : c2)
                futures.emplace(k, pool.submit([this, &c1, &c2, &init, &transform, &args...](key_type k){
                    return Impl::transform_zip_id(c1, c2, init, transform, k, mutex, std::forward<Args>(args)...);
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
                    for(const auto& [k, v2] : c2)
                        if (!v2.points().at(k).empty())
                            keys.push_back(k);
                    for(auto k : keys)
                        if (futures.count(k) == 0)
                            futures.emplace(k, pool.submit([this, &c1, &c2, &init, &transform, &args...](key_type k){
                                return Impl::transform_zip_id(c1, c2, init, transform, k, mutex, std::forward<Args>(args)...);
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
