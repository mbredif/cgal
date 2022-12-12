typedef int Id;
typedef int Info;

#include <CGAL/DDT/traits/cgal_traits_d.h>
typedef CGAL::DDT::Cgal_traits<2,Id,Info> Traits;
typedef Traits::Point Point;
typedef Traits::Bbox Bbox;

#include <CGAL/DDT/Tile_container.h>
typedef CGAL::DDT::Tile<Traits> Tile;
typedef CGAL::DDT::Tile_container<Traits> TileContainer;

//#include <CGAL/DDT/scheduler/Sequential_scheduler.h>
//typedef CGAL::DDT::Sequential_scheduler<Tile> Scheduler;
#include <CGAL/DDT/scheduler/Multithread_scheduler.h>
typedef CGAL::DDT::Multithread_scheduler<Tile> Scheduler;
//#include <CGAL/DDT/scheduler/TBB_scheduler.h>
//typedef CGAL::DDT::TBB_scheduler<Tile> Scheduler;
//#include <CGAL/DDT/scheduler/MPI_scheduler.h>
//typedef CGAL::DDT::MPI_scheduler<Tile> Scheduler;

#include <CGAL/DDT/partitioner/Grid_partitioner.h>
typedef CGAL::DDT::Grid_partitioner<Traits> Partitioner;


#include <CGAL/Distributed_Delaunay_triangulation.h>
typedef CGAL::Distributed_Delaunay_triangulation<TileContainer> Distributed_Delaunay_triangulation;

#include <CGAL/DDT/insert.h>

#include <CGAL/DDT/IO/write_vrt.h>

#define ddt_assert(x) if (!(x)) { std::cerr << "Assertion failed [" << __LINE__ << "] : " << #x << std::endl; ++errors; }

int main(int, char **)
{
    int errors = 0;
    int ND[] = {2, 1};

    std::vector<Point> points;

    points.emplace_back(-2, -2);
    points.emplace_back(-2, -1);
    points.emplace_back(-1, -2);
    points.emplace_back(-1, -1);

    points.emplace_back(2, 2);
    points.emplace_back(2, 1);
    points.emplace_back(1, 2);
    points.emplace_back(1, 1);

    double range = 3;
    Bbox bbox(Traits::D, range);

    Partitioner partitioner(bbox, ND, ND+Traits::D);
    TileContainer tiles(Traits::D);
    Scheduler scheduler;
    CGAL::DDT::insert(tiles, scheduler, points.begin(), points.size(), partitioner);
    Distributed_Delaunay_triangulation tri(tiles);
    ddt_assert(tri.is_valid());

    for(auto vertex = tri.vertices_begin(); vertex != tri.vertices_end(); ++vertex)
    {
        ddt_assert(vertex.is_valid());
        ddt_assert(tri.is_valid(vertex));
        ddt_assert(tri.is_main(vertex));
        ddt_assert(!tri.is_foreign(vertex));
        ddt_assert(!tri.is_infinite(vertex));
        ddt_assert(tri.is_local(vertex));
        ddt_assert(tri.cell(vertex).is_valid());
        ddt_assert(tri.has_vertex(tri.cell(vertex), vertex));
    }

    for(auto facet = tri.facets_begin(); facet != tri.facets_end(); ++facet)
    {
        auto facet2 = tri.mirror_facet(facet);
        auto cell   = tri.cell(facet);
        auto cell2  = tri.cell(facet2);
        ddt_assert(facet.is_valid());
        ddt_assert(tri.is_main(facet));
        ddt_assert(tri.is_valid(facet));
        ddt_assert(!tri.is_foreign(cell));
        ddt_assert(tri.mirror_facet(facet2) == facet);
        ddt_assert(cell2 != cell);
        ddt_assert(facet2 != facet);
        ddt_assert(tri.index_of_covertex(facet2) == tri.mirror_index(facet));
        ddt_assert(tri.mirror_index(facet2) == tri.index_of_covertex(facet));
        ddt_assert(tri.relocate(tri.facet(cell, tri.index_of_covertex(facet)), tri.tile_id(facet)) == facet);
        ddt_assert(tri.relocate(tri.neighbor(cell2, tri.mirror_index(facet)), tri.tile_id(cell)) == cell);
        ddt_assert(tri.covertex(facet) == tri.mirror_vertex(facet2));
        ddt_assert(tri.covertex(facet2) == tri.mirror_vertex(facet));
    }

    int cid = 0;
    for(auto cell = tri.cells_begin(); cell != tri.cells_end(); ++cell, ++cid)
    {
        ddt_assert(cell.is_valid());
        ddt_assert(tri.is_valid(cell));
        ddt_assert(tri.is_main(cell));
        ddt_assert(!tri.is_foreign(cell));
        for(int d = 0; d <= Traits::D; ++d)
        {
            auto vd = tri.vertex(cell, d);
            auto fd = tri.facet(cell, d);
            auto cd = tri.neighbor(cell, d);
            ddt_assert(tri.is_infinite(vd) ? vd == tri.infinite_vertex() : vd == tri.main(vd));
            ddt_assert(cd == tri.main(cd));
            ddt_assert(tri.cell(tri.mirror_facet(tri.mirror_facet(fd))) == cell);
            ddt_assert(cell != cd);
            ddt_assert(cell != tri.main(cd));
            ddt_assert(cd != tri.cells_end());
            ddt_assert(tri.main(cd) != tri.cells_end());
            ddt_assert(tri.is_main(tri.main(cd)));
            ddt_assert(tri.neighbor(cd, tri.mirror_index(cell, d)) == cell);
            ddt_assert(tri.mirror_vertex(tri.mirror_facet(fd)) == vd);
            ddt_assert(tri.covertex(tri.mirror_facet(fd)) == tri.vertex(cd, tri.mirror_index(fd)));
            ddt_assert(tri.covertex(fd) == tri.vertex(cell, tri.mirror_index(tri.mirror_facet(fd))));
            ddt_assert(tri.has_vertex(tri.cell(vd), vd));
        }
        std::set<typename Distributed_Delaunay_triangulation::Cell_const_iterator> ring;
        tri.get_ring(cell, 1, ring);
    }

    CGAL::DDT::write_vrt_verts(tiles, scheduler, "test_DDT_out_v");
    CGAL::DDT::write_vrt_facets(tiles, scheduler, "test_DDT_out_f");
    CGAL::DDT::write_vrt_cells(tiles, scheduler, "test_DDT_out_c");

    return -errors;
}
