#include <boost/filesystem.hpp>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

#include <CGAL/property_map.h>
#include <CGAL/DDT/IO/read_las.h>
#include <CGAL/DDT/IO/write_pvtu.h>

#include <CGAL/DDT/traits/Triangulation_traits_3.h>
#include <CGAL/DDT/partitioner/Grid_partitioner.h>
#include <CGAL/DDT/Tile_container.h>
#include <CGAL/DDT/scheduler/Multithread_scheduler.h>
#include <CGAL/DDT/serializer/File_serializer.h>
#include <CGAL/DDT/insert.h>
#include <CGAL/DDT/Tile_points.h>

#include <CGAL/Bbox_3.h>
#include <CGAL/bounding_box.h>

#include <utility>
#include <vector>
#include <fstream>

// types
typedef unsigned char Tile_index;
typedef unsigned char Vertex_info; // unused user data
typedef CGAL::DDT::Triangulation_traits_3<Tile_index, Vertex_info> Traits;
typedef Traits::Random_points_in_box Random_points;
typedef Traits::Bbox Bbox;
typedef CGAL::DDT::Multithread_scheduler<Traits> Scheduler;
typedef CGAL::DDT::File_serializer<Traits> Serializer;
typedef CGAL::DDT::Tile_points<Traits> Tile_points;
typedef CGAL::DDT::Tile_container<Traits, Tile_points, Serializer> Tile_container;

typedef Traits::Point Point;
// typedef std::array<unsigned short, 4> Color;
// typedef std::pair<Point, Color> PointWithColor;

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
    for(int i = 0; i< argc - 4; ++i) {
        const char *las = argv[i+4];
        std::size_t num_points = tiles[i].insert(las);
        std::cout << i << " : " << las << " (" << num_points << " points)" << std::endl;
    }

    std::cout << "Inserting points using " << max_number_of_tiles << " tiles at most in memory" << std::endl;
    CGAL::DDT::triangulate(tiles, scheduler);

    std::cout << "Writing PVTU to " << out << std::endl;
    CGAL::DDT::write_pvtu(tiles, scheduler, out);

    return EXIT_SUCCESS;
}
