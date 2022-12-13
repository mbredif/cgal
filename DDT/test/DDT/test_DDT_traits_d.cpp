typedef int Id;
typedef int Flag;

#include <CGAL/DDT/traits/cgal_traits_d.h>
typedef CGAL::DDT::Cgal_traits_d<Id,Flag> Traits;

#include "test_traits.h"

int main(int, char **)
{
    return test_traits<Traits>("test_DDT_traits_d_out", 3, 1000, 2);
}
