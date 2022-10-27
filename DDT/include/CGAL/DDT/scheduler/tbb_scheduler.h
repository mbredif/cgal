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
#include <vector>
#include <chrono>
#include <tbb/concurrent_vector.h>
#include <tbb/parallel_reduce.h>
#include <tbb/task_arena.h>

namespace ddt
{

template<typename T>
struct tbb_scheduler
{
    typedef T Tile;
    typedef typename Tile::Vertex_const_handle_and_id Vertex_const_handle_and_id;
    typedef typename Tile::Vertex_const_handle Vertex_const_handle;
    typedef typename Tile::Point_id Point_id;
    typedef typename Tile::Point Point;
    typedef typename Tile::Id Id;
    tbb_scheduler()
    {}

    inline int number_of_threads() const
    {
        return tbb::this_task_arena::max_concurrency();
    }

    void send(const Point& p, Id id, Id target)
    {
        inbox[target].emplace_back(p,id);
    }

    void send(const Point& p, Id id)
    {
        send(p,id,id);
    }

    std::function<int(Tile&)>
    insert_func(bool do_simplify)
    {
        return [this, do_simplify](Tile& tile)
        {
            tbb::concurrent_vector<Point_id> received;
            inbox[tile.id()].swap(received);
            return int(tile.insert(received, do_simplify));
        };
    }

    template<typename F>
    std::function<int(Tile&)>
    splay_func(F&& f)
    {
        return [this,f](Tile& tile)
        {
            tbb::concurrent_vector<Point_id> received;
            inbox[tile.id()].swap(received);
            if(!tile.insert(received)) return 0;
            std::vector<Vertex_const_handle_and_id> vertices;
            (tile.*f)(vertices);
            std::map<Id, std::vector<Point_id>> outgoing;
            int count = tile.send_one(outgoing, vertices);
            for(auto& p : outgoing)
              inbox[p.first].grow_by(p.second.begin(), p.second.end());
            return count;
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
            std::map<Id, std::vector<Point_id>> outgoing;
            int count = tile.send_all(outgoing, vertices, begin, end);
            for(auto& p : outgoing)
              inbox[p.first].grow_by(p.second.begin(), p.second.end());
            return count;
        };
    }

    template<typename TileContainer>
    int for_each(TileContainer& tc, const std::function<int(Tile&)>& func, bool all_tiles)
    {
        std::vector<Id> ids;
        if (all_tiles) {
            ids.assign(tc.tile_ids_begin(), tc.tile_ids_end());
        } else {
            for(auto it : inbox) {
                if (!it.second.empty()) {
                    Id id = it.first;
                    ids.push_back(id);
                    if(!tc.is_loaded(id)) tc.init(id); /// @todo : load !
                }
            }
        }
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
    std::map<Id, tbb::concurrent_vector<Point_id>> inbox;
};

}

#endif // CGAL_DDT_SCHEDULER_TBB_SCHEDULER_H
