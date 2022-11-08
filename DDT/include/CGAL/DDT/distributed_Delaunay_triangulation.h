
namespace CGAL {

namespace ddt {

template<typename TileContainer, typename Scheduler>
size_t local_insert_received(TileContainer& tc, Scheduler& sch) {
    return sch.for_each(tc, sch.insert_func(), false);
}

template<typename TileContainer, typename Scheduler>
size_t send_all_bbox_points(TileContainer& tc, Scheduler& sch)       {
    typedef typename TileContainer::Tile Tile;
    return sch.for_each(tc, sch.send_all_func(tc.tile_ids_begin(), tc.tile_ids_end(), &Tile::get_bbox_points), true);
}

template<typename TileContainer, typename Scheduler>
size_t splay_stars(TileContainer& tc, Scheduler& sch)       {
    typedef typename TileContainer::Tile Tile;
    return sch.for_each_rec(tc, sch.splay_func(&Tile::get_finite_neighbors));
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
        sch.send(p, id);
    }
    return insert_received(tc, sch);
}



}

}

