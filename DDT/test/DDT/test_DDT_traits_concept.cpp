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

  typedef CGAL::Distributed_triangulation<::Triangulation, TileIndexProperty> Distributed_triangulation;

  Distributed_triangulation tri;

  return 0;
}
