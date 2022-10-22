
namespace CGAL {

namespace ddt {

/// \ingroup PkgDDTRef
/// Inserts the given point in the tile given by the given id, in the Delaunay triangulation stored in the tile container.
/// The scheduler provides the distribution environment (single thread, multithread, MPI...)
/// @returns the vertex const iterator to the inserted point, or the already existing point if if it was already present
template<typename TileContainer, typename Scheduler, typename Point, typename Id>
typename TileContainer::Vertex_const_iterator insert(TileContainer& tc, Scheduler& sc, const Point&, Id id);



/// \ingroup PkgDDTRef
/// Inserts the points of the provided point+id range in the tiles given by the given ids, in the Delaunay triangulation stored in the tile container.
/// The scheduler provides the distribution environment (single thread, multithread, MPI...)
/// @returns the number of newly inserted vertices
template<typename TileContainer, typename Scheduler, typename PointIdRange>
size_t insert(TileContainer& tc, Scheduler& sc, const PointIdRange&);

}

}


// Vertex_const_iterator insert(TileContainer& tc, Scheduler& sc, const Point&, ID )
// size_t insert(TileContainer& tc, Scheduler& sc, const PointRange&)
