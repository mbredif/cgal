typedef int Id;
typedef int Flag;

#include <CGAL/DDT/traits/cgal_traits_d.hpp>
typedef ddt::Cgal_traits<2,Id,Flag> Traits;

#include "test_traits.hpp"

int main(int, char **)
{
    return test_traits<Traits>("cgal_traits_d2", 3, 1000);
}
