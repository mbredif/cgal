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

namespace CGAL {

namespace ddt {

template<typename TileContainer, typename Scheduler>
size_t local_insert_received(TileContainer& tc, Scheduler& sch) {
    typedef typename TileContainer::Tile Tile;
    typedef typename Scheduler::Point_id_container Point_id_container;
    return sch.for_each(tc, [&sch](Tile& tile)
    {
        Point_id_container received;
        sch.receive(tile.id(), received);
        return int(tile.insert(received));
    });
}

template<typename TileContainer, typename Scheduler>
size_t send_all_bbox_points(TileContainer& tc, Scheduler& sch)       {
    typedef typename TileContainer::Tile Tile;
    typedef typename TileContainer::Tile_id_const_iterator Tile_id_const_iterator;
    typedef typename Tile::Vertex_const_handle Vertex_const_handle;
    Tile_id_const_iterator begin = tc.tile_ids_begin();
    Tile_id_const_iterator end = tc.tile_ids_end();
    return sch.for_each(tc, begin, end, [&sch, begin, end](Tile& tile)
    {
        std::vector<Vertex_const_handle> vertices;
        tile.get_bbox_points(vertices);
        return sch.send_all(tile, vertices, begin, end);
    });
}

template<typename TileContainer, typename Scheduler>
size_t splay_stars(TileContainer& tc, Scheduler& sch)       {
    typedef typename TileContainer::Tile Tile;
    typedef typename Scheduler::Point_id_container Point_id_container;
    typedef typename Tile::Vertex_const_handle_and_id Vertex_const_handle_and_id;
    return sch.for_each_rec(tc, [&sch](Tile& tile)
    {
        Point_id_container received;
        sch.receive(tile.id(), received);
        if(!tile.insert(received)) return 0;
        std::vector<Vertex_const_handle_and_id> outgoing;
        tile.get_finite_neighbors(outgoing);
        return sch.send_one(tile, outgoing);
    });
}

/// \ingroup PkgDDTRef
/// Inserts the recieved points, in the Delaunay triangulation stored in the tile container.
/// The scheduler provides the distribution environment (single thread, multithread, MPI...)
/// @returns the vertex const iterator to the inserted point, or the already existing point if if it was already present
template<typename TileContainer, typename Scheduler>
size_t insert_received(TileContainer& tc, Scheduler& sch){
    size_t insertions = local_insert_received(tc, sch);
    send_all_bbox_points(tc, sch);
    splay_stars(tc, sch);
    tc.finalize();
    return insertions;
}


/// \ingroup PkgDDTRef
/// Inserts the given point in the tile given by the given id, in the Delaunay triangulation stored in the tile container.
/// The scheduler provides the distribution environment (single thread, multithread, MPI...)
/// @returns the vertex const iterator to the inserted point, or the already existing point if if it was already present
template<typename TileContainer, typename Scheduler, typename Point, typename Id>
typename TileContainer::Vertex_const_iterator insert(TileContainer& tc, Scheduler& sch, const Point& point, Id id){
    sch.send(point, id);
    return insert_received(tc, sch);
}

/// \ingroup PkgDDTRef
/// Inserts the points of the provided point+id range in the tiles given by the given ids, in the Delaunay triangulation stored in the tile container.
/// The scheduler provides the distribution environment (single thread, multithread, MPI...)
/// @returns the number of newly inserted vertices
template<typename TileContainer, typename Scheduler, typename PointIdRange>
size_t insert(TileContainer& tc, Scheduler& sch, const PointIdRange& range) {
    for (auto point_id : range)
        sch.send(point_id.first, point_id.second);
    return insert_received(tc, sch);
}

/// \ingroup PkgDDTRef
/// Inserts the points of the provided point+id range in the tiles given by the given ids, in the Delaunay triangulation stored in the tile container.
/// The scheduler provides the distribution environment (single thread, multithread, MPI...)
/// @returns the number of newly inserted vertices
template<typename TileContainer, typename Scheduler, typename Iterator, typename Partitioner>
size_t insert(TileContainer& tc, Scheduler& sch, Iterator it, int count, Partitioner& part) {
    for(; count; --count, ++it)
    {
        auto p(*it);
        auto id = part(p);
        sch.send(p, id, id);
    }
    return insert_received(tc, sch);
}



}

}

#endif // CGAL_DDT_DISTRIBUTED_DELAUNAY_TRIANGULATION_H
