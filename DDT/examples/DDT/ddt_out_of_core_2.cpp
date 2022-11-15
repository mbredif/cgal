#define CGAL_DEBUG_DDT

#include <CGAL/DDT/traits/cgal_traits_2.h>
#include <CGAL/DDT/partitioner/grid_partitioner.h>
#include <CGAL/DDT/Tile_container.h>
#include <CGAL/DDT/scheduler/Sequential_scheduler.h>
#include <CGAL/DDT/serializer/File_serializer.h>
#include <CGAL/Distributed_Delaunay_triangulation.h>
#include <CGAL/DDT/insert.h>

#include <boost/program_options.hpp>

typedef unsigned char Id;
typedef unsigned char Flag;

typedef CGAL::DDT::Cgal_traits_2<Id,Flag> Traits;
typedef Traits::Random_points_in_box Random_points;

typedef CGAL::DDT::Tile<Traits> Tile;
typedef CGAL::DDT::Sequential_scheduler<Tile> Scheduler;
typedef CGAL::DDT::File_serializer<Tile> Serializer;
typedef CGAL::DDT::Tile_container<Traits, Tile, Serializer> TileContainer;
typedef CGAL::Distributed_Delaunay_triangulation<TileContainer> Distributed_Delaunay_triangulation;

namespace po = boost::program_options;

int main(int argc, char **argv)
{
  enum { D = Traits::D };

  int NP;
  std::vector<int> NT;
  std::string out_prefix;
  double range;

  po::options_description desc("Allowed options");
  desc.add_options()
  ("help,h", "produce help message")
  ("check", "check validity")
  ("points,p", po::value<int>(&NP)->default_value(10000), "number of points")
  ("tiles,t", po::value<std::vector<int>>(&NT), "number of tiles")
  ("range,r", po::value<double>(&range)->default_value(1), "range")
  ("output_prefix,o", po::value<std::string>(&out_prefix)->default_value("tile_"), "prefix for tile serialization")
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
  CGAL::DDT::Bbox<2, double> bbox(range);
  CGAL::DDT::grid_partitioner<Traits> partitioner(bbox, NT.begin(), NT.end());

  Serializer serializer(out_prefix);
  TileContainer tiles(serializer);
  Scheduler scheduler;

  std::cout << "- Range          : " << range << std::endl;
  std::cout << "- Points         : " << NP << std::endl;
  std::cout << "- Output prefix  : " << out_prefix <<  std::endl;
  std::cout << "- Tiles          : " << partitioner.size() << " ( ";
  std::copy(partitioner.begin(), partitioner.end(), std::ostream_iterator<int>(std::cout, " "));
  std::cout << ")" << std::endl;

  Random_points points(D, range);
  CGAL::DDT::insert(tiles, scheduler, points, NP, partitioner);

  if ( vm.count("check")  )
  {
    Distributed_Delaunay_triangulation tri(tiles);
    std::cout << "Validity     \t" << (tri.is_valid() ? "OK" : "ERROR!") << std::endl;
  }
  return 0;
}
