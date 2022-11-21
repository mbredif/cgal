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
    Multithread_scheduler(int max_number_of_tiles, int n_threads = 0) : max_number_of_tiles(max_number_of_tiles), pool(n_threads), timeout_(1)
    {
        pool.init();
    }
    template<class Duration>
    Multithread_scheduler(int max_number_of_tiles, int n_threads, Duration timeout) : max_number_of_tiles(max_number_of_tiles), pool(n_threads), timeout_(timeout)
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

    int send_one(const Tile& tile, std::vector<Vertex_const_handle_and_id>& vertices)
    {
        std::map<Id, Point_id_container> points;
        Id source = tile.id();
        int count = 0;
        for(auto& vi : vertices)
        {
            Vertex_const_handle v = vi.first;
            Id target = vi.second;
            Id vid = tile.id(v);
            if(target == source || target == vid) continue;
            const Point& p = tile.point(v);
            if(sent_[source][target].insert(std::make_pair(p,vid)).second)
            {
                ++count;
                points[target].emplace_back(p, vid);
            }
        }
        for(auto& p : points) inbox[p.first].append(p.second);
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

    template<typename TileContainer, typename Id_iterator>
    int for_each(TileContainer& tc, Id_iterator begin, Id_iterator end, const std::function<int(Tile&)>& func)
    {
        std::function<int(Id)> func2 = [this, &tc, &func](Id id)
        {
            {
                std::unique_lock<std::mutex> lock(tc_mutex);
                tc.load(id);
            }
            int count = func(*(tc.get_tile(id)));
            {
                std::unique_lock<std::mutex> lock(tc_mutex);
                if(tc.number_of_tiles() >= max_number_of_tiles) {
                    if(!tc.save(id)) assert(false);
                    tc.unload(id);
                }
            }
            return count;
        };

        // ensure sent_ has all the id inserted to prevent race conditions
        for(Id_iterator it = begin; it != end; ++it)
        {
            sent_.emplace(*it, std::map<Id, std::set<Point_id>>{});
            allbox_sent.emplace(*it, 0);
        }

        std::vector<std::future<int>> futures;
        for(Id_iterator it = begin; it != end; ++it)
            futures.push_back(pool.submit(func2, *it));

        int count = 0;
        for(auto& f: futures) count += f.get();
        return count;
    }

    template<typename TileContainer>
    int for_all(TileContainer& tc, const std::function<int(Tile&)>& func)
    {
        return for_each(tc, tc.tile_ids_begin(), tc.tile_ids_end(), func);
    }

    template<typename TileContainer>
    int for_each(TileContainer& tc, const std::function<int(Tile&)>& func)
    {
        std::set<Id> ids;
        size_t n = allbox.size();
        for(auto& it : allbox_sent)
            if (it.second != n)
                ids.insert(it.first);

        for(auto& it : inbox)
            if (!it.second.empty())
                ids.insert(it.first);

        return for_each(tc, ids.begin(), ids.end(), func);
    }

    // no barrier between each epoch, busy tiles are skipped
    template<typename TileContainer>
    int for_each_rec(TileContainer& tc, const std::function<int(Tile&)>& func)
    {
        int count = 0;
        std::function<int(Id)> func2 = [this, &tc, &func](Id id)
        {
            {
                std::unique_lock<std::mutex> lock(tc_mutex);
                tc.load(id);
            }
            int count = func(*(tc.get_tile(id)));
            {
                std::unique_lock<std::mutex> lock(tc_mutex);
                if(tc.number_of_tiles() >= max_number_of_tiles) {
                    if(!tc.save(id)) assert(false);
                    tc.unload(id);
                }
            }
            return count;
/*
            // instead of unloading the just-processed excess tile, unload a tile that is not being processed with minimal incoming points ?
            {
                std::unique_lock<std::mutex> lock(tc_mutex);
                if(!tc.is_loaded(id)) {
                    while(tc.number_of_tiles() >= max_number_of_tiles) {
                        auto it = tc.begin();
                        Id id0 = it->id();
                        if (futures.count(id0) == 0) continue; // race condition issues ?
                        size_t count0 = inbox[id0].size();
                        for(++it; it != tc.end() && count0; ++it)
                        {
                            Id id1 = it->id();
                            size_t count1 = inbox[id1].size();
                            if(count0 > count1) {
                                count0 = count1;
                                id0 = id1;
                            }
                        }
                        if(!tc.save(id0)) assert(false);
                        tc.unload(id0);
                    }
                    tc.load(id);
                }
            }
            return func(*(tc.get_tile(id)));
*/
        };

        std::map<Id, std::future<int>> futures;
        do {
            auto fit = futures.begin();
            while(fit!=futures.end())
            {
                if (fit->second.wait_for(timeout_) != std::future_status::ready) {
                    ++fit;
                } else {
                    count += fit->second.get();
                    futures.erase(fit++);
                }
            }
            for(const auto& it : inbox)
            {
                Id id = it.first;
                if (!it.second.empty() && futures.count(id) == 0)
                    futures[id] = pool.submit(func2, id);
            }
            size_t n = allbox.size();
            for(const auto& it : allbox_sent)
            {
                Id id = it.first;
                if (it.second != n && futures.count(id) == 0)
                    futures[id] = pool.submit(func2, id);
            }
        } while (!futures.empty());
        return count;
    }

private:
    safe<Point_id_container> allbox;
    std::map<Id, size_t> allbox_sent;
    std::map<Id, safe<Point_id_container>> inbox;
    std::map<Id, std::map<Id, std::set<Point_id>>> sent_; // no race condition, as the first Id is the source tile id
    size_t max_number_of_tiles;
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
