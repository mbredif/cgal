typedef int Tile_index;
typedef int Vertex_info;

#include <CGAL/DDT/traits/Triangulation_traits_3.h>
typedef CGAL::DDT::Triangulation_traits_3<Tile_index,Vertex_info> Traits;

#include "test_traits.h"

int main(int, char **)
{
    return test_traits_grid<Traits>("test_DDT_traits_3_out", 3, 300);
}
