#ifndef CGAL_DDT_DEMO
#define CGAL_DDT_DEMO

#define CGAL_DEBUG_DDT

#include <CGAL/DDT/Tile_container.h>
#include <CGAL/Distributed_Delaunay_triangulation.h>
#include <CGAL/DDT/IO/write_ply.h>
#include <CGAL/DDT/IO/write_vrt.h>
#include <CGAL/DDT/insert.h>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

template<
        class Traits,
        template <class> class Partitioner,
        template <class> class Scheduler,
        template <class> class Serializer>

int DDT_demo(int argc, char **argv)
{
  typedef typename Traits::Random_points_in_box Random_points;
  typedef CGAL::DDT::Tile<Traits> Tile;
  typedef CGAL::DDT::Tile_container<Traits, Tile, Serializer<Tile>> TileContainer;
  typedef CGAL::Distributed_Delaunay_triangulation<TileContainer> Distributed_Delaunay_triangulation;

  int NP, loglevel, max_concurrency, max_number_of_tiles;
  std::vector<int> NT;
  std::string vrt, ply, ser;
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
  typename Traits::Bbox bbox(dimension, range);
  Partitioner<Traits> partitioner(bbox, NT.begin(), NT.end());
  Scheduler<Tile> scheduler(max_concurrency);
  Serializer<Tile> serializer(ser);
  TileContainer tiles(dimension, max_number_of_tiles, serializer);

  std::cout << "- Loglevel    : " << loglevel << std::endl;
  std::cout << "- Range       : " << range << std::endl;
  std::cout << "- Points      : " << NP << std::endl;
  std::cout << "- Concurrency : " << scheduler.max_concurrency() << std::endl;
  std::cout << "- memTiles    : " << max_number_of_tiles << std::endl;
  std::cout << "- VRT Out     : " << (vrt.empty() ? "[no output]" : vrt) << std::endl;
  std::cout << "- PLY Out     : " << (ply.empty() ? "[no output]" : ply) << std::endl;
  std::cout << "- Tiles       : " << partitioner.size() << " ( ";
  std::copy(partitioner.begin(), partitioner.end(), std::ostream_iterator<int>(std::cout, " "));
  std::cout << ")" << std::endl;

  Random_points points(dimension, range);
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
      std::cout << "Validity     \t" << (tri.is_valid(true, 5) ? "OK" : "ERROR!") << std::endl;
  }
  return 0;
}

#endif // CGAL_DDT_DEMO
