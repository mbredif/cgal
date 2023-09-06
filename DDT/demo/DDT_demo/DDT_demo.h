#ifndef CGAL_DDT_DEMO
#define CGAL_DDT_DEMO

#define CGAL_DEBUG_DDT

#include <CGAL/Distributed_triangulation.h>
#include <CGAL/DDT/serializer/VRT_file_serializer.h>
#include <CGAL/DDT/serializer/PVTU_file_serializer.h>
#include <CGAL/DDT/serializer/File_serializer.h>
#include <CGAL/DDT/IO/write_ply.h>
#include <boost/program_options.hpp>
#include "logging.h"

namespace po = boost::program_options;

template<
        class Triangulation,
        class TileIndexProperty,
        class Partitioner,
        class Scheduler,
        class Serializer>

int DDT_demo(int argc, char **argv)
{
  typedef CGAL::DDT::Triangulation_traits<Triangulation> Traits;
  typedef typename Traits::Random_points_in_box Random_points;
  typedef typename Traits::Point Point;
  typedef typename TileIndexProperty::value_type Tile_index;
  typedef CGAL::Distributed_triangulation<Triangulation, TileIndexProperty, Serializer> Distributed_triangulation;
  typedef CGAL::Distributed_point_set<Tile_index, Point>                                Distributed_point_set;

  int NP, loglevel, max_concurrency, max_number_of_tiles;
  std::vector<int> NT;
  std::string vrt, ply, cgal, pvtu, ser;
  double range;
  int dimension = Traits::D;

  po::options_description desc("Allowed options");
  desc.add_options()
  ("help,h", "produce help message")
  ("check", "check validity")
  ("points,p", po::value<int>(&NP)->default_value(10000), "number of points")
  ("log,l", po::value<int>(&loglevel)->default_value(0), "log level")
  ("max_concurrency,j", po::value<int>(&max_concurrency)->default_value(0), "maximum concurrency (0=automatic)")
  ("tiles,t", po::value<std::vector<int>>(&NT), "number of tiles")
  ("range,r", po::value<double>(&range)->default_value(1), "range")
  ("serialize,s", po::value<std::string>(&ser)->default_value("tile_"), "prefix for tile serialization")
  ("vrt", po::value<std::string>(&vrt), "VRT+CSV output basename")
  ("ply", po::value<std::string>(&ply), "PLY output basename")
  ("pvtu", po::value<std::string>(&pvtu), "PVTU+VTU output basename")
  ("cgal", po::value<std::string>(&cgal), "CGAL output basename")
  ("memory,m", po::value<int>(&max_number_of_tiles)->default_value(0), "max number of tiles in memory")
  ;

  if ( Traits::D == 0 )
      desc.add_options()
      ("dimension,d", po::value<int>(&dimension)->default_value(0), "ambient dimension");

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
      if ( dimension < 2 ) {
          std::cerr << "Specifiy the dynamic ambient dimension using --dimension [-d]." << std::endl;
          return -1;
      }
      if ( (int)(NT.size()) > dimension )
      {
          std::cout << "Discarding excess tile grid dimension(s) : ";
          std::copy(NT.begin()+dimension, NT.end(), std::ostream_iterator<int>(std::cout, " "));
          std::cout << std::endl;
          NT.resize(dimension);
      }
  }
  catch(po::error& e)
  {
      std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
      std::cerr << desc << std::endl;
      return -1;
  }

  if (max_number_of_tiles > 0 && (max_concurrency > max_number_of_tiles || max_concurrency == 0))
  {
      std::cout << "Limiting concurrency to the maximum number of tiles in memory : ";
      std::cout << max_concurrency << " --> " << max_number_of_tiles << std::endl;
      max_concurrency = max_number_of_tiles;
  }

  typename Traits::Bbox bbox = Traits::bbox(dimension, range);
  Partitioner partitioner(bbox, NT.begin(), NT.end());
  Scheduler scheduler(max_concurrency);
  Serializer serializer(ser);
  Distributed_triangulation tri(dimension, max_number_of_tiles, serializer);

  std::cout << "- Program     : " << argv[0] << std::endl;
  std::cout << "- Loglevel    : " << loglevel << std::endl;
  std::cout << "- Range       : " << range << std::endl;
  std::cout << "- Points      : " << NP << std::endl;
  std::cout << "- Concurrency : " << scheduler.max_concurrency() << std::endl;
  std::cout << "- memTiles    : " << max_number_of_tiles << std::endl;
  std::cout << "- VRT Out     : " << (vrt.empty() ? "[no output]" : vrt ) << std::endl;
  std::cout << "- PLY Out     : " << (ply.empty() ? "[no output]" : ply ) << std::endl;
  std::cout << "- PVTU Out    : " << (pvtu.empty()? "[no output]" : pvtu) << std::endl;
  std::cout << "- CGAL Out    : " << (cgal.empty()? "[no output]" : cgal) << std::endl;
  std::cout << "- Tiles       : " << partitioner.size() << ", " << partitioner << std::endl;
  std::cout << "- Serializer  : " << serializer << std::endl;

  CGAL::DDT::logging<> log("--- Overall --> ", loglevel);
  log.step("Random_points   ");
  Random_points generator(dimension, range);
  Distributed_point_set points(generator, NP, partitioner);

  log.step("insertion       ");
  std::size_t count = tri.insert(points, scheduler);

  if ( vm.count("vrt")  )
  {
      log.step("write_vrt       ");
      tri.write(CGAL::DDT::VRT_serializer(vrt), scheduler);
  }

  if ( vm.count("cgal")  )
  {
      log.step("write_cgal       ");
      tri.write(CGAL::DDT::File_serializer(cgal), scheduler);
  }

  if ( vm.count("pvtu")  )
  {
      log.step("write_pvtu       ");
      tri.write(CGAL::DDT::PVTU_serializer(pvtu), scheduler);
  }

  if ( vm.count("ply")  )
  {
      log.step("write_ply       ");
      CGAL::DDT::write_ply(tri, ply+".ply");
  }

  if ( vm.count("check")  )
  {
      log.step("validity        ");
      std::cout << "Validity     \t" << (tri.is_valid(true, 5) ? "OK" : "ERROR!") << std::endl;
  }
  std::cout << std::endl << count << " points were inserted." << std::endl;
  return 0;
}

#endif // CGAL_DDT_DEMO
