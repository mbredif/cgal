
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

/// \ingroup PkgDDTRef
/// Construction of a distributed Delaunay triangulation from a prepartitioned distributed point set
template<typename DistributedPointSet, typename TileContainer, typename Scheduler>
void distributed_Delaunay_triangulation(
        DistributedPointSet distributed_point_set,
        TileContainer tile_container,
        Scheduler scheduler);
}

