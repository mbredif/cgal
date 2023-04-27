typedef int Tile_index;
typedef int Vertex_info;

#include <CGAL/DDT/traits/Triangulation_traits_d.h>
typedef CGAL::DDT::Triangulation_traits_d<Tile_index,Vertex_info> Traits;

#include "test_traits.h"

int main(int, char **)
{
    return test_traits_grid<Traits>("test_DDT_traits_d_out", 3, 1000, 2);
}
