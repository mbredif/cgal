#if __cplusplus >= 201703L

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/DDT/triangulation/Delaunay_triangulation_2.h>
#include <CGAL/DDT/property_map/Vertex_info_property_map.h>
#include <CGAL/DDT/partitioner/Grid_partitioner.h>
#include <CGAL/DDT/scheduler/STD_scheduler.h>
#include <CGAL/DDT/serializer/File_serializer.h>

#include "DDT_demo.h"

typedef int Tile_index;
typedef CGAL::Exact_predicates_inexact_constructions_kernel                  Geom_traits;
typedef CGAL::Triangulation_vertex_base_with_info_2<Tile_index, Geom_traits> Vb;
typedef CGAL::Triangulation_data_structure_2<Vb>                             TDS;
typedef CGAL::Delaunay_triangulation_2<Geom_traits, TDS>                     Triangulation;

int main(int argc, char **argv) {
    return DDT_demo<
            Triangulation,
            CGAL::DDT::Vertex_info_property_map<Triangulation>,
            CGAL::DDT::Grid_partitioner<Tile_index, Triangulation>,
            CGAL::DDT::STD_scheduler_par,
            CGAL::DDT::File_serializer
            >(argc, argv);
}

#else

int main(int argc, char **argv) {
    std::cout << "CGAL::DDT::STD_scheduler_par support requires C++17" << std::endl;
    return 0;
}

#endif
