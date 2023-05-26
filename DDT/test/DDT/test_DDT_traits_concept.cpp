struct Triangulation {};
#include "../../../DDT/doc/DDT/Concepts/TriangulationTraits.h"
#include "../../../DDT/doc/DDT/Concepts/Partitioner.h"
#include "../../../DDT/doc/DDT/Concepts/VertexPropertyMap.h"

#include "test_traits.h"


int main(int argc, char **)
{
  typedef ::Partitioner Partitioner;
  typedef ::VertexPropertyMap TileIndexProperty;

  Partitioner partitioner;
  // do not run it!
  if (argc == -1)
    test_traits<Triangulation, TileIndexProperty, Partitioner>(partitioner, "test_DDT_traits_concept", 0);

  typedef CGAL::DDT::Tile_container<::Triangulation, TileIndexProperty> TileContainer;
  typedef CGAL::Distributed_triangulation<TileContainer> Distributed_triangulation;


  TileContainer tiles;
  Distributed_triangulation ddt(tiles);

  return 0;
}
