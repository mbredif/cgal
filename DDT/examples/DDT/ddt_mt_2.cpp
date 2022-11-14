typedef unsigned char Id;
typedef unsigned char Flag;

#include <CGAL/DDT/traits/cgal_traits_2.h>
//#include <CGAL/DDT/traits/cgal_traits_3.h>
//#include <CGAL/DDT/traits/cgal_traits_d.h>

typedef CGAL::DDT::Cgal_traits_2<Id,Flag> Traits;
//typedef CGAL::DDT::Cgal_traits_3<Id,Flag> Traits;
//typedef CGAL::DDT::Cgal_traits_d<Id,Flag> Traits; // dynamic
//typedef CGAL::DDT::Cgal_traits<2,Id,Flag> Traits;
//typedef CGAL::DDT::Cgal_traits<3,Id,Flag> Traits;
//typedef CGAL::DDT::Cgal_traits<4,Id,Flag> Traits;

typedef Traits::Random_points_in_box Random_points;

#include <CGAL/DDT/partitioner/grid_partitioner.h>
#define DDT_USE_THREADS 1
#include <CGAL/DDT/Tile_container.h>
typedef CGAL::DDT::Tile<Traits> Tile;

//#include <CGAL/DDT/scheduler/Sequential_scheduler.h>
//typedef CGAL::DDT::Sequential_scheduler<Tile> Scheduler;
//#include <CGAL/DDT/scheduler/Multithread_scheduler.h>
//typedef CGAL::DDT::Multithread_scheduler<Tile> Scheduler;
#include <CGAL/DDT/scheduler/TBB_scheduler.h>
typedef CGAL::DDT::TBB_scheduler<Tile> Scheduler;
//#include <CGAL/DDT/scheduler/MPI_scheduler.h>
//typedef CGAL::DDT::MPI_scheduler<Tile> Scheduler;

#include <CGAL/DDT/serializer/file_serializer.h>
typedef CGAL::DDT::File_serializer<Id,Tile> Serializer;

typedef CGAL::DDT::Tile_container<Traits, Serializer> TileContainer;
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

    int NP, loglevel;
    std::vector<int> NT;
    std::string out;
    double range;

    po::options_description desc("Allowed options");
    desc.add_options()
    ("help,h", "produce help message")
    ("check", "check validity")
    ("points,p", po::value<int>(&NP)->default_value(10000), "number of points")
    ("log,l", po::value<int>(&loglevel)->default_value(0), "log level")
    //("threads,j", po::value<int>(&threads)->default_value(0), "number of threads (0=all)")
    ("tiles,t", po::value<std::vector<int>>(&NT), "number of tiles")
    ("range,r", po::value<double>(&range)->default_value(1), "range")
    ("out,o", po::value<std::string>(&out), "output directory")
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
    CGAL::DDT::Bbox<D, double> bbox(range);
    CGAL::DDT::grid_partitioner<Traits> partitioner(bbox, NT.begin(), NT.end());

    Serializer serializer;
    serializer.add(0, "0.txt");
    serializer.add(1, "1.txt");
    TileContainer tiles(serializer);
    Scheduler scheduler;

    std::cout << "- Loglevel : " << loglevel << std::endl;
    std::cout << "- Range    : " << range << std::endl;
    std::cout << "- Points   : " << NP << std::endl;
    std::cout << "- Threads  : " << scheduler.number_of_threads() << std::endl;
    std::cout << "- Out dir  : " << (out.empty() ? "[no output]" : out) << std::endl;
    std::cout << "- Tiles    : " << partitioner.size() << " ( ";
    std::copy(partitioner.begin(), partitioner.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << ")" << std::endl;

    Random_points points(D, range);
    CGAL::DDT::insert(tiles, scheduler, points, NP, partitioner);
    Distributed_Delaunay_triangulation tri(tiles);

    if ( vm.count("out")  )
    {
        CGAL::DDT::logging<> log("Writing      ", loglevel);
        CGAL::DDT::write_ply(tiles, out+".ply");

        CGAL::DDT::write_vrt_cell(tri, out+"_c.vrt");
        CGAL::DDT::write_vrt_vert(tri, out+"_v.vrt");
    }

    if ( vm.count("check")  )
    {
        CGAL::DDT::logging<> log("Validity     ", loglevel);
        std::cout << "Validity     \t" << (tri.is_valid() ? "OK" : "ERROR!") << std::endl;
    }
    return 0;
}
