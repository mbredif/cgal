#include <boost/filesystem.hpp>

#include <CGAL/property_map.h>
#include <CGAL/DDT/IO/read_las.h>
#include <CGAL/DDT/IO/write_pvtu.h>

#include <CGAL/DDT/traits/Triangulation_traits_3.h>
#include <CGAL/DDT/partitioner/Grid_partitioner.h>
#include <CGAL/DDT/Tile_container.h>
#include <CGAL/DDT/scheduler/Multithread_scheduler.h>
#include <CGAL/DDT/serializer/File_serializer.h>
#include <CGAL/DDT/insert.h>
#include <CGAL/DDT/tile_points/LAS_tile_points.h>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>
#include <CGAL/DDT/traits/Vertex_info_property_map.h>
#include <CGAL/Delaunay_triangulation_3.h>

#include <utility>
#include <vector>
#include <fstream>

// types
typedef unsigned char Tile_index;
typedef CGAL::Exact_predicates_inexact_constructions_kernel                  Geom_traits;
typedef CGAL::Triangulation_vertex_base_with_info_3<Tile_index, Geom_traits> Vb;
typedef CGAL::Triangulation_data_structure_3<Vb>                             TDS;
typedef CGAL::Delaunay_triangulation_3<Geom_traits, TDS>                     Triangulation;
typedef CGAL::DDT::Vertex_info_property_map<Triangulation>                   TileIndexProperty;

typedef CGAL::DDT::Multithread_scheduler                                     Scheduler;
typedef CGAL::DDT::File_serializer<Triangulation, TileIndexProperty>         Serializer;
typedef CGAL::DDT::LAS_tile_points<Triangulation>                            Tile_points;
typedef CGAL::DDT::Messaging<Triangulation, TileIndexProperty, Tile_points> Messaging;
typedef CGAL::DDT::Messaging_container<Tile_index, Messaging> Messaging_container;
typedef CGAL::DDT::Tile_container<Triangulation, TileIndexProperty, Serializer>           Tile_container;

int main(int argc, char*argv[])
{
    if (argc < 5) {
        std::cerr << "usage : " << argv[0] << " [max_number_of_tiles in memory] [tmp prefix] [out prefix] [las files...]" << std::endl;
        return -1;
    }
    int max_number_of_tiles = atoi(argv[1]);
    const char* tmp  = argv[2];
    const char* out  = argv[3];

    Serializer serializer(tmp);
    Tile_container tiles(3, max_number_of_tiles, serializer);
    Scheduler scheduler;
    Messaging_container messagings;
    for(int i = 0; i< argc - 4; ++i) {
        const char *las = argv[i+4];
        std::size_t num_points = messagings[i].insert(las);
        std::cout << i << " : " << las << " (" << num_points << " points)" << std::endl;
    }

    std::cout << "Inserting points using " << max_number_of_tiles << " tiles at most in memory" << std::endl;
    CGAL::DDT::triangulate(tiles, messagings, scheduler);

    std::cout << "Writing PVTU to " << out << std::endl;
    CGAL::DDT::write_pvtu(tiles, scheduler, out);

    return EXIT_SUCCESS;
}
