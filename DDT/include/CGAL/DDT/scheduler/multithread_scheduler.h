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
#include <CGAL/DDT/scheduler/thread_pool.h>

namespace ddt
{

template<typename T>
struct multithread_scheduler
{
    typedef T Tile;
    typedef typename Tile::Vertex_const_handle_and_id Vertex_const_handle_and_id;
    typedef typename Tile::Vertex_const_handle Vertex_const_handle;
    typedef typename Tile::Point_id Point_id;
    typedef typename Tile::Point Point;
    typedef typename Tile::Id Id;
    multithread_scheduler(int n_threads = 0) : pool(n_threads), timeout_(1)
    {
        pool.init();
    }
    template<class Duration>
    multithread_scheduler(int n_threads, Duration timeout) : pool(n_threads), timeout_(timeout)
    {
        pool.init();
    }
    inline int number_of_threads() const
    {
        return pool.number_of_threads();
    }
    ~multithread_scheduler()
    {
        pool.shutdown();
    }

    void send(const Point& p, Id id, Id target)
    {
        inbox[target].emplace_back(p,id);
    }

    void send(const Point& p, Id id)
    {
        send(p,id,id);
    }

    bool send_vertex(const Tile& tile, Vertex_const_handle v, Id target, std::map<Id, std::vector<Point_id>>& outbox)
    {
        if (tile.vertex_is_infinite(v)) return false;
        Id source = tile.id();
        Id vid = tile.id(v);
        if(target==vid || target == source || !sent_[source][target].insert(v).second)
            return false;
        outbox[target].emplace_back(tile.point(v), vid);
        return true;
    }

    int send_one(const Tile& tile, std::vector<Vertex_const_handle_and_id>& vertices)
    {
        std::map<Id, std::vector<Point_id>> outbox;
        Id source = tile.id();
        int count = 0;
        for(auto& vi : vertices)
            count += send_vertex(tile, vi.first, vi.second, outbox);
        for(auto& o : outbox) inbox[o.first].append(o.second);
        return count;
    }

    template<typename Id_iterator>
    int send_all(const Tile& tile, std::vector<Vertex_const_handle>& vertices, Id_iterator begin, Id_iterator end)
    {
        std::map<Id, std::vector<Point_id>> outbox;
        Id source = tile.id();
        int count = 0;
        for(Vertex_const_handle v : vertices)
            for(Id_iterator target = begin; target != end; ++target)
                count += send_vertex(tile, v, *target, outbox);
        for(auto& o : outbox) inbox[o.first].append(o.second);
        return count;
    }

    std::function<int(Tile&)>
    insert_func()
    {
        return [this](Tile& tile)
        {
            std::vector<Point_id> received;
            inbox[tile.id()].swap(received);
            return int(tile.insert(received));
        };
    }

    template<typename F>
    std::function<int(Tile&)>
    splay_func(F&& f)
    {
        return [this,f](Tile& tile)
        {
            std::vector<Point_id> received;
            inbox[tile.id()].swap(received);
            if(!tile.insert(received)) return 0;
            std::vector<Vertex_const_handle_and_id> vertices;
            (tile.*f)(vertices);
            return send_one(tile, vertices);
        };
    }

    template<typename Id_iterator, typename F>
    std::function<int(Tile&)>
    send_all_func(Id_iterator begin, Id_iterator end, F&& f)
    {
        return [this,f,begin,end](Tile& tile)
        {
            std::vector<Vertex_const_handle> vertices;
            (tile.*f)(vertices);
            return send_all(tile, vertices, begin, end);
        };
    }

    template<typename TileContainer>
    int for_each(TileContainer& tc, const std::function<int(Tile&)>& func, bool all_tiles)
    {
        std::vector<Id> ids;
        if (all_tiles) {
            ids.assign(tc.tile_ids_begin(), tc.tile_ids_end());
        } else {
            for(const auto& it : inbox) {
                if (!it.second.empty()) {
                    Id id = it.first;
                    ids.push_back(id);
                    if(!tc.is_loaded(id)) tc.init(id); /// @todo : load !
                }
            }
        }
        // ensure sent_ has all the id inserted to prevent race conditions
        for(Id id : ids)
            sent_.emplace(id, std::map<Id, std::set<Vertex_const_handle>>{});

        std::vector<std::future<int>> futures;
        for(Id id : ids)
            futures.push_back(pool.submit(func, std::ref(*(tc.get_tile(id)))));

        int count = 0;
        for(auto& f: futures) count += f.get();
        return count;
    }

    // no barrier between each epoch, busy tiles are skipped
    template<typename TileContainer>
    int for_each_rec(TileContainer& tc, const std::function<int(Tile&)>& func)
    {
        int count = 0;
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
            ;
            for(const auto& it : inbox)
            {
                Id id = it.first;
                if (!it.second.empty() && futures.count(id) == 0)
                {
                    if(!tc.is_loaded(id)) tc.init(id); /// @todo : load !
                    futures[id] = pool.submit(func, std::ref(*(tc.get_tile(id))));
                }
            }
        } while (!futures.empty());
        return count;
    }

private:
    std::map<Id, safe<std::vector<Point_id>>> inbox;
    std::map<Id, std::map<Id, std::set<Vertex_const_handle>>> sent_; // no race condition, as the first Id is the source tile id
    thread_pool pool;
    std::chrono::milliseconds timeout_;
};

}

#endif // CGAL_DDT_SCHEDULER_MULTITHREAD_SCHEDULER_H
