#include <boost/filesystem.hpp>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

#include <CGAL/property_map.h>
#include <CGAL/IO/read_las_points.h>
#include <CGAL/DDT/IO/write_ply.h>

#include <CGAL/DDT/traits/Triangulation_traits_3.h>
#include <CGAL/DDT/partitioner/Grid_partitioner.h>
#include <CGAL/DDT/Tile_container.h>
#include <CGAL/DDT/scheduler/Multithread_scheduler.h>
#include <CGAL/DDT/serializer/File_serializer.h>


#include <CGAL/Distributed_Delaunay_triangulation.h>
#include <CGAL/DDT/IO/write_ply.h>
#include <CGAL/DDT/insert.h>

#include <CGAL/Bbox_3.h>
#include <CGAL/bounding_box.h>

#include <utility>
#include <vector>
#include <fstream>

// types
typedef int Tile_index;
typedef unsigned char Vertex_info; // unused user data
typedef CGAL::DDT::Triangulation_traits_3<Tile_index, Vertex_info> Traits;
typedef Traits::Random_points_in_box Random_points;
typedef Traits::Bbox Bbox;
typedef CGAL::DDT::Tile<Traits> Tile;
typedef CGAL::DDT::Multithread_scheduler<Traits> Scheduler;
typedef CGAL::DDT::File_serializer<Traits> Serializer;
typedef CGAL::DDT::Tile_container<Traits, Serializer> Tile_container;
typedef CGAL::Distributed_Delaunay_triangulation<Tile_container> Distributed_Delaunay_triangulation;


typedef Traits::Point Point;
// typedef std::array<unsigned short, 4> Color;
// typedef std::pair<Point, Color> PointWithColor;

int main(int argc, char*argv[])
{
    const char* fname = (argc>1) ? argv[1] : "data/pig_points.las";

    // Reads a .las point set file with normal vectors and colors
    std::ifstream in(fname, std::ios_base::binary);
    std::vector<Point> points; // store points
    std::cout << "reading las.." << std::endl;
    if(!CGAL::IO::read_LAS(in, std::back_inserter (points)))
	{
	    std::cerr << "Error: cannot read file " << fname << std::endl;
	    return EXIT_FAILURE;
	}

    enum { D = Traits::D };
    int max_number_of_tiles   = (argc>3) ? atoi(argv[3]) : 1;
    double range = 1;
    int number_of_tiles_per_axis = 3;
    int number_of_points = points.size();
    CGAL::Bbox_3 bbox = bbox_3(points.begin(),points.end());
    
    CGAL::DDT::Grid_partitioner<Traits> partitioner(bbox, number_of_tiles_per_axis);
    Serializer serializer("tile_");
    Tile_container tiles(D, max_number_of_tiles, serializer);
    Scheduler scheduler;
    std::cout << "start DDT" << std::endl;
    CGAL::DDT::insert(tiles, scheduler, points.begin(), number_of_points, partitioner);

    Distributed_Delaunay_triangulation tri(tiles);

    if(!tri.is_valid())
    {
        std::cerr << "tri is not valid" << std::endl;
        return 1;
    }

    const std::string& testname = "out/out.ply";
    boost::filesystem::create_directories(testname);
    std::cout << "== write_ply ==" << std::endl;
    CGAL::DDT::write_ply(tiles, testname + "/out.ply");   
    return EXIT_SUCCESS;
}
