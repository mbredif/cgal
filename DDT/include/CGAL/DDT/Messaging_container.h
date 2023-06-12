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
    typedef std::map<Tile_index, Messaging> Container;
    typedef typename Container::iterator iterator;
    typedef typename Container::value_type value_type;
    typedef typename Container::mapped_type mapped_type;
    typedef typename Container::key_type key_type;

    mapped_type& operator[](key_type key) { return messagings[key]; }
    iterator begin  () { return messagings.begin (); }
    iterator end    () { return messagings.end   (); }

    // Global communication between tiles, outgoing from Tile "id".
    // messagings[i][j] is a set of points sent from Tile i to Tile j.
    // upon completion, messagings[i][j] is empty if i!=j (all points are sent)
    // and messagings[i][i] is the union of all the points received by Tile i
    void send_points(Tile_index id) {
        typedef typename Messaging::Points Points;
        Messaging& msg = messagings[id];
        for(auto& p : msg.points()) {
            if(p.first != id) {
                auto& points = messagings[p.first].points()[p.first];
                points.insert(points.end(), p.second.begin(), p.second.end());
                p.second.clear();
            }
        }
        for(auto& messaging : messagings) {
            Points& p = messaging.second.points()[messaging.first];
            p.insert(p.end(), msg.extreme_points().begin(), msg.extreme_points().end());
        }
        msg.extreme_points().clear();
    }

  private:
    Container messagings;
};

}
}

#endif // CGAL_DDT_MESSAGING_CONTAINER_H
