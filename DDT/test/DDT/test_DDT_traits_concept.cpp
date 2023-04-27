#include "../../../DDT/doc/DDT/Concepts/TriangulationTraits.h"
#include "../../../DDT/doc/DDT/Concepts/Partitioner.h"
#include "test_traits.h"

int main(int argc, char **)
{
  typedef ::TriangulationTraits Traits;
  typedef ::Partitioner Partitioner;

  Partitioner partitioner;
  // do not run it!
  if (argc == -1)
    test_traits<Traits, Partitioner>(partitioner, "test_DDT_traits_concept", 0);

  typedef CGAL::DDT::Tile_container<Traits> TileContainer;
  typedef CGAL::Distributed_Delaunay_triangulation<TileContainer> Distributed_Delaunay_triangulation;


  TileContainer tiles;
  Distributed_Delaunay_triangulation ddt(tiles);

  return 0;
}
