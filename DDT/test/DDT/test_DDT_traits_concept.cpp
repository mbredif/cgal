struct Triangulation {};
#include <Concepts/TriangulationTraits.h>
#include <Concepts/Partitioner.h>
#include <Concepts/VertexPropertyMap.h>
#include <Concepts/Scheduler.h>

#include "test_traits.h"


int main(int argc, char **)
{
  typedef ::Partitioner Partitioner;
  typedef ::VertexPropertyMap TileIndexProperty;
  typedef ::Scheduler Scheduler;

  Partitioner partitioner1;
  Partitioner partitioner2;
  // do not run it!
  if (argc == -1) {
    test_traits<Triangulation, TileIndexProperty, Partitioner, Scheduler>(partitioner1, partitioner2,
        "test_DDT_traits_concept", 0);

  }

  return 0;
}
