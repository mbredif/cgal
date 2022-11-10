typedef int Id;
typedef int Flag;

#include <CGAL/DDT/traits/cgal_traits_d.h>
typedef CGAL::DDT::Cgal_traits<2,Id,Flag> Traits;

#include "test_traits.hpp"

int main(int, char **)
{
    return test_traits<Traits>("test_DDT_traits_d2_out", 3, 1000);
}
