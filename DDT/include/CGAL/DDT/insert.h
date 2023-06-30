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

template<typename Tile, typename PointSet>
std::size_t splay_tile(Tile& tile, PointSet& point_set)
{
    typedef typename Tile::value_type                 Tile_triangulation;
    typedef typename Tile_triangulation::Tile_index   Tile_index;
    typedef typename Tile_triangulation::Vertex_index Vertex_index;
    typedef typename PointSet::Points                Points;
    Points received;
    Tile_triangulation& tri = tile.value();
    point_set.receive_points(tri.id(), received);
    if (received.empty()) return 0;
    // insert them into the current tile triangulation and get the new foreign points
    std::set<Vertex_index> inserted;
    if(!tri.insert(received, inserted, true)) return 0;
    // get the relevant neighbor points
    std::map<Tile_index, std::set<Vertex_index>> vertices;
    tri.get_finite_neighbors(inserted, vertices);
    // send them to the relevant neighboring tiles
    return point_set.send_vertices_to_one_tile(tri, vertices);
}

template<typename TileContainer, typename PointSetContainer, typename Scheduler>
std::size_t insert_and_send_all_axis_extreme_points(TileContainer& tiles, PointSetContainer& point_sets, Scheduler& sch)
{
    typedef typename TileContainer::Tile              Tile;
    typedef typename Tile::value_type         Tile_triangulation;
    typedef typename Tile_triangulation::Vertex_index Vertex_index;
    typedef typename PointSetContainer::mapped_type  PointSet;
    return sch.for_each_zip(tiles, point_sets, [](Tile& tile, PointSet& point_set)
    {
        std::size_t count = splay_tile(tile, point_set);
        Tile_triangulation& tri = tile.value();
        // send the extreme points along each axis to all tiles to initialize the star splaying
        std::vector<Vertex_index> vertices;
        tri.get_axis_extreme_points(vertices);
        for(Vertex_index v : vertices)
            tri.bbox() += tri.bbox(v);
        point_set.send_vertices_to_all_tiles(tri, vertices);
        return count;
    });
}

template<typename TileContainer, typename PointSetContainer, typename Scheduler>
std::size_t splay_stars(TileContainer& tiles, PointSetContainer& point_sets, Scheduler& sch)
{
    typedef typename TileContainer::Tile Tile;
    typedef typename PointSetContainer::mapped_type PointSet;
    return sch.for_each_rec(tiles, point_sets, [](Tile& tile, PointSet& point_set) { return splay_tile(tile, point_set); });
}

} // namespace impl

}
}

#endif // CGAL_DDT_INSERT_H
