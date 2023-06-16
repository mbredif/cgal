#include <CGAL/DDT/traits/Triangulation_traits_2.h>
#include <CGAL/DDT/partitioner/Grid_partitioner.h>
#include <CGAL/DDT/scheduler/Sequential_scheduler.h>
#include <CGAL/DDT/serializer/File_points_serializer.h>
#include <CGAL/Distributed_triangulation.h>
#include <CGAL/DDT/IO/write_ply.h>
#include <CGAL/DDT/IO/write_vrt.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/DDT/traits/Vertex_info_property_map.h>
#include <CGAL/Delaunay_triangulation_2.h>

typedef int Tile_index;
typedef CGAL::Exact_predicates_inexact_constructions_kernel                  Geom_traits;
typedef CGAL::Triangulation_vertex_base_with_info_2<Tile_index, Geom_traits> Vb;
typedef CGAL::Triangulation_data_structure_2<Vb>                             TDS;
typedef CGAL::Delaunay_triangulation_2<Geom_traits, TDS>                     Triangulation;
typedef CGAL::DDT::Vertex_info_property_map<Triangulation>                   TileIndexProperty;

typedef CGAL::Random_points_in_square_2<typename Triangulation::Point>                Random_points;
typedef CGAL::DDT::Sequential_scheduler                                               Scheduler;
typedef CGAL::DDT::File_points_serializer<Triangulation, TileIndexProperty>           Serializer;
typedef CGAL::Distributed_triangulation<Triangulation, TileIndexProperty, Serializer> Distributed_triangulation;

int main(int argc, char **argv)
{
    int number_of_points          = (argc>1) ? atoi(argv[1]) : 1000;
    int number_of_tiles_per_axis  = (argc>2) ? atoi(argv[2]) : 3;
    int max_number_of_tiles       = (argc>3) ? atoi(argv[3]) : 1;
    double range = 1;

    CGAL::Bbox_2 bbox(-range, -range, range, range);
    CGAL::DDT::Grid_partitioner<Triangulation, TileIndexProperty> partitioner(bbox, number_of_tiles_per_axis);
    Serializer serializer("tile_");
    Distributed_triangulation tri(2, max_number_of_tiles, serializer);
    Scheduler scheduler;
    Random_points points(range);
    tri.insert(scheduler, points, number_of_points, partitioner);

    CGAL::DDT::write_vrt_verts(tri, scheduler, "out_v");
    CGAL::DDT::write_vrt_facets(tri, scheduler, "out_f");
    CGAL::DDT::write_vrt_cells(tri, scheduler, "out_c");
    CGAL::DDT::write_vrt_bboxes(tri, "out_b");
    CGAL::DDT::write_vrt_tins(tri, scheduler, "out_t");

    return 0;
}
