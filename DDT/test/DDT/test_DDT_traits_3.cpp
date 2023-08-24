#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>
#include <CGAL/DDT/Delaunay_triangulation_3.h>
#include <CGAL/DDT/traits/Vertex_info_property_map.h>

typedef int                                                                  Tile_index;
typedef CGAL::Exact_predicates_inexact_constructions_kernel                  Geom_traits;
typedef CGAL::Triangulation_vertex_base_with_info_3<Tile_index, Geom_traits> Vb;
typedef CGAL::Triangulation_data_structure_3<Vb>                             TDS;
typedef CGAL::Delaunay_triangulation_3<Geom_traits, TDS>                     Triangulation;
typedef CGAL::DDT::Vertex_info_property_map<Triangulation>                   TileIndexProperty;

#include "test_traits.h"

int main(int, char **)
{
    return test_traits_grid<Triangulation,TileIndexProperty>("test_DDT_traits_3_out", 3, 300);
}
