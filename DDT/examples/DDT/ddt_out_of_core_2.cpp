#define CGAL_DEBUG_DDT

#include <CGAL/DDT/traits/cgal_traits_2.h>
//#include <CGAL/DDT/partitioner/random_partitioner.h>
#include <CGAL/DDT/partitioner/grid_partitioner.h>
#include <CGAL/DDT/Tile_container.h>
#include <CGAL/DDT/scheduler/Sequential_scheduler.h>
#include <CGAL/DDT/serializer/File_serializer.h>
#include <CGAL/Distributed_Delaunay_triangulation.h>
#include <CGAL/DDT/insert.h>
#include <CGAL/DDT/IO/write_vrt.h>

#include <boost/program_options.hpp>

typedef int Id;
typedef CGAL::DDT::Cgal_traits_2<Id> Traits;
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
  std::string ser, vrt;
  double range;
  int max_number_of_tiles;

  po::options_description desc("Allowed options");
  desc.add_options()
  ("help,h", "produce help message")
  ("check", "check validity")
  ("points,p", po::value<int>(&NP)->default_value(10000), "number of points")
  ("tiles,t", po::value<std::vector<int>>(&NT), "number of tiles")
  ("range,r", po::value<double>(&range)->default_value(1), "range")
  ("serialize_prefix,s", po::value<std::string>(&ser)->default_value("tile_"), "prefix for tile serialization")
  ("vrt", po::value<std::string>(&vrt)->default_value("out"), "prefix for vrt output")
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
  CGAL::DDT::Bbox<2, double> bbox(range);
  CGAL::DDT::grid_partitioner<Traits> partitioner(bbox, NT.begin(), NT.end());
//  CGAL::DDT::random_partitioner<Traits> partitioner(0, NT[0]-1);

  Serializer serializer(ser);
  TileContainer tiles(max_number_of_tiles, serializer);
  Scheduler scheduler;

  std::cout << "- Range          : " << range << std::endl;
  std::cout << "- Points         : " << NP << std::endl;
  std::cout << "- Output prefix  : " << ser <<  std::endl;
  std::cout << "- VRT Out  : " << (vrt.empty() ? "[no output]" : vrt) << std::endl;
  std::cout << "- Tiles          : " << partitioner.size() << " ( ";
  std::copy(NT.begin(), NT.end(), std::ostream_iterator<int>(std::cout, " "));
  std::cout << ")" << std::endl;

  Random_points points(D, range);
  size_t n = CGAL::DDT::insert(tiles, scheduler, points, NP, partitioner);
  std::cout << n << " points inserted" << std::endl;

  Distributed_Delaunay_triangulation tri(tiles);
  if ( vm.count("vrt")  )
  {
      CGAL::DDT::write_vrt_verts(tiles, scheduler, vrt+"_v");
      CGAL::DDT::write_vrt_facets(tiles, scheduler, vrt+"_f");
      CGAL::DDT::write_vrt_cells(tiles, scheduler, vrt+"_c");
  }

  if ( vm.count("check")  )
  {
    std::cout << "Validity     \t" << (tri.is_valid() ? "OK" : "ERROR!") << std::endl;
  }
  return 0;
}
