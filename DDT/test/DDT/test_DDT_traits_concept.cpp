struct Triangulation {};
#include <Concepts/TriangulationTraits.h>
#include <Concepts/Partitioner.h>
#include <Concepts/VertexPropertyMap.h>

#include "test_traits.h"


int main(int argc, char **)
{
  typedef ::Partitioner Partitioner;
  typedef ::VertexPropertyMap TileIndexProperty;

  Partitioner partitioner;
  // do not run it!
  if (argc == -1)
    test_traits<Triangulation, TileIndexProperty, Partitioner>(partitioner, "test_DDT_traits_concept", 0);

  return 0;
}
