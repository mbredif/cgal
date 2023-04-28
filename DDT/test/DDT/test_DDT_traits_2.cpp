#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/DDT/traits/Data.h>

typedef int Tile_index;
typedef int Vertex_info;
typedef CGAL::DDT::Data<Tile_index, Vertex_info>                        Data;

typedef CGAL::Exact_predicates_inexact_constructions_kernel                  Geom_traits;
//typedef CGAL::Triangulation_vertex_base_with_info_2<Data, Geom_traits> Vb;
typedef CGAL::Triangulation_vertex_base_with_info_2<Tile_index, Geom_traits> Vb;
typedef CGAL::Triangulation_data_structure_2<Vb>                             TDS;
typedef CGAL::Delaunay_triangulation_2<Geom_traits, TDS>                     Delaunay_triangulation;

#include <CGAL/DDT/traits/Triangulation_traits_2.h>
typedef CGAL::DDT::Triangulation_traits_2<Delaunay_triangulation> Traits;

#include "test_traits.h"

int main(int, char **)
{
    return test_traits_grid<Traits>("test_DDT_traits_2_out", 3, 1000);
}
