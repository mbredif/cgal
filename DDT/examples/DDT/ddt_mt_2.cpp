#define CGAL_DEBUG_DDT

typedef int  Id;
typedef unsigned char Info; // unused user data

#include <CGAL/DDT/traits/cgal_traits_2.h>
//#include <CGAL/DDT/traits/cgal_traits_3.h>
//#include <CGAL/DDT/traits/cgal_traits_d.h>

typedef CGAL::DDT::Cgal_traits_2<Id, Info> Traits;
//typedef CGAL::DDT::Cgal_traits_3<Id, Info> Traits;
//typedef CGAL::DDT::Cgal_traits_d<Id, Info Traits; // dynamic
//typedef CGAL::DDT::Cgal_traits<2,Id, Info> Traits;
//typedef CGAL::DDT::Cgal_traits<3,Id, Info> Traits;
//typedef CGAL::DDT::Cgal_traits<4,Id, Info> Traits;

typedef Traits::Random_points_in_box Random_points;
typedef Traits::Bbox Bbox;

#include <CGAL/DDT/partitioner/Grid_partitioner.h>
#define DDT_USE_THREADS 1
#include <CGAL/DDT/Tile_container.h>
typedef CGAL::DDT::Tile<Traits> Tile;

//#include <CGAL/DDT/scheduler/Sequential_scheduler.h>
//typedef CGAL::DDT::Sequential_scheduler<Tile> Scheduler;
#include <CGAL/DDT/scheduler/Multithread_scheduler.h>
typedef CGAL::DDT::Multithread_scheduler<Tile> Scheduler;
//#include <CGAL/DDT/scheduler/TBB_scheduler.h>
//typedef CGAL::DDT::TBB_scheduler<Tile> Scheduler;
//#include <CGAL/DDT/scheduler/MPI_scheduler.h>
//typedef CGAL::DDT::MPI_scheduler<Tile> Scheduler;

#include <CGAL/DDT/serializer/File_serializer.h>
typedef CGAL::DDT::File_serializer<Tile> Serializer;
typedef CGAL::DDT::Tile_container<Traits, Tile, Serializer> TileContainer;
//typedef CGAL::DDT::Tile_container<Traits> TileContainer;
#include <CGAL/Distributed_Delaunay_triangulation.h>
typedef CGAL::Distributed_Delaunay_triangulation<TileContainer> Distributed_Delaunay_triangulation;
#include <CGAL/DDT/IO/write_ply.h>
#include <CGAL/DDT/IO/write_vrt.h>
#include <CGAL/DDT/IO/logging.h>

#include <CGAL/DDT/insert.h>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

int main(int argc, char **argv)
{
    enum { D = Traits::D };

    int NP, loglevel, threads, max_number_of_tiles;
    std::vector<int> NT;
    std::string vrt, ply, ser;
    double range;

    po::options_description desc("Allowed options");
    desc.add_options()
    ("help,h", "produce help message")
    ("check", "check validity")
    ("points,p", po::value<int>(&NP)->default_value(10000), "number of points")
    ("log,l", po::value<int>(&loglevel)->default_value(0), "log level")
    ("threads,j", po::value<int>(&threads)->default_value(0), "number of threads (0=all)")
    ("tiles,t", po::value<std::vector<int>>(&NT), "number of tiles")
    ("range,r", po::value<double>(&range)->default_value(1), "range")
    ("serialize,s", po::value<std::string>(&ser)->default_value("tile_"), "prefix for tile serialization")
    ("vrt", po::value<std::string>(&vrt), "VRT+CSV output basename")
    ("ply", po::value<std::string>(&ply), "PLY output basename")
    ("memory,m", po::value<int>(&max_number_of_tiles)->default_value(1), "max number of tiles in memory")
    ;

    po::variables_map vm;
    try
    {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        if ( vm.count("help")  )
        {
            std::cout << "Distributed Delaunay Triangulation" << std::endl
                      << desc << std::endl;
            return 0;
        }
        po::notify(vm);
        if ( NT.size() > D )
        {
            std::cout << "Discarding excess tile grid dimension(s) : ";
            std::copy(NT.begin()+D, NT.end(), std::ostream_iterator<int>(std::cout, " "));
            std::cout << std::endl;
            NT.resize(D);
        }
    }
    catch(po::error& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
        std::cerr << desc << std::endl;
        return -1;
    }

    Bbox bbox(D, range);
    CGAL::DDT::Grid_partitioner<Traits> partitioner(bbox, NT.begin(), NT.end());

    if (0 < max_number_of_tiles) {
        if (threads == 0) threads = std::thread::hardware_concurrency();
        if (max_number_of_tiles < threads) threads = max_number_of_tiles;
    }

    Serializer serializer(ser);
    TileContainer tiles(D, max_number_of_tiles, serializer);
    Scheduler scheduler(threads);

    std::cout << "- Loglevel : " << loglevel << std::endl;
    std::cout << "- Range    : " << range << std::endl;
    std::cout << "- Points   : " << NP << std::endl;
    std::cout << "- Threads  : " << scheduler.max_concurrency() << std::endl;
    std::cout << "- memTiles : " << max_number_of_tiles << std::endl;
    std::cout << "- VRT Out  : " << (vrt.empty() ? "[no output]" : vrt) << std::endl;
    std::cout << "- PLY Out  : " << (ply.empty() ? "[no output]" : ply) << std::endl;
    std::cout << "- Tiles    : " << partitioner.size() << " ( ";
    std::copy(partitioner.begin(), partitioner.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << ")" << std::endl;

    Random_points points(D, range);
    CGAL::DDT::insert(tiles, scheduler, points, NP, partitioner);
    Distributed_Delaunay_triangulation tri(tiles);

    if ( vm.count("vrt")  )
    {
        CGAL::DDT::write_vrt_verts(tiles, scheduler, vrt+"_v");
        CGAL::DDT::write_vrt_facets(tiles, scheduler, vrt+"_f");
        CGAL::DDT::write_vrt_cells(tiles, scheduler, vrt+"_c");
        CGAL::DDT::write_vrt_bboxes(tiles, vrt+"_b");
        CGAL::DDT::write_vrt_tins(tiles, scheduler, vrt+"_t");
    }

    if ( vm.count("ply")  )
    {
        CGAL::DDT::write_ply(tiles, ply+".ply");
    }

    if ( vm.count("check")  )
    {
        CGAL::DDT::logging<> log("Validity     ", loglevel);
        std::cout << "Validity     \t" << (tri.is_valid(true, 5) ? "OK" : "ERROR!") << std::endl;
    }
    return 0;
}
