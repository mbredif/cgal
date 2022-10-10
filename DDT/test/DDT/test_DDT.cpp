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

typedef ddt::Tile<Traits> Tile;
typedef ddt::Scheduler<Tile> Scheduler;
typedef ddt::grid_partitioner<Traits> Partitioner;
#include <CGAL/DDT/serializer/file_serializer.h>
typedef ddt::File_Serializer<Id,Tile> Serializer;

typedef ddt::DDT<Traits, Scheduler, Serializer> DDT;
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
    tri.send_points(points.begin(), points.size(), partitioner);
    tri.insert_received_points();
    tri.send_all_bbox_points();
    tri.splay_stars();
    tri.finalize();
    assert(tri.is_valid());

    boost::filesystem::path outdir("./test_DDT_out/");
    boost::filesystem::create_directories(outdir);
    ddt::write_vrt_vert(tri, outdir.string() + "DDT_vert.vrt");
    ddt::write_vrt_facet(tri, outdir.string() + "DDT_facet.vrt");
    ddt::write_vrt_cell(tri, outdir.string() + "DDT_cell.vrt");
    ddt::write_vrt_verts(tri, outdir.string());
    ddt::write_vrt_facets(tri, outdir.string());
    ddt::write_vrt_cells(tri, outdir.string());


    for(auto vertex = tri.vertices_begin(); vertex != tri.vertices_end(); ++vertex)
    {
        assert(vertex.is_valid());
        assert(tri.is_main(vertex));
        assert(!tri.is_foreign(vertex));
        assert(!tri.is_infinite(vertex));
        assert(tri.is_local(vertex));
    }

    for(auto facet = tri.facets_begin(); facet != tri.facets_end(); ++facet)
    {
        assert(facet->is_valid());
        assert(facet->is_main());
        assert(!facet->is_foreign());
        assert(facet->is_local() || facet->is_mixed());
        assert(facet->neighbor()->neighbor()->main() == facet);
        assert(facet->neighbor()->main()->neighbor()->main() == facet);
        assert(facet->neighbor()->full_cell() != facet->full_cell());
        assert(tri.main(facet->neighbor()->full_cell()) != tri.main(facet->full_cell()));
        assert(facet->neighbor()->main() != facet);
        assert(facet->neighbor()->main()->index_of_covertex() == facet->mirror_index());
        assert(facet->neighbor()->mirror_index() == facet->index_of_covertex());
        assert(facet->full_cell().facet(facet->index_of_covertex()) == facet);
        assert(tri.main(facet->neighbor()->neighbor()->full_cell()) == tri.main(facet->full_cell()));
    }

    int cid = 0;
    for(auto cell = tri.cells_begin(); cell != tri.cells_end(); ++cell, ++cid)
    {
        assert(cell.is_valid());
        assert(tri.is_main(cell));
        assert(!tri.is_foreign(cell));
        for(int d = 0; d < 3; ++d)
        {
            auto celld = cell.neighbor(d);
            assert(tri.main(cell.facet(d)->neighbor()->neighbor()->full_cell()) == cell);
            assert(cell != celld);
            assert(cell != tri.main(celld));
            assert(tri.main(celld) != tri.cells_end());
            assert(tri.is_main(tri.main(celld)));
            assert(tri.main(celld.neighbor(cell.mirror_index(d))) == cell);
        }
        std::set<typename DDT::Cell_const_iterator> ring;
        tri.get_ring(cell, 1, ring);
        // write_vrt_cell_range(tri, ring.begin(), ring.end(), outdir.string() + std::to_string(cid) + "_ring.vrt");
    }

    return 0;
}
