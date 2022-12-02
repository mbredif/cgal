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

#include <map>
#include <set>
#include <vector>
#include <chrono>
#include <CGAL/DDT/scheduler/multithread_scheduler/thread_pool.h>

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTSchedulerClasses
/// \cgalModels Scheduler
template<typename T>
struct Multithread_scheduler
{
    typedef T Tile;
    typedef typename Tile::Vertex_const_handle_and_id Vertex_const_handle_and_id;
    typedef typename Tile::Vertex_const_handle Vertex_const_handle;
    typedef typename Tile::Point_id Point_id;
    typedef typename Tile::Point Point;
    typedef typename Tile::Id Id;
    typedef std::vector<Point_id> Point_id_container;

    /// constructor
    Multithread_scheduler(int n_threads = 0) : pool(n_threads), timeout_(1)
    {
        pool.init();
    }
    template<class Duration>
    Multithread_scheduler(int n_threads, Duration timeout) : pool(n_threads), timeout_(timeout)
    {
        pool.init();
    }
    inline int number_of_threads() const
    {
        return pool.number_of_threads();
    }
    ~Multithread_scheduler()
    {
        pool.shutdown();
    }

    inline void receive(Id id, Point_id_container& received)
    {
        inbox[id].swap(received);
        allbox_sent.emplace(id, 0);
        size_t size0 = received.size();
        allbox.copy_after(received, allbox_sent[id]);
        size_t size1 = received.size();
        allbox_sent[id] += size1 - size0;
#ifdef CGAL_DEBUG_DDT
        std::unique_lock<std::mutex> cout_lock(cout_mutex);
        std::cout << int(id) << " : " << size0 << " + " << (size1-size0) << std::endl;
#endif
    }

    void send(const Point& p, Id id, Id target)
    {
        inbox[target].emplace_back(p,id);
    }

    int send_one(const Tile& tile, const std::map<Id, std::set<Vertex_const_handle>>& vertices)
    {
        Point_id_container points;
        Id source = tile.id();
        int count = 0;
        for(auto& vi : vertices)
        {
            points.clear();
            Id target = vi.first;
            assert(target != source);
            for(Vertex_const_handle v : vi.second)
            {
                Id vid = tile.id(v);
                assert(target != vid);
                const Point& p = tile.point(v);
                if(sent_[source][target].insert(std::make_pair(p,vid)).second)
                {
                    ++count;
                    points.emplace_back(p, vid);
                }
            }
            inbox[target].append(points);
        }
        return count;
    }

    int send_all(const Tile& tile, const std::vector<Vertex_const_handle>& vertices)
    {
        Point_id_container points;
        for(Vertex_const_handle v : vertices)
            points.emplace_back(tile.point(v),tile.id(v));
        allbox.append(points);
        return points.size();
    }

    template<typename TileContainer, typename Id_iterator, typename UnaryOp, typename V = int, typename BinaryOp = std::plus<>>
    V for_each(TileContainer& tc, Id_iterator begin, Id_iterator end, UnaryOp op1, BinaryOp op2 = {}, V init = {})
    {
        std::function<V(Id)> func = [this, &tc, &op1](Id id)
        {
            typename TileContainer::Tile_iterator tile;
            {
                std::unique_lock<std::mutex> lock(tc_mutex);
                tile = tc.load(id);
                tile->in_use = true;
            }
            V count = op1(*tile);
            {
                std::unique_lock<std::mutex> lock(tc_mutex);
                tile->in_use = false;
            }
            return count;
        };

        // ensure sent_ has all the id inserted to prevent race conditions
        for(Id_iterator it = begin; it != end; ++it)
        {
            sent_.emplace(*it, std::map<Id, std::set<Point_id>>{});
            allbox_sent.emplace(*it, 0);
        }

        std::vector<std::future<V>> futures;
        for(Id_iterator it = begin; it != end; ++it)
            futures.push_back(pool.submit(func, *it));

        V value = init;
        for(auto& f: futures) value = op2(value, f.get());
        return value;
    }

    template<typename TileContainer, typename UnaryOp, typename V = int, typename BinaryOp = std::plus<>>
    V for_all(TileContainer& tc, UnaryOp op1, BinaryOp op2 = {}, V init = {})
    {
        std::vector<Id> ids;
        {
            std::unique_lock<std::mutex> lock(tc_mutex);
            ids.insert(ids.end(), tc.tile_ids_begin(), tc.tile_ids_end());
        }
        return for_each(tc, ids.begin(), ids.end(), op1, op2, init);
    }

    template<typename TileContainer, typename UnaryOp, typename V = int, typename BinaryOp = std::plus<>>
    V for_each(TileContainer& tc, UnaryOp op1, BinaryOp op2 = {}, V init = {})
    {
        std::set<Id> ids;
        size_t n = allbox.size();
        for(auto& it : allbox_sent)
            if (it.second != n)
                ids.insert(it.first);

        for(auto& it : inbox)
            if (!it.second.empty())
                ids.insert(it.first);

        return for_each(tc, ids.begin(), ids.end(), op1, op2, init);
    }

    // no barrier between each epoch, busy tiles are skipped
    template<typename TileContainer, typename UnaryOp, typename V = int, typename BinaryOp = std::plus<>>
    V for_each_rec(TileContainer& tc, UnaryOp op1, BinaryOp op2 = {}, V init = {})
    {
        std::function<V(Id)> func = [this, &tc, &op1](Id id)
        {
            typename TileContainer::Tile_iterator tile;
            {
                std::unique_lock<std::mutex> lock(tc_mutex);
                tile = tc.load(id);
                tile->in_use = true;
            }
            V count = op1(*tile);
            {
                std::unique_lock<std::mutex> lock(tc_mutex);
                tile->in_use = false;
            }
            return count;
        };

        V count = init;
        std::map<Id, std::future<V>> futures;
        do {
            auto fit = futures.begin();
            while(fit!=futures.end())
            {
                if (fit->second.wait_for(timeout_) != std::future_status::ready) {
                    ++fit;
                } else {
                    count = op2(count, fit->second.get());
                    futures.erase(fit++);
                }
            }
            for(const auto& it : inbox)
            {
                Id id = it.first;
                if (!it.second.empty() && futures.count(id) == 0)
                    futures[id] = pool.submit(func, id);
            }
            size_t n = allbox.size();
            for(const auto& it : allbox_sent)
            {
                Id id = it.first;
                if (it.second != n && futures.count(id) == 0)
                    futures[id] = pool.submit(func, id);
            }
        } while (!futures.empty());
        return count;
    }

private:
    safe<Point_id_container> allbox;
    std::map<Id, size_t> allbox_sent;
    std::map<Id, safe<Point_id_container>> inbox;
    std::map<Id, std::map<Id, std::set<Point_id>>> sent_; // no race condition, as the first Id is the source tile id

    thread_pool pool;
    std::chrono::milliseconds timeout_;

    std::mutex tc_mutex;
#ifdef CGAL_DEBUG_DDT
    std::mutex cout_mutex;
#endif
};

}
}

#endif // CGAL_DDT_SCHEDULER_MULTITHREAD_SCHEDULER_H
