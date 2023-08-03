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

#ifndef CGAL_DISTRIBUTED_POINT_SET_H
#define CGAL_DISTRIBUTED_POINT_SET_H

#include <map>
#include <CGAL/DDT/tile_points/No_tile_points.h>
#include <CGAL/DDT/Point_set.h>

namespace CGAL {

template<typename Point_, typename TileIndex, typename TilePoints = CGAL::DDT::No_tile_points>
struct Distributed_point_set {
    typedef Point_ Point;
    typedef TileIndex Tile_index;

    typedef CGAL::DDT::Point_set<Point, Tile_index, TilePoints> Point_set;
    typedef typename Point_set::Points Points;
    typedef std::map<Tile_index, Point_set> Container;
    typedef typename Container::iterator iterator;
    typedef typename Container::value_type value_type;
    typedef typename Container::mapped_type mapped_type;
    typedef typename Container::reference reference;
    typedef typename Container::key_type key_type;

    Distributed_point_set() {}

    template <typename Iterator>
    Distributed_point_set(Iterator begin, Iterator end, Tile_index id = Tile_index()) {
        for(Iterator it = begin; it != end; ++it, ++id) {
            point_sets[id].insert(*it);
        }
    }

    template<typename PointIndexRange>
    Distributed_point_set(const PointIndexRange& range) {
        for (auto& p : range)
            point_sets[p.first].send_point(p.first,p.first,p.second);
    }

    template<typename PointRange, typename Partitioner>
    Distributed_point_set(const PointRange& points, Partitioner& part) {
        for(const auto& p : points)  {
            typename Partitioner::Tile_index id = part(p);
            point_sets[id].send_point(id,id,p);
        }
    }

    template<typename Iterator, typename Partitioner>
    Distributed_point_set(Iterator it, int count, Partitioner& part) {
        for(; count; --count, ++it) {
            auto p(*it);
            Tile_index id = part(p);
            point_sets[id].send_point(id,id,p);
        }
    }

    mapped_type& operator[](key_type key) { return point_sets[key]; }
    iterator find(key_type key) { return point_sets.find(key); }
    iterator erase(iterator pos) { return point_sets.erase(pos); }
    iterator begin  () { return point_sets.begin (); }
    iterator end    () { return point_sets.end   (); }

    const Points& extreme_points() const { return extreme_points_; }
    Points& extreme_points() { return extreme_points_; }

    // Global communication between tiles, outgoing from Tile "id".
    // point_sets[i][j] is a set of points sent from Tile i to Tile j.
    // upon completion, point_sets[i][j] is empty if i!=j (all points are sent)
    // and point_sets[i][i] is the union of all the points received by Tile i
    void send_points(Tile_index id) {
        Point_set& msg = point_sets[id];
        for(auto& p : msg.points()) {
            if(p.first != id) {
                auto& points = point_sets[p.first].points()[p.first];
                points.insert(p.second.begin(), p.second.end());
                p.second.clear();
            }
        }
        send_extreme_points(id);
    }

    void send_extreme_points(Tile_index id) {
        Points& points = point_sets[id].extreme_points();
        for(auto& [i, msg] : point_sets) {
            if (i==id) continue;
            Points& p = msg.points()[i];
            p.insert(points.begin(), points.end());
        }
        extreme_points_.insert(points.begin(), points.end());
        points.clear();
    }

  private:
    Container point_sets;
    Points extreme_points_;
};

}

#endif // CGAL_DISTRIBUTED_POINT_SET_H
