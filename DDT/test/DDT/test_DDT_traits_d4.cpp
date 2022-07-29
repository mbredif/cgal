typedef unsigned char Id;
typedef unsigned char Flag;

#include <CGAL/DDT/traits/cgal_traits_d.h>
typedef ddt::Cgal_traits<4,Id,Flag> Traits;

#include "test_traits.hpp"

int main(int, char **)
{
    return test_traits<Traits>("test_DDT_traits_d4_out", 2, 100);
}
