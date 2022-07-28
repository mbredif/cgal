typedef unsigned char Id;
typedef unsigned char Flag;

#include <assert.h>
#include <vector>
#include <CGAL/DDT/traits/cgal_traits_d.h>
typedef ddt::Cgal_traits<2,Id,Flag> Traits;

#include <CGAL/DDT/DDT.h>
#include <CGAL/DDT/scheduler/scheduler.h>
#include <CGAL/DDT/partitioner/grid_partitioner.h>
#include <CGAL/DDT/IO/write_vrt.h>

typedef ddt::Tile<Traits> Tile;
typedef ddt::Scheduler<Tile> Scheduler;
typedef ddt::grid_partitioner<Traits> Partitioner;
typedef ddt::DDT<Traits, Scheduler> DDT;
typedef Traits::Point Point;

std::ostream& operator<<(std::ostream& out, DDT::Vertex_const_iterator v)
{
    if (v->is_infinite()) return out << "(inf)";
    return out << v->point();
}

std::ostream& operator<<(std::ostream& out, DDT::Cell_const_iterator c)
{
    out << "C" << int(c->tile()->id()) << ": ";
    for(int d = 0; d <= c->tile()->current_dimension(); ++d)
        out << c->vertex(d) << "; ";
    return out;
}

std::ostream& operator<<(std::ostream& out, DDT::Facet_const_iterator f)
{
    return out << "F" << f->index_of_covertex() << f->full_cell();
}

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
    ddt::Bbox<Traits::D> bbox(range);

    Partitioner partitioner(bbox, ND, ND+Traits::D);
    DDT tri;
    tri.send_points(points.begin(), points.size(), partitioner);
    tri.insert_received_points();
    tri.send_all_bbox_points();
    tri.splay_stars();
    tri.finalize();
    assert(tri.is_valid());

    ddt::write_vrt_vert(tri, "./DDT_vert.vrt");
    ddt::write_vrt_facet(tri, "./DDT_facet.vrt");
    ddt::write_vrt_cell(tri, "./DDT_cell.vrt");
    ddt::write_vrt_verts(tri, "./DDT/");
    ddt::write_vrt_facets(tri, "./DDT/");
    ddt::write_vrt_cells(tri, "./DDT/");


    for(auto vertex = tri.vertices_begin(); vertex != tri.vertices_end(); ++vertex)
    {
        assert(vertex->is_valid());
        assert(vertex->is_main());
        assert(!vertex->is_foreign());
        assert(!vertex->is_infinite());
        assert(vertex->is_local());
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
        assert(facet->neighbor()->full_cell()->main() != facet->full_cell()->main());
        assert(facet->neighbor()->main() != facet);
        assert(facet->neighbor()->main()->index_of_covertex() == facet->mirror_index());
        assert(facet->neighbor()->mirror_index() == facet->index_of_covertex());
        assert(facet->full_cell()->facet(facet->index_of_covertex()) == facet);
        assert(facet->neighbor()->neighbor()->full_cell()->main() == facet->full_cell()->main());
    }

    int cid = 0;
    for(auto cell = tri.cells_begin(); cell != tri.cells_end(); ++cell, ++cid)
    {
        assert(cell->is_valid());
        assert(cell->is_main());
        assert(!cell->is_foreign());
        for(int d = 0; d < 3; ++d)
        {
            auto celld = cell->neighbor(d);
            assert(cell->facet(d)->neighbor()->neighbor()->full_cell()->main() == cell);
            assert(cell != celld);
            assert(cell != celld->main());
            assert(celld->main() != tri.cells_end());
            assert(celld->main().is_main());
            assert(celld->neighbor(cell->mirror_index(d))->main() == cell);
        }
        std::set<typename DDT::Cell_const_iterator> ring;
        tri.get_ring(cell, 1, ring);
        write_vrt_cell_range(ring.begin(), ring.end(), std::to_string(cid) + "_ring.vrt");
    }

    return 0;
}
