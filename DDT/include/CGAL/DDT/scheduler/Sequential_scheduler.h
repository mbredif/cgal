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
    typedef typename Tile::Vertex_const_handle_and_id Vertex_const_handle_and_id;
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
        for(auto& s : sent_)
            std::cout << "sent [" << int(s.first) << "] : " << s.second.size() << " points" << std::endl;
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

    int send_one(const Tile& tile, const std::vector<Vertex_const_handle_and_id>& vertices)
    {
        Id source = tile.id();
        int count = 0;
        for(auto& vi : vertices)
        {
            Vertex_const_handle v = vi.first;
            Id target = vi.second;
            Id vid = tile.id(v);
            if(target == source || target == vid) continue;
            const Point& p = tile.point(v);
            if(sent_[target].insert(std::make_pair(p,vid)).second)
            {
                ++count;
                inbox[target].emplace_back(p, vid);
            }
        }
        return count;
    }

    int send_all(const Tile& tile, const std::vector<Vertex_const_handle>& vertices)
    {
        for(Vertex_const_handle v : vertices)
            allbox.emplace_back(tile.point(v),tile.id(v));
        return vertices.size();
    }

    template<typename TileContainer, typename Id_iterator>
    int for_each(TileContainer& tc, Id_iterator begin, Id_iterator end, const std::function<int(Tile&)>& func)
    {
        int count = 0;
        for(Id_iterator it = begin; it != end; ++it)
        {
            typename TileContainer::Tile_iterator tile = tc.find(*it);
            if(tile == tc.end()) {
                while(tc.number_of_tiles() >= tc.maximum_number_of_tiles()) {
                    auto it = tc.begin();
                    Id id0 = it->id();
                    size_t count0 = inbox[id0].size();
                    for(++it; it != tc.end() && count0; ++it)
                    {
                        Id id = it->id();
                        size_t count = inbox[id].size();
                        if(count0 > count) {
                            count0 = count;
                            id0 = id;
                        }
                    }
#ifdef CGAL_DEBUG_DDT
                    std::cout << "unload " << int(id0) << " ( " << inbox[id0].size() << " inbox points)" << std::endl;
#endif
                    tc.unload(id0);
                }
#ifdef CGAL_DEBUG_DDT
                std::cout << "load " << int(*it) << " ( " << inbox[*it].size() << " inbox points)" << std::endl;
#endif
                tile = tc.load(*it).first;
            }
            count += func(*tile);
        }
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
    Point_id_container allbox;
    std::map<Id, size_t> allbox_sent;
    std::map<Id, std::vector<Point_id>> inbox;
    std::map<Id, std::set<Point_id>> sent_;
};

}
}

#endif // CGAL_DDT_SCHEDULER_SEQUENTIAL_SCHEDULER_H
