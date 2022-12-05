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

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTSchedulerClasses
/// \cgalModels Scheduler
template<typename T>
struct TBB_scheduler
{
    typedef T Tile;
    typedef typename Tile::Vertex_const_handle_and_id Vertex_const_handle_and_id;
    typedef typename Tile::Vertex_const_handle Vertex_const_handle;
    typedef typename Tile::Point_id Point_id;
    typedef typename Tile::Point Point;
    typedef typename Tile::Id Id;
    typedef tbb::concurrent_vector<Point_id> Point_id_container;

    TBB_scheduler(int num_threads = 0) : arena(num_threads ? num_threads : tbb::task_arena::automatic)
    {}

    inline int max_concurrency() const
    {
        return arena.max_concurrency();
    }

    inline void receive(Id id, Point_id_container& received)
    {
        inbox[id].swap(received);
        allbox_sent.emplace(id, 0);
        size_t size0 = received.size();
        received.grow_by(allbox.begin()+allbox_sent[id], allbox.end());
        size_t size1 = received.size();
        allbox_sent[id] += size1 - size0;
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
                Id vid = tile.vertex_id(v);
                assert(target != vid);
                const Point& p = tile.point(v);
                points.emplace_back(p, vid);
            }
            inbox[target].grow_by(points.begin(), points.end());
            count += points.size();
        }
        return count;
    }

    int send_all(const Tile& tile, std::vector<Vertex_const_handle>& vertices)
    {
        Point_id_container points;
        for(Vertex_const_handle v : vertices)
            points.emplace_back(tile.point(v),tile.vertex_id(v));
        allbox.grow_by(points.begin(), points.end());
        return points.size();
    }

    template<typename TileContainer, typename Id_iterator, typename UnaryOp, typename V = int, typename BinaryOp = std::plus<>>
    V for_each(TileContainer& tc, Id_iterator begin, Id_iterator end, UnaryOp op1, BinaryOp op2 = {}, V init = {})
    {
        // ensure allbox_sent has all the id inserted to prevent race conditions
        for(Id_iterator it = begin; it != end; ++it)
            allbox_sent.emplace(*it, 0);

        std::vector<Id> ids(begin, end);
        return arena.execute([&]{
            return tbb::parallel_reduce(
                  tbb::blocked_range<int>(0,ids.size()),
                  init, [&](tbb::blocked_range<int> r, double running_total)
            {
                V c = init;
                for (int i=r.begin(); i<r.end(); ++i)
                {
                    Id id = ids[i];
                    std::pair<typename TileContainer::Tile_iterator, bool> insertion;
                    do {
                        std::unique_lock<std::mutex> lock(tc_mutex);
                        insertion = tc.insert(id);
                    } while (!tc.load(insertion));
                    typename TileContainer::Tile_iterator tile = insertion.first;
                    c = op2(c,op1(*tile));
                    {
                        std::unique_lock<std::mutex> lock(tc_mutex);
                        tc.unload(tile);
                    }
                }
                return c;
            }, op2 );
        });
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

    template<typename TileContainer, typename UnaryOp, typename V = int, typename BinaryOp = std::plus<>>
    V for_each_rec(TileContainer& tc, UnaryOp op1, BinaryOp op2 = {}, V init = {})
    {
        V value = init, v;
        do{
          v = for_each(tc, op1, op2, init);
          value = op2(value,v);
        } while (v!=init);
        return value;
    }

    Point_id_container allbox;
private:
    std::map<Id, size_t> allbox_sent;
    std::map<Id, Point_id_container> inbox;

    std::mutex tc_mutex;
    tbb::task_arena arena;
};

}
}

#endif // CGAL_DDT_SCHEDULER_TBB_SCHEDULER_H
