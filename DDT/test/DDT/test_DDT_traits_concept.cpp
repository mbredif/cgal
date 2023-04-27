#include "../../../DDT/doc/DDT/Concepts/TriangulationTraits.h"
#include "../../../DDT/doc/DDT/Concepts/Partitioner.h"
#include "test_traits.h"

int main(int argc, char **)
{
  ::Partitioner partitioner;
  // do not run it!
  if (argc == -1)
    test_traits<::TriangulationTraits, ::Partitioner>(partitioner, "test_DDT_traits_concept", 0);

  return 0;
}
