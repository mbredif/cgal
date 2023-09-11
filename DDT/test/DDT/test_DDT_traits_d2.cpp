#include <CGAL/Epick_d.h>
#include <CGAL/Triangulation_ds_vertex.h>
#include <CGAL/DDT/triangulation/Delaunay_triangulation.h>
#include <CGAL/DDT/property_map/Vertex_data_property_map.h>

typedef int                                                       Tile_index;
typedef CGAL::Dimension_tag<2>                                    Dim_tag;
typedef CGAL::Epick_d<Dim_tag>                                    Geom_traits;

typedef CGAL::Delaunay_triangulation<Geom_traits>                 Triangulation;

typedef CGAL::Triangulation_full_cell<Geom_traits>                CbInfo;
typedef CGAL::Triangulation_vertex<Geom_traits,Tile_index>        VbInfo;
typedef CGAL::Triangulation_data_structure<Dim_tag,VbInfo,CbInfo> TDSInfo;
typedef CGAL::Delaunay_triangulation<Geom_traits, TDSInfo>        TriangulationInfo;
typedef CGAL::DDT::Vertex_data_property_map<TriangulationInfo>    PropertyInfo;

typedef CGAL::DDT::Data<Tile_index, unsigned char>                Data;
typedef CGAL::Triangulation_full_cell<Geom_traits>                CbData;
typedef CGAL::Triangulation_vertex<Geom_traits, Data>             VbData;
typedef CGAL::Triangulation_data_structure<Dim_tag,VbData,CbData> TDSData;
typedef CGAL::Delaunay_triangulation<Geom_traits, TDSData>        TriangulationData;
typedef CGAL::DDT::Vertex_data_id_property_map<TriangulationData> PropertyData;

#include "test_traits.h"

int main(int, char **)
{
    int error = 0;
    error += test_part<Triangulation,     Tile_index  >("out/test_DDT_d2/part");
    error += test_info<TriangulationInfo, PropertyInfo>("out/test_DDT_d2/info");
    error += test_info<TriangulationData, PropertyData>("out/test_DDT_d2/data");
    std::cout << error << " error(s)." << std::endl;
    return error;
}
