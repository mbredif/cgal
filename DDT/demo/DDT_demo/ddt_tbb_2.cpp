#include <CGAL/DDT/traits/cgal_traits_2.h>
#include <CGAL/DDT/partitioner/Grid_partitioner.h>
#include <CGAL/DDT/scheduler/TBB_scheduler.h>
#include <CGAL/DDT/serializer/File_serializer.h>

#include "DDT_demo.h"

typedef int Id;
int main(int argc, char **argv) {
    return DDT_demo<
            CGAL::DDT::Cgal_traits_2<Id>,
            CGAL::DDT::Grid_partitioner,
            CGAL::DDT::TBB_scheduler,
            CGAL::DDT::File_serializer
            >(argc, argv);
}
