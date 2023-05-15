#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/DDT/traits/Triangulation_traits_2.h>
#include <CGAL/DDT/traits/Vertex_info_property_map.h>

typedef int                                                                  Tile_index;
typedef CGAL::Exact_predicates_inexact_constructions_kernel                  Geom_traits;
typedef CGAL::Triangulation_vertex_base_with_info_2<Tile_index, Geom_traits> Vb;
typedef CGAL::Triangulation_data_structure_2<Vb>                             TDS;
typedef CGAL::Delaunay_triangulation_2<Geom_traits, TDS>                     Triangulation;
typedef CGAL::DDT::Vertex_info_property_map<Triangulation>                   TileIndexProperty;
typedef Triangulation::Point Point;
typedef CGAL::Bbox_2 Bbox;

#include <CGAL/DDT/Tile_container.h>
typedef CGAL::DDT::Tile_container<Triangulation, TileIndexProperty> TileContainer;
typedef TileContainer::Tile Tile;

//#include <CGAL/DDT/scheduler/Sequential_scheduler.h>
//typedef CGAL::DDT::Sequential_scheduler Scheduler;
#include <CGAL/DDT/scheduler/Multithread_scheduler.h>
typedef CGAL::DDT::Multithread_scheduler Scheduler;
//#include <CGAL/DDT/scheduler/TBB_scheduler.h>
//typedef CGAL::DDT::TBB_scheduler Scheduler;
//#include <CGAL/DDT/scheduler/MPI_scheduler.h>
//typedef CGAL::DDT::MPI_scheduler Scheduler;

#include <CGAL/DDT/partitioner/Grid_partitioner.h>
typedef CGAL::DDT::Grid_partitioner<Triangulation, TileIndexProperty> Partitioner;


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
    Bbox bbox(-range, -range, range, range);

    TileContainer tiles;
    Partitioner partitioner(bbox, ND, ND+tiles.maximal_dimension());
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
        for(int d = 0; d <= tiles.maximal_dimension(); ++d)
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
