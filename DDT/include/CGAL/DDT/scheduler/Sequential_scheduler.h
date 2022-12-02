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

/// \ingroup PkgDDTSchedulerClasses
/// \cgalModels Scheduler
template<typename T>
struct Sequential_scheduler
{
    typedef T Tile;
    typedef typename Tile::Vertex_const_handle Vertex_const_handle;
    typedef typename Tile::Point_id Point_id;
    typedef typename Tile::Point Point;
    typedef typename Tile::Id Id;
    typedef std::vector<Point_id> Point_id_container;

    /// constructor

    inline int number_of_threads() const
    {
        return 1;
    }

#ifdef CGAL_DEBUG_DDT
    ~Sequential_scheduler() {
        std::cout << "broadcast : " << allbox.size() << " points" << std::endl;
    }
#endif

    inline void receive(Id id, Point_id_container& received)
    {
        inbox[id].swap(received);
        allbox_sent.emplace(id, 0);
        size_t size0 = received.size();
        // allbox.copy_after(received, allbox_sent[id]);
        received.insert(received.end(), allbox.begin() + allbox_sent[id], allbox.end());
        size_t size1 = received.size();
        allbox_sent[id] += size1 - size0;
#ifdef CGAL_DEBUG_DDT
        std::cout << int(id) << " : " << size0 << " + " << (size1-size0) << std::endl;
#endif
    }

    void send(const Point& p, Id id, Id target)
    {
        inbox[target].emplace_back(p,id);
    }

    int send_one(const Tile& tile, const std::map<Id, std::set<Vertex_const_handle>>& vertices)
    {
        Id source = tile.id();
        int count = 0;
        for(auto& vi : vertices)
        {
            Id target = vi.first;
            assert(target != source);
            for(Vertex_const_handle v : vi.second)
            {
                Id vid = tile.id(v);
                assert(target != vid);
                const Point& p = tile.point(v);
                inbox[target].emplace_back(p, vid);
            }
            count += vi.second.size();
        }
        return count;
    }

    int send_all(const Tile& tile, const std::vector<Vertex_const_handle>& vertices)
    {
        for(Vertex_const_handle v : vertices)
            allbox.emplace_back(tile.point(v),tile.id(v));
        return vertices.size();
    }

    template<typename TileContainer, typename Id_iterator, typename UnaryOp, typename V = int, typename BinaryOp = std::plus<>>
    V for_range(TileContainer& tc, Id_iterator begin, Id_iterator end, UnaryOp op1, BinaryOp op2 = {}, V init = {})
    {
        V value = init;
        for(Id_iterator it = begin; it != end; ++it)
            value = op2(value, op1(*tc.load(*it)));
        return value;
    }

    template<typename TileContainer, typename UnaryOp, typename V = int, typename BinaryOp = std::plus<>>
    V for_all(TileContainer& tc, UnaryOp op1, BinaryOp op2 = {}, V init = {})
    {
        return for_range(tc, tc.tile_ids_begin(), tc.tile_ids_end(), op1, op2, init);
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

        return for_range(tc, ids.begin(), ids.end(), op1, op2, init);
    }

    // cycles indefinitely, and stops when the last N tiles reported a count of 0
    template<typename TileContainer, typename UnaryOp, typename V = int, typename BinaryOp = std::plus<>>
    V for_each_rec(TileContainer& tc, UnaryOp op1, BinaryOp op2 = {}, V init = {})
    {
        V value = init, v;
        do {
            v = for_each(tc, op1, op2, init);
            value = op2(value, v);
        } while (v != init);
        return value;
    }

private:
    Point_id_container allbox;
    std::map<Id, size_t> allbox_sent;
    std::map<Id, std::vector<Point_id>> inbox;
};

}
}

#endif // CGAL_DDT_SCHEDULER_SEQUENTIAL_SCHEDULER_H
