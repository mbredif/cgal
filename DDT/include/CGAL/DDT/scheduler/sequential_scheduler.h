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

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTRef
/// \cgalModels Scheduler
template<typename T>
struct sequential_scheduler
{
    typedef T Tile;
    typedef typename Tile::Vertex_const_handle_and_id Vertex_const_handle_and_id;
    typedef typename Tile::Vertex_const_handle Vertex_const_handle;
    typedef typename Tile::Point_id Point_id;
    typedef typename Tile::Point Point;
    typedef typename Tile::Id Id;
    typedef std::vector<Point_id> Point_id_container;

    sequential_scheduler() {}
    inline int number_of_threads() const
    {
        return 1;
    }

    inline void receive(Id id, Point_id_container& received) { inbox[id].swap(received); }

    void send(const Point& p, Id id, Id target)
    {
        inbox[target].emplace_back(p,id);
    }

    bool send_vertex(const Tile& tile, Vertex_const_handle v, Id target)
    {
        if (tile.vertex_is_infinite(v)) return false;
        Id source = tile.id();
        Id vid = tile.id(v);
        if(target==vid || target == source || !sent_[target].insert(v).second)
            return false;
        send(tile.point(v), vid, target);
        return true;
    }

    int send_one(const Tile& tile, std::vector<Vertex_const_handle_and_id>& outbox)
    {
        Id source = tile.id();
        int count = 0;
        for(auto& vi : outbox)
            count += send_vertex(tile, vi.first, vi.second);
        return count;
    }

    template<typename Id_iterator>
    int send_all(const Tile& tile, std::vector<Vertex_const_handle>& outbox, Id_iterator begin, Id_iterator end)
    {
        Id source = tile.id();
        int count = 0;
        for(Vertex_const_handle v : outbox)
            for(Id_iterator target = begin; target != end; ++target)
                count += send_vertex(tile, v, *target);
        return count;
    }

    template<typename TileContainer, typename Id_iterator>
    int for_each(TileContainer& tc, Id_iterator begin, Id_iterator end, const std::function<int(Tile&)>& func)
    {
        int count = 0;
        for(Id_iterator it = begin; it != end; ++it)
            count += func(*(tc.get_tile(*it)));
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

    // cycles indefinitely, and stops when the last N tiles reported a count of 0
    template<typename TileContainer>
    int for_each_rec(TileContainer& tc, const std::function<int(Tile&)>& func)
    {
        int count = 0, c;
        do {
            c = for_each(tc, func);
            count += c;
        } while (c != 0);
        return count;
    }

private:
    std::map<Id, std::vector<Point_id>> inbox;
    std::map<Id, std::set<Vertex_const_handle>> sent_;
};

}
}

#endif // CGAL_DDT_SCHEDULER_SEQUENTIAL_SCHEDULER_H
