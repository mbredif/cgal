typedef int Tile_index;
typedef int Vertex_info;

#include <CGAL/DDT/traits/cgal_traits_d.h>
typedef CGAL::DDT::Cgal_traits_d<Tile_index,Vertex_info> Traits;

#include "test_traits.h"

int main(int, char **)
{
    return test_traits<Traits>("test_DDT_traits_d_out", 3, 1000, 2);
}
