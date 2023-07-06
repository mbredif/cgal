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
#include <tbb/concurrent_vector.h>
#include <tbb/parallel_reduce.h>
#include <tbb/task_arena.h>
#include <CGAL/DDT/Tile.h>

namespace CGAL {
namespace DDT {

namespace Impl {
template<typename Container, typename V, typename Transform, typename Reduce, typename Key>
V transform_reduce_id(Container& c, V value, Transform transform, Reduce reduce, Key k, std::mutex& mutex)
{
    std::unique_lock<std::mutex> lock(mutex);
    typename Container::iterator it = c.find(k);
    lock.unlock();

    return reduce(value, transform(k, it->second));
}

template< typename Container1, typename Container2, typename V, typename Transform, typename Reduce, typename Key, typename... Args>
V transform_zip_id(Container1& c1, Container2& c2, V value, Transform transform, Reduce reduce, Key k, std::mutex& mutex, Args&&... args)
{
    std::unique_lock<std::mutex> lock(mutex);
    typename Container1::iterator it1 = c1.try_emplace(k, k, std::forward<Args>(args)...).first;
    typename Container2::iterator it2 = c2.find(k);
    lock.unlock();

    value = reduce(value, transform(k, it1->second, it2->second));

    lock.lock();
    c2.send_points(k);

    return value;
}

}

/// \ingroup PkgDDTSchedulerClasses
/// \cgalModels Scheduler
struct TBB_scheduler
{
    TBB_scheduler(int max_concurrency = 0) : arena(max_concurrency ? max_concurrency : tbb::task_arena::automatic) {}

    inline int max_concurrency() const { return arena.max_concurrency(); }

    template<typename Container,
             typename Transform,
             typename V,
             typename Reduce = std::plus<>>
    V transform_reduce(Container& c, V init, Transform transform, Reduce reduce = {})
    {
        typedef typename Container::key_type key_type;
        std::vector<key_type> keys;
        for(const auto& [k,v] : c) keys.push_back(k);
        keys.erase(std::unique(keys.begin(), keys.end()), keys.end()); // erase duplicates, if any
        return arena.execute([&]{
            return tbb::parallel_reduce(
                  tbb::blocked_range<int>(0,keys.size()),
                  init, [&](tbb::blocked_range<int> r, V value)
            {
                for (int i=r.begin(); i<r.end(); ++i)
                    value = Impl::transform_reduce_id(c, value, transform, reduce, keys[i], mutex);
                return value;
            }, reduce);
        });
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
        std::vector<key_type> keys;
        {
            std::unique_lock<std::mutex> lock(mutex);
            for (const auto& [k,v] : c2) keys.push_back(k);
            keys.erase(std::unique(keys.begin(), keys.end()), keys.end()); // erase duplicates, if any
        }
        return arena.execute([&]{
            return tbb::parallel_reduce(
                  tbb::blocked_range<int>(0,keys.size()),
                  init, [&](tbb::blocked_range<int> r, V value)
            {
                for (int i=r.begin(); i<r.end(); ++i)
                    value = Impl::transform_zip_id(c1, c2, value, transform, reduce, keys[i], mutex, std::forward<Args>(args)...);
                return value;
            }, reduce);
        });
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

private:
    tbb::task_arena arena;
    std::mutex mutex;
};

}
}

#endif // CGAL_DDT_SCHEDULER_TBB_SCHEDULER_H
