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
#include <tbb/concurrent_vector.h>
#include <tbb/parallel_reduce.h>
#include <tbb/task_arena.h>

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTSchedulerClasses
/// \cgalModels Scheduler
template<typename T>
struct tbb_scheduler
{
    typedef T Tile;
    typedef typename Tile::Vertex_const_handle_and_id Vertex_const_handle_and_id;
    typedef typename Tile::Vertex_const_handle Vertex_const_handle;
    typedef typename Tile::Point_id Point_id;
    typedef typename Tile::Point Point;
    typedef typename Tile::Id Id;
    typedef tbb::concurrent_vector<Point_id> Point_id_container;

    tbb_scheduler()
    {}

    inline int number_of_threads() const
    {
        return tbb::this_task_arena::max_concurrency();
    }

    inline void receive(Id id, Point_id_container& received) { inbox[id].swap(received); }

    void send(const Point& p, Id id, Id target)
    {
        inbox[target].emplace_back(p,id);
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
        for(auto& o : outbox)
            inbox[o.first].grow_by(o.second.begin(), o.second.end());
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
        for(auto& o : outbox)
            inbox[o.first].grow_by(o.second.begin(), o.second.end());
        return count;
    }

    template<typename TileContainer, typename Id_iterator>
    int for_each(TileContainer& tc, Id_iterator begin, Id_iterator end, const std::function<int(Tile&)>& func)
    {
        // ensure sent_ has all the id inserted to prevent race conditions
        for(Id_iterator it = begin; it != end; ++it)
            sent_.emplace(*it, std::map<Id, std::set<Vertex_const_handle>>{});

        std::vector<Id> ids(begin, end);
        int count = tbb::parallel_reduce(
              tbb::blocked_range<int>(0,ids.size()),
              0,
              [&](tbb::blocked_range<int> r, double running_total)
              {
                  int c = 0;
                  for (int i=r.begin(); i<r.end(); ++i)
                  {
                      c+=func(*(tc.get_tile(ids[i])));
                  }
                  return c;
              }, std::plus<int>() );
        return count;
    }

    template<typename TileContainer>
    int for_each(TileContainer& tc, const std::function<int(Tile&)>& func)
    {
        std::vector<Id> ids;
        for(auto& it : inbox) {
            if (it.second.empty()) continue;
            Id id = it.first;
            ids.push_back(id);
            if(!tc.is_loaded(id)) tc.init(id); /// @todo : load !
        }
        return for_each(tc, ids.begin(), ids.end(), func);
    }

    // no barrier between each epoch, busy tiles are skipped
    template<typename TileContainer>
    int for_each_rec(TileContainer& tc, const std::function<int(Tile&)>& func)
    {

        int count = 0, c = 0;
        do{
          std::vector<Id> ids;
          for(auto it : inbox) {
              if (!it.second.empty()) {
                  Id id = it.first;
                  ids.push_back(id);
                  if(!tc.is_loaded(id)) tc.init(id); /// @todo : load !
              }
          }
          c = tbb::parallel_reduce(
          tbb::blocked_range<int>(0,ids.size()),
          0,
          [&](tbb::blocked_range<int> r, double running_total)
          {
              int c = 0;
              for (int i=r.begin(); i<r.end(); ++i)
              {
                  c+=func(*(tc.get_tile(ids[i])));
              }
              return c;
          }, std::plus<int>() );
          count += c;
        } while (c!=0);
        return count;
    }

private:
    std::map<Id, Point_id_container> inbox;
    std::map<Id, std::map<Id, std::set<Vertex_const_handle>>> sent_; // no race condition, as the first Id is the source tile id
};

}
}

#endif // CGAL_DDT_SCHEDULER_TBB_SCHEDULER_H
