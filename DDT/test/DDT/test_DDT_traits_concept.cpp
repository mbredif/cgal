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

  // do not run it!
  if (argc == -1) {
    test_info<Triangulation, TileIndexProperty, Scheduler>("test_DDT_traits_concept");
    test_part<Triangulation, ::TileIndex, Scheduler>("test_DDT_traits_concept");
  }

  return 0;
}
