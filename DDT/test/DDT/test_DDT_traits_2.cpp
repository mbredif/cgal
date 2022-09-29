typedef int Id;
typedef int Flag;

#include <CGAL/DDT/traits/cgal_traits_2.h>
typedef ddt::Cgal_traits_2<Id,Flag> Traits;

#include "test_traits.hpp"

int main(int, char **)
{
    return test_traits<Traits>("test_DDT_traits_2_out", 3, 1000);
}
