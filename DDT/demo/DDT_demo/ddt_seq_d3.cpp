#include <CGAL/DDT/traits/Triangulation_traits_d.h>
#include <CGAL/DDT/partitioner/Grid_partitioner.h>
#include <CGAL/DDT/scheduler/Sequential_scheduler.h>
#include <CGAL/DDT/serializer/File_serializer.h>

#include "DDT_demo.h"

typedef int Tile_index;
int main(int argc, char **argv) {
    return DDT_demo<
            CGAL::DDT::Triangulation_traits<3,Tile_index>,
            CGAL::DDT::Grid_partitioner,
            CGAL::DDT::Sequential_scheduler,
            CGAL::DDT::No_tile_points,
            CGAL::DDT::File_serializer
            >(argc, argv);
}
