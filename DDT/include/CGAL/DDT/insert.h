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

#ifndef CGAL_DDT_DISTRIBUTED_TRIANGULATION_H
#define CGAL_DDT_DISTRIBUTED_TRIANGULATION_H

#include <vector>

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
    typedef typename Messaging::Vertex_index Vertex_index;
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
    typedef typename MessagingContainer::mapped_type Messaging;
    typedef typename Messaging::Vertex_index Vertex_index;
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

/// \ingroup PkgDDTInsert
/// Triangulate the points into the tile container, so that each of its tile triangulations contain a local view of the overall
/// triangulation of all inserted points.
/// The scheduler provides the distribution environment (single thread, multithread, MPI...)
/// @returns the number of newly inserted vertices
template<typename TileContainer, typename MessagingContainer, typename Scheduler>
std::size_t triangulate(TileContainer& tc, MessagingContainer& messagings, Scheduler& sch){
    std::size_t n = tc.number_of_finite_vertices();
    impl::insert_and_send_all_axis_extreme_points(tc, messagings, sch);
    impl::splay_stars(tc, messagings, sch);
    tc.finalize(); /// @todo : return 0 for unloaded tiles
    return tc.number_of_finite_vertices() - n;
}

/*
/// \ingroup PkgDDTInsert
/// Inserts the given point in the tile given by the given id, in the Delaunay triangulation stored in the tile container.
/// The scheduler provides the distribution environment (single thread, multithread, MPI...)
/// @returns the vertex const iterator to the inserted point, or the already existing point if if it was already present
template<typename TileContainer, typename Scheduler, typename Point, typename Tile_index>
typename TileContainer::Vertex_index insert(TileContainer& tc, Scheduler& sch, const Point& point, Tile_index id){
    tc.send_point_to_its_tile(id,point);
    return triangulate(tc, sch); /// @todo this returns a std::size_t, not a Vertex_index
}
*/

/// \ingroup PkgDDTInsert
/// inserts the points of the provided point+id range in the tiles given by the given ids, in the Delaunay triangulation stored in the tile container.
/// The scheduler provides the distribution environment (single thread, multithread, MPI...)
/// @returns the number of newly inserted vertices
template<typename TileContainer, typename Scheduler, typename PointIndexRange>
std::size_t insert(TileContainer& tc, Scheduler& sch, const PointIndexRange& range) {
    typedef Messaging<typename TileContainer::Triangulation, typename TileContainer::TileIndexProperty> Messaging;
    Messaging_container<typename TileContainer::Tile_index, Messaging> messaging;
    for (auto& p : range)
        messaging[p.first].send_point(p.first,p.first,p.second);
    return triangulate(tc, messaging, sch);
}

/// \ingroup PkgDDTInsert
/// inserts the points of the provided point range in the tiles given by the partitioning function, in the Delaunay triangulation stored in the tile container.
/// The scheduler provides the distribution environment (single thread, multithread, MPI...)
/// @returns the number of newly inserted vertices
template<typename TileContainer, typename Scheduler, typename PointRange, typename Partitioner>
std::size_t insert(TileContainer& tc, Scheduler& sch, const PointRange& points, Partitioner& part) {
    typedef Messaging<typename TileContainer::Triangulation, typename TileContainer::TileIndexProperty> Messaging;
    Messaging_container<typename TileContainer::Tile_index, Messaging> messaging;
    for(const auto& p : points)  {
        typename Partitioner::Tile_index id = part(p);
        messaging[id].send_point(id,id,p);
    }
    return triangulate(tc, messaging, sch);
}

/// \ingroup PkgDDTInsert
/// inserts the points of the provided point range in the tiles given by the partitioning function, in the Delaunay triangulation stored in the tile container.
/// The scheduler provides the distribution environment (single thread, multithread, MPI...)
/// @returns the number of newly inserted vertices
template<typename TileContainer, typename Scheduler, typename Iterator, typename Partitioner>
std::size_t insert(TileContainer& tc, Scheduler& sch, Iterator it, int count, Partitioner& part) {
#if __cplusplus >= 202002L
    // using c++20 and #include <ranges>
    return impl::insert(tc, sch, std::views::counted(it, count), part);
#else
    typedef Messaging<typename TileContainer::Triangulation, typename TileContainer::TileIndexProperty> Messaging;
    Messaging_container<typename TileContainer::Tile_index, Messaging> messaging;
    for(; count; --count, ++it) {
        auto p(*it);
        typename Partitioner::Tile_index id = part(p);
        messaging[id].send_point(id,id,p);
    }
    return triangulate(tc, messaging, sch);
#endif
}

}
}

#endif // CGAL_DDT_DISTRIBUTED_TRIANGULATION_H
