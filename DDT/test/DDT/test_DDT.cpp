typedef int Id;
typedef int Flag;

#include <assert.h>
#include <vector>
#include <boost/filesystem.hpp>

#include <CGAL/DDT/traits/cgal_traits_d.h>
typedef CGAL::DDT::Cgal_traits<2,Id,Flag> Traits;
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

int main(int, char **)
{
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
    assert(tri.is_valid());

    for(auto vertex = tri.vertices_begin(); vertex != tri.vertices_end(); ++vertex)
    {
        assert(vertex.is_valid());
        assert(tri.is_valid(vertex));
        assert(tri.is_main(vertex));
        assert(!tri.is_foreign(vertex));
        assert(!tri.is_infinite(vertex));
        assert(tri.is_local(vertex));
        assert(tri.cell(vertex).is_valid());
        assert(tri.has_vertex(tri.cell(vertex), vertex));
    }

    for(auto facet = tri.facets_begin(); facet != tri.facets_end(); ++facet)
    {
        auto facet2 = tri.mirror_facet(facet);
        auto cell   = tri.cell(facet);
        auto cell2  = tri.cell(facet2);
        assert(facet.is_valid());
        assert(tri.is_main(facet));
        assert(tri.is_valid(facet));
        assert(!tri.is_foreign(cell));
        assert(tri.mirror_facet(facet2) == facet);
        assert(cell2 != cell);
        assert(facet2 != facet);
        assert(tri.index_of_covertex(facet2) == tri.mirror_index(facet));
        assert(tri.mirror_index(facet2) == tri.index_of_covertex(facet));
        assert(tri.relocate(tri.facet(cell, tri.index_of_covertex(facet)), tri.tile_id(facet)) == facet);
        assert(tri.relocate(tri.neighbor(cell2, tri.mirror_index(facet)), tri.tile_id(cell)) == cell);
        assert(tri.covertex(facet) == tri.mirror_vertex(facet2));
        assert(tri.covertex(facet2) == tri.mirror_vertex(facet));
    }

    int cid = 0;
    for(auto cell = tri.cells_begin(); cell != tri.cells_end(); ++cell, ++cid)
    {
        assert(cell.is_valid());
        assert(tri.is_valid(cell));
        assert(tri.is_main(cell));
        assert(!tri.is_foreign(cell));
        for(int d = 0; d <= Traits::D; ++d)
        {
            auto vd = tri.vertex(cell, d);
            auto fd = tri.facet(cell, d);
            auto cd = tri.neighbor(cell, d);
            assert(tri.is_infinite(vd) ? vd == tri.infinite_vertex() : vd == tri.main(vd));
            assert(cd == tri.main(cd));
            assert(tri.cell(tri.mirror_facet(tri.mirror_facet(fd))) == cell);
            assert(cell != cd);
            assert(cell != tri.main(cd));
            assert(cd != tri.cells_end());
            assert(tri.main(cd) != tri.cells_end());
            assert(tri.is_main(tri.main(cd)));
            assert(tri.neighbor(cd, tri.mirror_index(cell, d)) == cell);
            assert(tri.mirror_vertex(tri.mirror_facet(fd)) == vd);
            assert(tri.covertex(tri.mirror_facet(fd)) == tri.vertex(cd, tri.mirror_index(fd)));
            assert(tri.covertex(fd) == tri.vertex(cell, tri.mirror_index(tri.mirror_facet(fd))));
            assert(tri.has_vertex(tri.cell(vd), vd));
        }
        std::set<typename Distributed_Delaunay_triangulation::Cell_const_iterator> ring;
        tri.get_ring(cell, 1, ring);
    }

    CGAL::DDT::write_vrt_verts(tiles, scheduler, "test_DDT_out_v");
    CGAL::DDT::write_vrt_facets(tiles, scheduler, "test_DDT_out_f");
    CGAL::DDT::write_vrt_cells(tiles, scheduler, "test_DDT_out_c");


    return 0;
}
