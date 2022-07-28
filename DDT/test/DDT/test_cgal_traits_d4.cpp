typedef unsigned char Id;
typedef unsigned char Flag;

#include "traits/cgal_traits_d.hpp"
typedef ddt::Cgal_traits<4,Id,Flag> Traits;

#include "test_traits.hpp"

int main(int, char **)
{
    return test_traits<Traits>("cgal_traits_d4", 2, 100);
}
