#include <CGAL/Epick_d.h>
#include <CGAL/Delaunay_triangulation.h>
#include <CGAL/DDT/traits/Triangulation_traits_d.h>
#include <CGAL/DDT/partitioner/Grid_partitioner.h>
#include <CGAL/DDT/scheduler/Multithread_scheduler.h>
#include <CGAL/DDT/serializer/File_serializer.h>

#include "DDT_demo.h"

typedef int Tile_index;
typedef CGAL::Dimension_tag<3>                                    Dim_tag;
typedef CGAL::Epick_d<Dim_tag>                                    Geom_traits;
typedef CGAL::Triangulation_vertex<Geom_traits,Tile_index>        Vb;
typedef CGAL::Triangulation_data_structure<Dim_tag,Vb>            TDS;
typedef CGAL::Delaunay_triangulation<Geom_traits, TDS>            Delaunay_triangulation;
typedef CGAL::DDT::Triangulation_traits_d<Delaunay_triangulation> Traits;
int main(int argc, char **argv) {
    return DDT_demo<
            CGAL::DDT::Triangulation_traits_d<Delaunay_triangulation>,
            CGAL::DDT::Grid_partitioner,
            CGAL::DDT::Multithread_scheduler,
            CGAL::DDT::No_tile_points,
            CGAL::DDT::File_serializer
            >(argc, argv);
}
