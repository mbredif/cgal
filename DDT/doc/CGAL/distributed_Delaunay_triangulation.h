
namespace CGAL {

/// \ingroup PkgDDTRef
/// @todo doc
template<typename PointRange, typename Partitioner, typename TileContainer, typename Scheduler>
void distributed_Delaunay_triangulation(
        PointRange point_range,
        Partitioner partitioner,
        TileContainer tile_container,
        Scheduler scheduler);

}
