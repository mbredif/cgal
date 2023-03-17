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
typedef CGAL::DDT::Tile<Traits> Tile;
typedef CGAL::DDT::Multithread_scheduler<Traits> Scheduler;
typedef CGAL::DDT::File_serializer<Traits> Serializer;
typedef CGAL::DDT::Tile_container<Traits, Serializer> Tile_container;

typedef Traits::Point Point;
// typedef std::array<unsigned short, 4> Color;
// typedef std::pair<Point, Color> PointWithColor;

int main(int argc, char*argv[])
{
    int max_number_of_tiles = atoi(argv[1]);
    const char* ser_prefix  = argv[2];

    Serializer serializer(ser_prefix);
    Tile_container tiles(3, max_number_of_tiles, serializer);
    Scheduler scheduler;
    for(int i = 0; i< argc - 3; ++i) {
        std::size_t num_points = tiles[i].send_file(argv[i+3]);
        std::cout << i << " : " << argv[i+3] << " (" << num_points << " points)" << std::endl;
    }

    std::cout << "Inserting points" << std::endl;
    CGAL::DDT::impl::insert_received(tiles, scheduler);

    std::cout << "Writing PVTU" << std::endl;
    CGAL::DDT::write_pvtu(tiles, scheduler, "out");

    return EXIT_SUCCESS;
}
