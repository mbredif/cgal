#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/DDT/Delaunay_triangulation_2.h>
#include <CGAL/DDT/traits/Vertex_info_property_map.h>

typedef int                                                                  Tile_index;
typedef CGAL::Exact_predicates_inexact_constructions_kernel                  Geom_traits;

typedef CGAL::Delaunay_triangulation_2<Geom_traits>                          Triangulation;

typedef CGAL::Triangulation_vertex_base_with_info_2<Tile_index, Geom_traits> VbInfo;
typedef CGAL::Triangulation_data_structure_2<VbInfo>                         TDSInfo;
typedef CGAL::Delaunay_triangulation_2<Geom_traits, TDSInfo>                 TriangulationInfo;
typedef CGAL::DDT::Vertex_info_property_map<TriangulationInfo>               PropertyInfo;

typedef CGAL::DDT::Data<Tile_index, unsigned char>                           Data;
typedef CGAL::Triangulation_vertex_base_with_info_2<Data, Geom_traits>       VbData;
typedef CGAL::Triangulation_data_structure_2<VbData>                         TDSData;
typedef CGAL::Delaunay_triangulation_2<Geom_traits, TDSData>                 TriangulationData;
typedef CGAL::DDT::Vertex_info_id_property_map<TriangulationData>            PropertyData;

#include "test_traits.h"

int main(int, char **)
{
    int error = 0;
    error += test_part<Triangulation, Tile_index>("out/test_DDT_2/part");
    error += test_info<TriangulationData, PropertyData>("out/test_DDT_2/data");
    error += test_info<TriangulationInfo, PropertyInfo>("out/test_DDT_2/info");
    std::cout << error << " error(s)." << std::endl;
    return error;
}
