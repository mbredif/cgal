struct PointSet {};
struct Triangulation {};

#include <Concepts/SimplexIndex.h>

struct VertexIndex : public SimplexIndex {};
struct FacetIndex: public SimplexIndex {};
struct CellIndex: public SimplexIndex {};

#include <Concepts/Point.h>
#include <Concepts/Bbox.h>

namespace CGAL{
namespace DDT {
namespace Impl {
    ::Point p;
    ::Bbox b;
    ::VertexIndex v;
    ::CellIndex c;
    ::FacetIndex f;
}
}
}

#include <Concepts/TriangulationTraits.h>
#include <Concepts/PointSetTraits.h>
#include <Concepts/KernelTraits.h>
#include <Concepts/PointSetTraits.h>
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
  }

  return 0;
}
