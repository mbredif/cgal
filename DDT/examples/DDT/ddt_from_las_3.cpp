#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>
#include <CGAL/DDT/triangulation/Delaunay_triangulation_3.h>
#include <CGAL/DDT/property_map/Vertex_info_property_map.h>
#include <CGAL/Distributed_triangulation.h>

#include <CGAL/DDT/partitioner/Grid_partitioner.h>
#include <CGAL/DDT/scheduler/TBB_scheduler.h>
#include <CGAL/DDT/serializer/File_serializer.h>
#include <CGAL/DDT/point_set/LAS_point_set.h>
#include <CGAL/DDT/serializer/PVTU_file_serializer.h>

// types
typedef unsigned char Tile_index;
typedef CGAL::Exact_predicates_inexact_constructions_kernel                  Geom_traits;
typedef CGAL::Triangulation_vertex_base_with_info_3<Tile_index, Geom_traits> Vb;
typedef CGAL::Triangulation_data_structure_3<Vb>                             TDS;
typedef CGAL::Delaunay_triangulation_3<Geom_traits, TDS>                     Triangulation;
typedef typename Triangulation::Point                                        Point;
typedef CGAL::DDT::Vertex_info_property_map<Triangulation>                   TileIndexProperty;

typedef CGAL::DDT::TBB_scheduler                                             Scheduler;
typedef CGAL::DDT::File_serializer                                           Serializer;
typedef CGAL::Distributed_triangulation<Triangulation, TileIndexProperty, Serializer> Distributed_triangulation;

int main(int argc, char*argv[])
{
    std::cout << argv[0] << " [max_number_of_tiles in memory] [tmp dir] [out dir] [las files...]" << std::endl;
    if (argc < 5 || argc > 260) {
        std::cerr << "maximum number of las files is 256, as tile indices are coded using a uchar8." << std::endl;
        return -1;
    }
    int max_number_of_tiles = atoi(argv[1]);
    char* const tmp  = argv[2];
    char* const out  = argv[3];
    char* const*begin= argv+4; // filenames
    char* const*end  = argv+argc;

    Serializer serializer(tmp);
    Distributed_triangulation tri(3, {}, max_number_of_tiles, serializer);
    Scheduler scheduler;

    auto points = CGAL::DDT::make_distributed_LAS_point_set<Point>(1, begin, end);

    std::cout << "Inserting points using " << max_number_of_tiles << " tiles at most in memory" << std::endl;
    tri.insert(points, scheduler);

    std::cout << "Writing PVTU to " << out << std::endl;
    tri.write(CGAL::DDT::PVTU_serializer(out), scheduler);

    return EXIT_SUCCESS;
}
