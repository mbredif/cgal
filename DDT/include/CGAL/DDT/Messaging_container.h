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

#ifndef CGAL_DDT_MESSAGING_CONTAINER_H
#define CGAL_DDT_MESSAGING_CONTAINER_H

#include <map>

namespace CGAL {
namespace DDT {

template<typename Messaging>
struct Messaging_container {
    typedef typename Messaging::Tile_index Tile_index;
    typedef typename Messaging::Points Points;
    typedef std::map<Tile_index, Messaging> Container;
    typedef typename Container::iterator iterator;
    typedef typename Container::value_type value_type;
    typedef typename Container::mapped_type mapped_type;
    typedef typename Container::key_type key_type;

    mapped_type& operator[](key_type key) { return messagings[key]; }
    iterator erase(iterator pos) { return messagings.erase(pos); }
    iterator begin  () { return messagings.begin (); }
    iterator end    () { return messagings.end   (); }

    const Points& extreme_points() const { return extreme_points_; }
    Points& extreme_points() { return extreme_points_; }

    // Global communication between tiles, outgoing from Tile "id".
    // messagings[i][j] is a set of points sent from Tile i to Tile j.
    // upon completion, messagings[i][j] is empty if i!=j (all points are sent)
    // and messagings[i][i] is the union of all the points received by Tile i
    void send_points(Tile_index id) {
        Messaging& msg = messagings[id];
        for(auto& p : msg.points()) {
            if(p.first != id) {
                auto& points = messagings[p.first].points()[p.first];
                points.insert(points.end(), p.second.begin(), p.second.end());
                p.second.clear();
            }
        }
        send_extreme_points(id);
    }

    void send_extreme_points(Tile_index id) {
        Points& points = messagings[id].extreme_points();
        for(auto& [i, msg] : messagings) {
            if (i==id) continue;
            Points& p = msg.points()[i];
            p.insert(p.end(), points.begin(), points.end());
        }
        extreme_points_.insert(extreme_points_.end(), points.begin(), points.end());
        points.clear();
    }

  private:
    Container messagings;
    Points extreme_points_;
};

}
}

#endif // CGAL_DDT_MESSAGING_CONTAINER_H
