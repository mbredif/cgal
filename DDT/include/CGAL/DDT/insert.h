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

#ifndef CGAL_DDT_INSERT_H
#define CGAL_DDT_INSERT_H

#include <vector>
#include <set>
#include <map>

#if __cplusplus >= 202002L
# include <ranges> // c++20 for std::views::counted
#endif

namespace CGAL {
namespace DDT {
namespace impl {

template<typename Tile, typename Messaging>
std::size_t splay_tile(Tile& tile, Messaging& messaging)
{
    typedef typename Tile::Tile_index   Tile_index;
    typedef typename Tile::Tile_triangulation::Vertex_index Vertex_index;
    typedef typename Messaging::Points       Points;
    Points received;
    messaging.receive_points(tile.id(), received);
    if (received.empty()) return 0;
    // insert them into the current tile triangulation and get the new foreign points
    std::set<Vertex_index> inserted;
    if(!tile.triangulation().insert(received, inserted, true)) return 0;
    // get the relevant neighbor points
    std::map<Tile_index, std::set<Vertex_index>> vertices;
    tile.triangulation().get_finite_neighbors(inserted, vertices);
    // send them to the relevant neighboring tiles
    return messaging.send_vertices_to_one_tile(tile.triangulation(), vertices);
}

template<typename TileContainer, typename MessagingContainer, typename Scheduler>
std::size_t insert_and_send_all_axis_extreme_points(TileContainer& tc, MessagingContainer& messagings, Scheduler& sch)
{
    typedef typename TileContainer::Tile Tile;
    typedef typename Tile::Tile_triangulation::Vertex_index Vertex_index;
    typedef typename MessagingContainer::mapped_type Messaging;
    return sch.for_each_zip(tc, messagings, [](Tile& tile, Messaging& messaging)
    {
        std::size_t count = splay_tile(tile, messaging);

        // send the extreme points along each axis to all tiles to initialize the star splaying
        std::vector<Vertex_index> vertices;
        tile.triangulation().get_axis_extreme_points(vertices);
        for(Vertex_index v : vertices)
            tile.bbox() += tile.triangulation().bbox(v);
        messaging.send_vertices_to_all_tiles(tile.triangulation(), vertices);
        return count;
    });
}

template<typename TileContainer, typename MessagingContainer, typename Scheduler>
std::size_t splay_stars(TileContainer& tc, MessagingContainer& messagings, Scheduler& sch)
{
    typedef typename TileContainer::Tile Tile;
    typedef typename MessagingContainer::mapped_type Messaging;
    return sch.for_each_rec(tc, messagings, [](Tile& tile, Messaging& messaging) { return splay_tile(tile, messaging); });
}

} // namespace impl

}
}

#endif // CGAL_DDT_INSERT_H
