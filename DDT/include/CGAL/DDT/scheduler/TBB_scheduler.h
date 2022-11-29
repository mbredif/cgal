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

    TBB_scheduler()
    {}

    inline int number_of_threads() const
    {
        return tbb::this_task_arena::max_concurrency();
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

    int send_one(const Tile& tile, std::vector<Vertex_const_handle_and_id>& vertices)
    {
        std::map<Id, std::vector<Point_id>> points;
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
        for(auto& p : points)
            inbox[p.first].grow_by(p.second.begin(), p.second.end());
        return count;
    }

    int send_all(const Tile& tile, std::vector<Vertex_const_handle>& vertices)
    {
        Point_id_container points;
        for(Vertex_const_handle v : vertices)
            points.emplace_back(tile.point(v),tile.id(v));
        allbox.grow_by(points.begin(), points.end());
        return points.size();
    }

    template<typename TileContainer, typename Id_iterator>
    int for_each(TileContainer& tc, Id_iterator begin, Id_iterator end, const std::function<int(Tile&)>& func)
    {
        // ensure sent_ has all the id inserted to prevent race conditions
        for(Id_iterator it = begin; it != end; ++it)
        {
            sent_.emplace(*it, std::map<Id, std::set<Point_id>>{});
            allbox_sent.emplace(*it, 0);
        }

        std::vector<Id> ids(begin, end);
        int count = tbb::parallel_reduce(
              tbb::blocked_range<int>(0,ids.size()),
              0, [&](tbb::blocked_range<int> r, double running_total)
        {
            int c = 0;
            for (int i=r.begin(); i<r.end(); ++i)
            {
                Id id = ids[i];
                typename TileContainer::Tile_iterator tile;
                {
                    std::unique_lock<std::mutex> lock(tc_mutex);
                    tile = tc.load(id);
                    tile->in_use = true;
                }
                c+=func(*tile);
                {
                    std::unique_lock<std::mutex> lock(tc_mutex);
                    tile->in_use = false;
                }
            }
            return c;
        }, std::plus<int>() );
        return count;
    }

    template<typename TileContainer>
    int for_all(TileContainer& tc, const std::function<int(Tile&)>& func)
    {
        std::vector<Id> ids;
        {
            std::unique_lock<std::mutex> lock(tc_mutex);
            ids.insert(ids.end(), tc.tile_ids_begin(), tc.tile_ids_end());
        }
        return for_each(tc, ids.begin(), ids.end(), func);
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

    template<typename TileContainer>
    int for_each_rec(TileContainer& tc, const std::function<int(Tile&)>& func)
    {
        int count = 0, c = 0;
        do{
          c = for_each(tc, func);
          count += c;
        } while (c!=0);
        return count;
    }

private:
    Point_id_container allbox;
    std::map<Id, size_t> allbox_sent;
    std::map<Id, Point_id_container> inbox;
    std::map<Id, std::map<Id, std::set<Point_id>>> sent_; // no race condition, as the first Id is the source tile id
    std::mutex tc_mutex;
};

}
}

#endif // CGAL_DDT_SCHEDULER_TBB_SCHEDULER_H
