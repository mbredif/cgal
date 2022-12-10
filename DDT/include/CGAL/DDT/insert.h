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

#ifndef CGAL_DDT_DISTRIBUTED_DELAUNAY_TRIANGULATION_H
#define CGAL_DDT_DISTRIBUTED_DELAUNAY_TRIANGULATION_H

#include <vector>

#if __cplusplus >= 202002L
# include <ranges> // c++20 for std::views::counted
#endif

namespace CGAL {
namespace DDT {
namespace impl {

template<typename TileContainer, typename Tile>
size_t splay_tile(TileContainer& tc, Tile& tile)
{
    typedef typename Tile::Id Id;
    typedef typename Tile::Points Points;
    typedef typename Tile::Vertex_const_handle Vertex_const_handle;
    Points received;
    tc.receive_points(tile, received);
    if (received.empty()) return 0;


    // insert them into the current tile triangulation and get the new foreign points
    std::set<Vertex_const_handle> inserted;
    if(!tile.insert(received, inserted, true)) return 0;
    // get the relevant neighbor points
    std::map<Id, std::set<Vertex_const_handle>> vertices;
    tile.get_finite_neighbors(inserted, vertices);
    // send them to the relevant neighboring tiles
    return tc.send_vertices_to_one_tile(tile, vertices);
}

template<typename TileContainer, typename Scheduler>
size_t insert_and_send_all_axis_extreme_points(TileContainer& tc, Scheduler& sch)
{
    typedef typename TileContainer::Tile Tile;
    typedef typename Tile::Vertex_const_handle Vertex_const_handle;
    typedef typename Tile::Point Point;
    typedef typename Tile::Id Id;
    return sch.for_each(tc, [](TileContainer& tc, Tile& tile)
    {
        size_t count = splay_tile(tc, tile);

        // send the extreme points along each axis to all tiles to initialize the star splaying
        std::vector<Vertex_const_handle> vertices;
        tile.get_axis_extreme_points(vertices);
        tc.send_vertices_to_all_tiles(tile, vertices);
        return count;
    });
}

template<typename TileContainer, typename Scheduler>
size_t splay_stars(TileContainer& tc, Scheduler& sch)
{
    typedef typename TileContainer::Tile Tile;
    return sch.for_each_rec(tc, [](TileContainer& tc, Tile& tile) { return splay_tile(tc, tile); });
}

// Inserts the recieved points, in the Delaunay triangulation stored in the tile container.
// The scheduler provides the distribution environment (single thread, multithread, MPI...)
// @returns the vertex const iterator to the inserted point, or the already existing point if if it was already present
template<typename TileContainer, typename Scheduler>
size_t insert_received(TileContainer& tc, Scheduler& sch){
    size_t n = tc.number_of_finite_vertices();
    insert_and_send_all_axis_extreme_points(tc, sch);
    splay_stars(tc, sch);
    tc.finalize(); /// @todo : return 0 for unloaded tiles
    return tc.number_of_finite_vertices() - n;
}

} // namespace impl

/*
/// \ingroup PkgDDTInsert
/// Inserts the given point in the tile given by the given id, in the Delaunay triangulation stored in the tile container.
/// The scheduler provides the distribution environment (single thread, multithread, MPI...)
/// @returns the vertex const iterator to the inserted point, or the already existing point if if it was already present
template<typename TileContainer, typename Scheduler, typename Point, typename Id>
typename TileContainer::Vertex_const_iterator insert(TileContainer& tc, Scheduler& sch, const Point& point, Id id){
    tc.send_point_to_its_tile(id,point);
    return impl::insert_received(tc, sch); /// @todo this returns a size_t, not a Vertex_const_iterator
}
*/

/// \ingroup PkgDDTInsert
/// Inserts the points of the provided point+id range in the tiles given by the given ids, in the Delaunay triangulation stored in the tile container.
/// The scheduler provides the distribution environment (single thread, multithread, MPI...)
/// @returns the number of newly inserted vertices
template<typename TileContainer, typename Scheduler, typename PointIdRange>
size_t insert(TileContainer& tc, Scheduler& sch, const PointIdRange& range) {
    for (auto& p : range)
        tc.send_point_to_its_tile(p.first,p.second);
    return impl::insert_received(tc, sch);
}

/// \ingroup PkgDDTInsert
/// Inserts the points of the provided point range in the tiles given by the partitioning function, in the Delaunay triangulation stored in the tile container.
/// The scheduler provides the distribution environment (single thread, multithread, MPI...)
/// @returns the number of newly inserted vertices
template<typename TileContainer, typename Scheduler, typename PointRange, typename Partitioner>
size_t insert(TileContainer& tc, Scheduler& sch, PointRange points, Partitioner& part) {
    for(auto& p : points)
        tc.send_point_to_its_tile(part(p),p);
    return impl::insert_received(tc, sch);
}

/// \ingroup PkgDDTInsert
/// Inserts the points of the provided point range in the tiles given by the partitioning function, in the Delaunay triangulation stored in the tile container.
/// The scheduler provides the distribution environment (single thread, multithread, MPI...)
/// @returns the number of newly inserted vertices
template<typename TileContainer, typename Scheduler, typename Iterator, typename Partitioner>
size_t insert(TileContainer& tc, Scheduler& sch, Iterator it, int count, Partitioner& part) {
#if __cplusplus >= 202002L
    // using c++20 and #include <ranges>
    return impl::insert(tc, sch, std::views::counted(it, count), part);
#else
    for(; count; --count, ++it) {
        auto p(*it);
        tc.send_point_to_its_tile(part(p),p);
    }
    return impl::insert_received(tc, sch);
#endif
}

}
}

#endif // CGAL_DDT_DISTRIBUTED_DELAUNAY_TRIANGULATION_H
