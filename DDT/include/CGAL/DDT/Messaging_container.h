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
    typedef typename Messaging::Points Points;

    mapped_type& operator[](key_type key) { return messagings[key]; }
    iterator begin  () { return messagings.begin (); }
    iterator end    () { return messagings.end   (); }

    void send_points(Tile_index id) {
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
        extreme_points_.insert(extreme_points_.end(), msg.extreme_points().begin(), msg.extreme_points().end());
        msg.extreme_points().clear();
    }

    private:
    std::map<Tile_index, Messaging> messagings;
    Points extreme_points_;
};

}
}

#endif // CGAL_DDT_MESSAGING_CONTAINER_H
