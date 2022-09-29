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

namespace ddt
{

template<typename T>
struct tbb_scheduler
{
    typedef T Tile;
    typedef typename Tile::Vertex_const_handle_and_id Vertex_const_handle_and_id;
    typedef typename Tile::Vertex_const_handle Vertex_const_handle;
    typedef typename Tile::Point_id_source Point_id_source;
    typedef typename Tile::Point Point;
    typedef typename Tile::Id Id;
    tbb_scheduler(int n_threads)
      : n_threads(n_threads)
    {}

    inline int number_of_threads() const
    {
        return n_threads;
    }

    void send(const Point& p, Id id, Id source, Id target)
    {
        inbox[target].emplace_back(p,id,source);
    }

    void send(const Point& p, Id id)
    {
        send(p,id,id,id);
    }

    std::function<int(Tile&, bool)>
    insert_func(bool do_simplify)
    {
        return [this, do_simplify](Tile& tile, bool /*unused*/ )
        {
            tbb::concurrent_vector<Point_id_source> received;
            inbox[tile.id()].swap(received);
            return int(tile.insert(received, do_simplify));
        };
    }

    template<typename F>
    std::function<int(Tile&, bool)>
    splay_func(F&& f, bool skip_tiles_receiving_no_points = false)
    {
        return [this,f](Tile& tile, bool skip_tiles_receiving_no_points)
        {
            tbb::concurrent_vector<Point_id_source> received;
            inbox[tile.id()].swap(received);
            if(!tile.insert(received) && skip_tiles_receiving_no_points) return 0;
            std::vector<Vertex_const_handle_and_id> vertices;
            (tile.*f)(vertices);
            std::map<Id, std::vector<Point_id_source>> outgoing;
            int count = tile.send_one(outgoing, vertices);
            for(auto& p : outgoing)
              inbox[p.first].grow_by(p.second.begin(), p.second.end());
            return count;
        };
    }

    template<typename Id_iterator, typename F>
    std::function<int(Tile&, bool)>
    send_all_func(Id_iterator begin, Id_iterator end, F&& f)
    {
        return [this,f,begin,end](Tile& tile, bool /*unused*/)
        {
            std::vector<Vertex_const_handle> vertices;
            (tile.*f)(vertices);
            std::map<Id, std::vector<Point_id_source>> outgoing;
            int count = tile.send_all(outgoing, vertices, begin, end);
            for(auto& p : outgoing)
              inbox[p.first].grow_by(p.second.begin(), p.second.end());
            return count;
        };
    }

    template<typename Tile_iterator>
    int for_each(Tile_iterator begin, Tile_iterator end, const std::function<int(Tile&, bool)>& func, bool skip_tiles_receiving_no_points=false)
    {
        std::vector<std::reference_wrapper<Tile>> tiles(begin, end);

        int count = tbb::parallel_reduce(
              tbb::blocked_range<int>(0,tiles.size()),
              0,
              [&](tbb::blocked_range<int> r, double running_total)
              {
                  int c = 0;
                  for (int i=r.begin(); i<r.end(); ++i)
                  {
                      c+=func(tiles[i].get(), false);
                  }
                  return c;
              }, std::plus<int>() );
        return count;
    }
    /*
        // barrier between each epoch
        template<typename Tile_iterator>
        int for_each_rec(Tile_iterator begin, Tile_iterator end, const std::function<int(Tile&, bool)>& func)
        {
            int count = for_each(begin, end, func), c;
            while((c = for_each(begin, end, func, true)))
                count += c;
            return count;
        }
    */

    // no barrier between each epoch, busy tiles are skipped
    template<typename Tile_iterator>
    int for_each_rec(Tile_iterator begin, Tile_iterator end, const std::function<int(Tile&, bool)>& func)
    {
        if (begin == end) return 0;
        std::vector<std::reference_wrapper<Tile>> tiles(begin, end);

        int count = 0;
        int checkcount = 0;
        do{
              int checkcount = tbb::parallel_reduce(
              tbb::blocked_range<int>(0,tiles.size()),
              0,
              [&](tbb::blocked_range<int> r, double running_total)
              {
                  int c = 0;
                  for (int i=r.begin(); i<r.end(); ++i)
                  {
                      c+=func(tiles[i].get(), false);
                  }
                  return c;
              }, std::plus<int>() );
              count += checkcount;
        } while (checkcount!=0);

        return count;
    }
private:
    std::map<Id, tbb::concurrent_vector<Point_id_source>> inbox;
    int n_threads; // SL: useful?
};

}

#endif // CGAL_DDT_SCHEDULER_TBB_SCHEDULER_H
