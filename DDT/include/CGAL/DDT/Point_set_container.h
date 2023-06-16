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

#ifndef CGAL_DDT_POINT_SET_CONTAINER_H
#define CGAL_DDT_POINT_SET_CONTAINER_H

#include <map>

namespace CGAL {
namespace DDT {

template<typename Point_set>
struct Point_set_container {
    typedef typename Point_set::Tile_index Tile_index;
    typedef typename Point_set::Points Points;
    typedef std::map<Tile_index, Point_set> Container;
    typedef typename Container::iterator iterator;
    typedef typename Container::value_type value_type;
    typedef typename Container::mapped_type mapped_type;
    typedef typename Container::key_type key_type;

    mapped_type& operator[](key_type key) { return point_sets[key]; }
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
                points.insert(points.end(), p.second.begin(), p.second.end());
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
            p.insert(p.end(), points.begin(), points.end());
        }
        extreme_points_.insert(extreme_points_.end(), points.begin(), points.end());
        points.clear();
    }

  private:
    Container point_sets;
    Points extreme_points_;
};

}
}

#endif // CGAL_DDT_POINT_SET_CONTAINER_H
