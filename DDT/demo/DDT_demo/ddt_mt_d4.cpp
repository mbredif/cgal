#include <CGAL/Epick_d.h>
#include <CGAL/DDT/triangulation/Delaunay_triangulation.h>
#include <CGAL/DDT/property_map/Vertex_data_property_map.h>
#include <CGAL/DDT/partitioner/Grid_partitioner.h>
#include <CGAL/DDT/scheduler/Multithread_scheduler.h>
#include <CGAL/DDT/serializer/File_serializer.h>

#include "DDT_demo.h"

typedef int Tile_index;
typedef CGAL::Dimension_tag<4>                                    Dim_tag;
typedef CGAL::Epick_d<Dim_tag>                                    Geom_traits;
typedef CGAL::Triangulation_vertex<Geom_traits,Tile_index>        Vb;
typedef CGAL::Triangulation_data_structure<Dim_tag,Vb>            TDS;
typedef CGAL::Delaunay_triangulation<Geom_traits, TDS>            Triangulation;

int main(int argc, char **argv) {
    return DDT_demo<
            Triangulation,
            CGAL::DDT::Vertex_data_property_map<Triangulation>,
            CGAL::DDT::Grid_partitioner<Tile_index, Triangulation>,
            CGAL::DDT::Multithread_scheduler,
            CGAL::DDT::File_serializer
            >(argc, argv);
}
