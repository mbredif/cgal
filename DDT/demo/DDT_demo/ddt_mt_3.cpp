#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>
#include <CGAL/DDT/traits/Triangulation_traits_3.h>
#include <CGAL/DDT/traits/Vertex_info_property_map.h>
#include <CGAL/DDT/partitioner/Grid_partitioner.h>
#include <CGAL/DDT/scheduler/Multithread_scheduler.h>
#include <CGAL/DDT/serializer/File_serializer.h>

#include "DDT_demo.h"

typedef int Tile_index;
typedef CGAL::Exact_predicates_inexact_constructions_kernel                  Geom_traits;
typedef CGAL::Triangulation_vertex_base_with_info_3<Tile_index, Geom_traits> Vb;
typedef CGAL::Triangulation_data_structure_3<Vb>                             TDS;
typedef CGAL::Delaunay_triangulation_3<Geom_traits, TDS>                     Triangulation;
typedef CGAL::DDT::Vertex_info_property_map<Triangulation>                   TileIndexProperty;
int main(int argc, char **argv) {
    return DDT_demo<
            Triangulation,
            TileIndexProperty,
            CGAL::DDT::Grid_partitioner<Triangulation, TileIndexProperty>,
            CGAL::DDT::Multithread_scheduler,
            CGAL::DDT::File_serializer<Triangulation, TileIndexProperty>
            >(argc, argv);
}
