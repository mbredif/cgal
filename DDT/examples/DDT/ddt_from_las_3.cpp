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
typedef CGAL::DDT::Vertex_info_property_map<Triangulation>                   TileIndexProperty;

typedef CGAL::DDT::Multithread_scheduler                                     Scheduler;
typedef CGAL::DDT::File_serializer<Triangulation, TileIndexProperty>         Serializer;
typedef CGAL::DDT::LAS_tile_points<Triangulation>                            Tile_points;
typedef CGAL::DDT::Messaging<Tile_index, Tile_points::Point, Tile_points> Messaging;
typedef CGAL::DDT::Messaging_container<Messaging> Messaging_container;
typedef CGAL::DDT::Tile_container<Triangulation, TileIndexProperty, Serializer>           Tile_container;
typedef CGAL::Distributed_triangulation<Tile_container>             Distributed_triangulation;


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
    Distributed_triangulation tri(3, max_number_of_tiles, serializer);
    Scheduler scheduler;
    Messaging_container messagings;
    for(Tile_index i = 0; i< argc - 4; ++i) {
        const char *las = argv[i+4];
        std::size_t num_points = messagings[i].insert(las);
        std::cout << std::to_string(i) << " : " << las << " (" << num_points << " points)" << std::endl;
    }

    std::cout << "Inserting points using " << max_number_of_tiles << " tiles at most in memory" << std::endl;
    tri.insert(scheduler, messagings);

    std::cout << "Writing PVTU to " << out << std::endl;
    CGAL::DDT::write_pvtu(tri.tiles, scheduler, out);

    return EXIT_SUCCESS;
}
