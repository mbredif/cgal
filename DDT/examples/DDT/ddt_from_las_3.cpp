#include <boost/filesystem.hpp>

#include <CGAL/property_map.h>
#include <CGAL/DDT/IO/read_las.h>
#include <CGAL/DDT/IO/write_pvtu.h>

#include <CGAL/DDT/traits/Triangulation_traits_3.h>
#include <CGAL/DDT/partitioner/Grid_partitioner.h>
#include <CGAL/DDT/scheduler/Multithread_scheduler.h>
#include <CGAL/DDT/serializer/File_serializer.h>
#include <CGAL/DDT/tile_points/LAS_tile_points.h>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>
#include <CGAL/DDT/traits/Vertex_info_property_map.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Distributed_triangulation.h>

#include <utility>
#include <vector>
#include <fstream>

// types
typedef unsigned char Tile_index;
typedef CGAL::Exact_predicates_inexact_constructions_kernel                  Geom_traits;
typedef CGAL::Triangulation_vertex_base_with_info_3<Tile_index, Geom_traits> Vb;
typedef CGAL::Triangulation_data_structure_3<Vb>                             TDS;
typedef CGAL::Delaunay_triangulation_3<Geom_traits, TDS>                     Triangulation;
typedef typename Triangulation::Point                                        Point;
typedef CGAL::DDT::Vertex_info_property_map<Triangulation>                   TileIndexProperty;

typedef CGAL::DDT::Multithread_scheduler                                     Scheduler;
typedef CGAL::DDT::File_serializer<Triangulation, TileIndexProperty>         Serializer;
typedef CGAL::DDT::LAS_tile_points<Point>                                    Tile_points;
typedef CGAL::Distributed_point_set<Point, Tile_index, Tile_points>                   Distributed_point_set;
typedef CGAL::Distributed_triangulation<Triangulation, TileIndexProperty, Serializer> Distributed_triangulation;


int main(int argc, char*argv[])
{
    if (argc < 5 || argc > 260) {
        std::cerr << "usage : " << argv[0] << " [max_number_of_tiles in memory] [tmp prefix] [out prefix] [las files...]" << std::endl;
        std::cerr << "maximum number of las files is 256, as tile indices are coded using a uchar8." << std::endl;
        return -1;
    }
    int max_number_of_tiles = atoi(argv[1]);
    char* const tmp  = argv[2];
    char* const out  = argv[3];
    char* const*begin= argv+4; // filenames
    char* const*end  = argv+argc;

    Serializer serializer(tmp);
    Distributed_triangulation tri(3, max_number_of_tiles, serializer);
    Scheduler scheduler;

    Distributed_point_set points(begin, end);

    std::cout << "Inserting points using " << max_number_of_tiles << " tiles at most in memory" << std::endl;
    tri.insert(scheduler, points);

    std::cout << "Writing PVTU to " << out << std::endl;
    CGAL::DDT::write_pvtu(tri, scheduler, out);

    return EXIT_SUCCESS;
}
