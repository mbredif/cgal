#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/DDT/Delaunay_triangulation_2.h>
#include <CGAL/DDT/traits/Vertex_info_property_map.h>
#include <CGAL/DDT/partitioner/Grid_partitioner.h>
#include <CGAL/DDT/scheduler/Sequential_scheduler.h>
#include <CGAL/DDT/serializer/File_points_serializer.h>
#include <CGAL/DDT/serializer/VRT_file_serializer.h>
#include <CGAL/Distributed_triangulation.h>

typedef int Tile_index;
typedef CGAL::Exact_predicates_inexact_constructions_kernel                  Geom_traits;
typedef CGAL::Triangulation_vertex_base_with_info_2<Tile_index, Geom_traits> Vb;
typedef CGAL::Triangulation_data_structure_2<Vb>                             TDS;
typedef CGAL::Delaunay_triangulation_2<Geom_traits, TDS>                     Triangulation;
typedef typename Triangulation::Point                                        Point;
typedef CGAL::DDT::Vertex_info_property_map<Triangulation>                   TileIndexProperty;

typedef CGAL::Random_points_in_square_2<Point>                                        Random_points;
typedef CGAL::DDT::Sequential_scheduler                                               Scheduler;
typedef CGAL::DDT::File_points_serializer                                             Serializer;
typedef CGAL::Distributed_triangulation<Triangulation, TileIndexProperty, Serializer> Distributed_triangulation;
typedef CGAL::Distributed_point_set<Tile_index, Point>                                Distributed_point_set;

int main(int argc, char **argv)
{
    std::cout << argv[0] << " [number_of_points] [number_of_tiles_per_axis] [max_number_of_tiles]" << std::endl;
    int number_of_points          = (argc>1) ? atoi(argv[1]) : 1000;
    int number_of_tiles_per_axis  = (argc>2) ? atoi(argv[2]) : 3;
    int max_number_of_tiles       = (argc>3) ? atoi(argv[3]) : 1;
    double range = 1;

    CGAL::Bbox_2 bbox(-range, -range, range, range);
    CGAL::DDT::Grid_partitioner<Triangulation, TileIndexProperty> partitioner(bbox, number_of_tiles_per_axis);
    Serializer serializer("tile_");
    Distributed_triangulation tri(2, max_number_of_tiles, serializer);
    Scheduler scheduler;
    Random_points generator(range);
    Distributed_point_set points(generator, number_of_points, partitioner);
    tri.insert(points, scheduler);

    tri.write(CGAL::DDT::VRT_serializer("out/"), scheduler);

    return 0;
}
