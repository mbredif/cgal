#include <CGAL/Epick_d.h>
#include <CGAL/Delaunay_triangulation.h>
#include <CGAL/DDT/traits/Vertex_data_property_map.h>
#include <CGAL/DDT/traits/Triangulation_traits_d.h>
#include <CGAL/DDT/partitioner/Grid_partitioner.h>
#include <CGAL/DDT/scheduler/Sequential_scheduler.h>
#include <CGAL/DDT/serializer/File_serializer.h>

#include "DDT_demo.h"

typedef int Tile_index;
typedef CGAL::Dynamic_dimension_tag                               Dim_tag;
typedef CGAL::Epick_d<Dim_tag>                                    Geom_traits;
typedef CGAL::Triangulation_vertex<Geom_traits,Tile_index>        Vb;
typedef CGAL::Triangulation_data_structure<Dim_tag,Vb>            TDS;
typedef CGAL::Delaunay_triangulation<Geom_traits, TDS>            Triangulation;
typedef CGAL::DDT::Vertex_data_property_map<Triangulation>        TileIndexProperty;
int main(int argc, char **argv) {
    return DDT_demo<
            Triangulation,
            TileIndexProperty,
            CGAL::DDT::Grid_partitioner<Triangulation, TileIndexProperty>,
            CGAL::DDT::Sequential_scheduler,
            CGAL::DDT::File_serializer
            >(argc, argv);
}
