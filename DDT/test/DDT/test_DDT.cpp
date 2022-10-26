typedef int Id;
typedef int Flag;

#include <assert.h>
#include <vector>
#include <boost/filesystem.hpp>

#include <CGAL/DDT/traits/cgal_traits_d.h>
typedef ddt::Cgal_traits<2,Id,Flag> Traits;

#include <CGAL/DDT.h>
#include <CGAL/DDT/scheduler.h>
#include <CGAL/DDT/partitioner/grid_partitioner.h>
#include <CGAL/DDT/IO/write_vrt.h>
#include <CGAL/DDT/distributed_Delaunay_triangulation.h>

typedef ddt::Tile<Traits> Tile;
typedef ddt::Scheduler<Tile> Scheduler;
typedef ddt::grid_partitioner<Traits> Partitioner;
#include <CGAL/DDT/serializer/file_serializer.h>
typedef ddt::File_Serializer<Id,Tile> Serializer;

typedef ddt::DDT<Traits, Serializer> DDT;
typedef Traits::Point Point;

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
    ddt::Bbox<Traits::D, double> bbox(range);

    Partitioner partitioner(bbox, ND, ND+Traits::D);
    Serializer serializer;
    DDT tri(serializer);
    Scheduler scheduler;
    CGAL::ddt::insert(tri, scheduler, points.begin(), points.size(), partitioner);
    assert(tri.is_valid());

    for(auto vertex = tri.vertices_begin(); vertex != tri.vertices_end(); ++vertex)
    {
        assert(vertex.is_valid());
        assert(tri.is_valid(vertex));
        assert(tri.is_main(vertex));
        assert(!tri.is_foreign(vertex));
        assert(!tri.is_infinite(vertex));
        assert(tri.is_local(vertex));
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
        assert(tri.locate(tri.facet(cell, tri.index_of_covertex(facet)), tri.tile_id(facet)) == facet);
        assert(tri.locate(tri.neighbor(cell2, tri.mirror_index(facet)), tri.tile_id(cell)) == cell);
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
        for(int d = 0; d < 3; ++d)
        {
            auto vd = tri.vertex(cell, d);
            auto fd = tri.facet(cell, d);
            auto cd = tri.neighbor(cell, d);
            assert(tri.is_infinite(vd) ? vd == tri.infinite_vertex() : vd == tri.main(vd));
            assert(fd == tri.main(fd));
            assert(cd == tri.main(cd));
            assert(tri.cell(tri.mirror_facet(tri.mirror_facet(fd))) == cell);
            assert(cell != cd);
            assert(cell != tri.main(cd));
            assert(cd != tri.cells_end());
            assert(tri.main(cd) != tri.cells_end());
            assert(tri.is_main(tri.main(cd)));
            assert(tri.neighbor(cd, tri.mirror_index(cell, d)) == cell);
        }
        std::set<typename DDT::Cell_const_iterator> ring;
        tri.get_ring(cell, 1, ring);
        // write_vrt_cell_range(tri, ring.begin(), ring.end(), outdir.string() + std::to_string(cid) + "_ring.vrt");
    }


    boost::filesystem::path outdir("./test_DDT_out/");
    boost::filesystem::create_directories(outdir);
    ddt::write_vrt_vert(tri, outdir.string() + "DDT_vert.vrt");
    ddt::write_vrt_facet(tri, outdir.string() + "DDT_facet.vrt");
    ddt::write_vrt_cell(tri, outdir.string() + "DDT_cell.vrt");
    ddt::write_vrt_verts(tri, outdir.string());
    ddt::write_vrt_facets(tri, outdir.string());
    ddt::write_vrt_cells(tri, outdir.string());


    return 0;
}
