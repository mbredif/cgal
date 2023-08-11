#include <CGAL/DDT/traits/Triangulation_traits_2.h>
#include <CGAL/DDT/partitioner/Grid_partitioner.h>
#include <CGAL/DDT/scheduler/Multithread_scheduler.h>
#include <CGAL/DDT/serializer/File_serializer.h>
#include <CGAL/DDT/serializer/VRT_file_serializer.h>
#include <CGAL/Distributed_triangulation.h>
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
typedef typename Triangulation::Point_2                                      Point;

typedef CGAL::Random_points_in_square_2<typename Triangulation::Point>       Random_points;
typedef CGAL::DDT::Multithread_scheduler                                     Scheduler;
typedef CGAL::DDT::File_serializer                                           Serializer;
typedef CGAL::Distributed_triangulation<Triangulation, TileIndexProperty, Serializer>             Distributed_triangulation;
typedef CGAL::Distributed_point_set<Point, Tile_index>                   Distributed_point_set;

int main(int argc, char **argv)
{
    std::cout << argv[0] << " [number_of_points] [number_of_tiles_per_axis] [threads] [max_number_of_tiles_in_mem]" << std::endl;
    int number_of_points           = (argc>1) ? atoi(argv[1]) : 1000;
    int number_of_tiles_per_axis   = (argc>2) ? atoi(argv[2]) : 3;
    int threads                    = (argc>3) ? atoi(argv[3]) : 0;
    int max_number_of_tiles_in_mem = (argc>4) ? atoi(argv[4]) : 0;
    double range = 1;

    CGAL::Bbox_2 bbox(-range, -range, range, range);
    CGAL::DDT::Grid_partitioner<Triangulation, TileIndexProperty> partitioner(bbox, number_of_tiles_per_axis);
    Random_points generator(range);
    Distributed_point_set points(generator, number_of_points, partitioner);

    Scheduler scheduler(threads);

    CGAL::DDT::File_serializer serializer("tile/");
    Distributed_triangulation tri(2, max_number_of_tiles_in_mem, serializer);

    tri.insert(scheduler, points);

    tri.write(scheduler, CGAL::DDT::VRT_serializer("out/"));

    CGAL::DDT::File_serializer serializer2("tile2/");
    Distributed_triangulation tri2(3, max_number_of_tiles_in_mem, serializer2);
    CGAL::DDT::Grid_partitioner<Triangulation, TileIndexProperty> partitioner2(bbox, number_of_tiles_per_axis + 1);
    tri2.partition(scheduler, partitioner2, tri);
    tri2.write(scheduler, CGAL::DDT::VRT_serializer("out2/"));

    return 0;
}
