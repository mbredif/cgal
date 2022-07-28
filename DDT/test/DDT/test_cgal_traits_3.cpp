typedef unsigned char Id;
typedef unsigned char Flag;

#include "traits/cgal_traits_3.hpp"
typedef ddt::Cgal_traits_3<Id,Flag> Traits;

#include "test_traits.hpp"

int main(int, char **)
{
    return test_traits<Traits>("cgal_traits_3", 3, 500);
}
