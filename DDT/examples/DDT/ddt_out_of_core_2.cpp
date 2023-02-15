#include <CGAL/DDT/traits/Triangulation_traits_2.h>
#include <CGAL/DDT/partitioner/Grid_partitioner.h>
#include <CGAL/DDT/Tile_container.h>
#include <CGAL/DDT/scheduler/Sequential_scheduler.h>
#include <CGAL/DDT/serializer/File_serializer.h>
#include <CGAL/Distributed_Delaunay_triangulation.h>
#include <CGAL/DDT/IO/write_ply.h>
#include <CGAL/DDT/IO/write_vrt.h>
#include <CGAL/DDT/insert.h>

typedef int Tile_index;
typedef unsigned char Vertex_info; // unused user data
typedef CGAL::DDT::Triangulation_traits_2<Tile_index, Vertex_info> Traits;
typedef Traits::Random_points_in_box Random_points;
typedef Traits::Bbox Bbox;
typedef CGAL::DDT::Tile<Traits> Tile;
typedef CGAL::DDT::Sequential_scheduler<Traits> Scheduler;
typedef CGAL::DDT::File_serializer<Tile> Serializer;
typedef CGAL::DDT::Tile_container<Traits, Serializer> Tile_container;
typedef CGAL::Distributed_Delaunay_triangulation<Tile_container> Distributed_Delaunay_triangulation;

int main(int argc, char **argv)
{
    enum { D = Traits::D };

    int number_of_points          = (argc>1) ? atoi(argv[1]) : 1000;
    int number_of_tiles_per_axis  = (argc>2) ? atoi(argv[2]) : 3;
    int max_number_of_tiles       = (argc>3) ? atoi(argv[3]) : 1;
    double range = 1;

    Bbox bbox(D, range);
    CGAL::DDT::Grid_partitioner<Traits> partitioner(bbox, number_of_tiles_per_axis);
    Serializer serializer("tile_");
    Tile_container tiles(D, max_number_of_tiles, serializer);
    Scheduler scheduler;
    Random_points points(D, range);
    CGAL::DDT::insert(tiles, scheduler, points, number_of_points, partitioner);
    Distributed_Delaunay_triangulation tri(tiles);

    CGAL::DDT::write_vrt_verts(tiles, scheduler, "out_v");
    CGAL::DDT::write_vrt_facets(tiles, scheduler, "out_f");
    CGAL::DDT::write_vrt_cells(tiles, scheduler, "out_c");
    CGAL::DDT::write_vrt_bboxes(tiles, "out_b");
    CGAL::DDT::write_vrt_tins(tiles, scheduler, "out_t");

    return 0;
}
