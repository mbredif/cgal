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

template<typename Tile, typename Scheduler>
size_t splay_tile(Tile& tile, Scheduler& sch)
{
    typedef typename Scheduler::Point_id_container Point_id_container;
    typedef typename Tile::Vertex_const_handle_and_id Vertex_const_handle_and_id;
    Point_id_container received;
    sch.receive(tile.id(), received);
    if(!tile.insert(received)) return 0;
    std::vector<Vertex_const_handle_and_id> vertices;
    tile.get_finite_neighbors(vertices);
    return sch.send_one(tile, vertices);
}

template<typename TileContainer, typename Scheduler>
size_t insert_and_send_all_bbox_points(TileContainer& tc, Scheduler& sch)
{
    typedef typename TileContainer::Tile Tile;
    typedef typename Tile::Vertex_const_handle Vertex_const_handle;
    return sch.for_each(tc, [&sch](Tile& tile)
    {
        size_t count = splay_tile(tile, sch);
        std::vector<Vertex_const_handle> bbox;
        tile.get_bbox_points(bbox);
        sch.send_all(tile, bbox);
        return count;
    });
}

template<typename TileContainer, typename Scheduler>
size_t splay_stars(TileContainer& tc, Scheduler& sch)
{
    typedef typename TileContainer::Tile Tile;
    return sch.for_each_rec(tc, [&sch](Tile& tile) { return splay_tile(tile, sch); });
}

/// \ingroup PkgDDTInsert
/// Inserts the recieved points, in the Delaunay triangulation stored in the tile container.
/// The scheduler provides the distribution environment (single thread, multithread, MPI...)
/// @returns the vertex const iterator to the inserted point, or the already existing point if if it was already present
template<typename TileContainer, typename Scheduler>
size_t insert_received(TileContainer& tc, Scheduler& sch){
    size_t n = tc.number_of_finite_vertices();
    insert_and_send_all_bbox_points(tc, sch);
    splay_stars(tc, sch);
    tc.finalize(); /// @todo : return 0 for unloaded tiles
    return tc.number_of_finite_vertices() - n;
}


/// \ingroup PkgDDTInsert
/// Inserts the given point in the tile given by the given id, in the Delaunay triangulation stored in the tile container.
/// The scheduler provides the distribution environment (single thread, multithread, MPI...)
/// @returns the vertex const iterator to the inserted point, or the already existing point if if it was already present
template<typename TileContainer, typename Scheduler, typename Point, typename Id>
typename TileContainer::Vertex_const_iterator insert(TileContainer& tc, Scheduler& sch, const Point& point, Id id){
    sch.send(point, id, id);
    tc.init(id);
    return insert_received(tc, sch);
}

/// \ingroup PkgDDTInsert
/// Inserts the points of the provided point+id range in the tiles given by the given ids, in the Delaunay triangulation stored in the tile container.
/// The scheduler provides the distribution environment (single thread, multithread, MPI...)
/// @returns the number of newly inserted vertices
template<typename TileContainer, typename Scheduler, typename PointIdRange>
size_t insert(TileContainer& tc, Scheduler& sch, const PointIdRange& range) {
    for (auto point_id : range) {
        auto id = point_id.second;
        sch.send(point_id.first, id, id);
        tc.init(id);
    }
    return insert_received(tc, sch);
}

/// \ingroup PkgDDTInsert
/// Inserts the points of the provided point range in the tiles given by the partitioning function, in the Delaunay triangulation stored in the tile container.
/// The scheduler provides the distribution environment (single thread, multithread, MPI...)
/// @returns the number of newly inserted vertices
template<typename TileContainer, typename Scheduler, typename PointRange, typename Partitioner>
size_t insert(TileContainer& tc, Scheduler& sch, PointRange points, Partitioner& part) {
    for(auto& p : points)
    {
        auto id = part(p);
        sch.send(p, id, id);
        tc.init(id);
    }
    return insert_received(tc, sch);
}

/// \ingroup PkgDDTInsert
/// Inserts the points of the provided point range in the tiles given by the partitioning function, in the Delaunay triangulation stored in the tile container.
/// The scheduler provides the distribution environment (single thread, multithread, MPI...)
/// @returns the number of newly inserted vertices
template<typename TileContainer, typename Scheduler, typename Iterator, typename Partitioner>
size_t insert(TileContainer& tc, Scheduler& sch, Iterator it, int count, Partitioner& part) {
#if __cplusplus >= 202002L
    // using c++20 and #include <ranges>
    return insert(tc, sch, std::views::counted(it, count), part);
#else
    for(; count; --count, ++it)
    {
        auto p(*it);
        auto id = part(p);
        sch.send(p, id, id);
        tc.init(id);
    }
    return insert_received(tc, sch);
#endif
}

}
}

#endif // CGAL_DDT_DISTRIBUTED_DELAUNAY_TRIANGULATION_H
