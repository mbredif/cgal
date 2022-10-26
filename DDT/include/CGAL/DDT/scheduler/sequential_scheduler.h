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

#ifndef CGAL_DDT_SCHEDULER_SEQUENTIAL_SCHEDULER_H
#define CGAL_DDT_SCHEDULER_SEQUENTIAL_SCHEDULER_H

#include <map>
#include <vector>

namespace ddt
{

template<typename T>
struct sequential_scheduler
{
    typedef T Tile;
    typedef typename Tile::Vertex_const_handle_and_id Vertex_const_handle_and_id;
    typedef typename Tile::Vertex_const_handle Vertex_const_handle;
    typedef typename Tile::Point_id_source Point_id_source;
    typedef typename Tile::Point Point;
    typedef typename Tile::Id Id;

    sequential_scheduler() {}
    inline int number_of_threads() const
    {
        return 1;
    }

    std::function<int(Tile&)>
    insert_func(bool do_simplify)
    {
        return [this, do_simplify](Tile& tile)
        {
            std::vector<Point_id_source> received;
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
            std::vector<Point_id_source> received;
            inbox[tile.id()].swap(received);
            if(!tile.insert(received)) return 0;
            std::vector<Vertex_const_handle_and_id> outgoing;
            (tile.*f)(outgoing);
            return tile.send_one(inbox, outgoing);
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
            return tile.send_all(inbox, vertices, begin, end);
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
        int count = 0;
        for(Id id : ids)
            count += func(*(tc.get_tile(id)));
        return count;
    }

    // cycles indefinitely, and stops when the last N tiles reported a count of 0
    template<typename TileContainer>
    int for_each_rec(TileContainer& tc, const std::function<int(Tile&)>& func)
    {
        int count = 0, c;
        do {
            c = for_each(tc, func, false);
            count += c;
        } while (c != 0);
        return count;
    }

    void send(const Point& p, Id id, Id source, Id target)
    {
        inbox[target].emplace_back(p,id,source);
    }

    void send(const Point& p, Id id)
    {
        send(p,id,id,id);
    }
private:
    std::map<Id, std::vector<Point_id_source>> inbox;
};

}

#endif // CGAL_DDT_SCHEDULER_SEQUENTIAL_SCHEDULER_H
