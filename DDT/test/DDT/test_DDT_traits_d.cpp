#include <CGAL/Epick_d.h>
#include <CGAL/Triangulation_ds_vertex.h>
#include <CGAL/DDT/Delaunay_triangulation.h>
#include <CGAL/DDT/traits/Vertex_data_property_map.h>

typedef int                                                       Tile_index;
typedef CGAL::Dynamic_dimension_tag                               Dim_tag;
typedef CGAL::Epick_d<CGAL::Dynamic_dimension_tag>                Geom_traits;
typedef CGAL::Triangulation_full_cell<Geom_traits>                Cb;
typedef CGAL::Triangulation_vertex<Geom_traits,Tile_index>        Vb;
typedef CGAL::Triangulation_data_structure<Dim_tag,Vb,Cb>         TDS;
typedef CGAL::Delaunay_triangulation<Geom_traits, TDS>            Triangulation;
typedef CGAL::DDT::Vertex_data_property_map<Triangulation>        TileIndexProperty;

#include "test_traits.h"

int main(int, char **)
{
    return test_traits_grid<Triangulation,TileIndexProperty>("test_DDT_traits_d_out", 3, 1000, 2);
}
