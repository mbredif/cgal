#include "../../../DDT/doc/DDT/Concepts/TriangulationTraits.h"
#include "test_traits.h"

int main(int argc, char **)
{
  // do not run it!
  if (argc == 33)
    test_traits<::TriangulationTraits>("test_DDT_traits_2_out", 3, 1000);

  return 0;
}
