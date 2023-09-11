#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/DDT/Delaunay_triangulation_2.h>
#include <CGAL/DDT/traits/Vertex_info_property_map.h>
#include <CGAL/DDT/serializer/File_points_serializer.h>

typedef int                                                                  Tile_index;
typedef CGAL::Exact_predicates_inexact_constructions_kernel                  Geom_traits;
typedef CGAL::Triangulation_vertex_base_with_info_2<Tile_index, Geom_traits> Vb;
typedef CGAL::Triangulation_data_structure_2<Vb>                             TDS;
typedef CGAL::Delaunay_triangulation_2<Geom_traits, TDS>                     Triangulation;
typedef CGAL::DDT::Vertex_info_property_map<Triangulation>                   TileIndexProperty;
typedef CGAL::DDT::File_points_serializer                                    Serializer;
typedef Triangulation::Point Point;
typedef CGAL::Bbox_2 Bbox;

#include <CGAL/DDT/scheduler/Sequential_scheduler.h>
typedef CGAL::DDT::Sequential_scheduler Scheduler;

#include <CGAL/DDT/partitioner/Grid_partitioner.h>
typedef CGAL::DDT::Grid_partitioner<Tile_index, Triangulation> Partitioner;

#include <CGAL/Distributed_triangulation.h>
typedef CGAL::Distributed_triangulation<Triangulation, TileIndexProperty, Serializer> Distributed_triangulation;
typedef CGAL::Distributed_point_set<Tile_index, Point> Distributed_point_set;

#include <CGAL/DDT/serializer/VRT_file_serializer.h>

#define ddt_assert(x) if (!(x)) { std::cerr << "Assertion failed [" << __LINE__ << "] : " << #x << std::endl; ++errors; }
#define ddt_assert_eq(x, y) if ((x)!=(y)) { ddt_assert((x)==(y)); std::cerr << #x << " = " << x << std::endl << #y << " = " << y << std::endl; }
#define ddt_assert_neq(x, y) if ((x)==(y)) { ddt_assert((x)!=(y)); std::cerr << #x << " = " << x << std::endl << #y << " = " << y << std::endl; }

std::ostream& operator<<(std::ostream& out, const Point& p)
{
    return out << p[0] << "," << p[1];
}

typedef typename Distributed_triangulation::Vertex_iterator Vertex_iterator;

template<typename Tile_triangulation>
std::ostream& write_point(std::ostream& out, const Tile_triangulation& tri, typename Tile_triangulation::Vertex_index v) {
    if (tri.vertex_is_infinite(v)) return out << "inf";
    return out << tri.vertex_id(v) << "|" << tri.point(v);
}

template<typename T>
std::ostream& operator<<(std::ostream& out, const CGAL::DDT::Vertex_iterator<T>& v)
{
    auto& tri = v.triangulation();
    out << "V" << v.id() << "(";
    return write_point(out, tri, *v) << ")";
}

template<typename T>
std::ostream& operator<<(std::ostream& out, const CGAL::DDT::Cell_iterator<T>& c)
{
    auto& tri = c.triangulation();
    out << "C" << c.id() << "|" << tri.cell_id(*c) << "(";
    write_point(out, tri, tri.vertex(*c,0)) << " ; ";
    write_point(out, tri, tri.vertex(*c,1)) << " ; ";
    write_point(out, tri, tri.vertex(*c,2)) << ")";
    return out;
}

template<typename T>
std::ostream& operator<<(std::ostream& out, const CGAL::DDT::Facet_iterator<T>& f)
{
    auto& tri = f.triangulation();
    auto c = tri.cell(*f);
    out << "F" << f.id() << "|" << tri.facet_id(*f) << "(" << tri.index_of_covertex(*f) << " : ";
    write_point(out, tri, tri.vertex(c,0)) << " ; ";
    write_point(out, tri, tri.vertex(c,1)) << " ; ";
    write_point(out, tri, tri.vertex(c,2)) << ")";
    return out;
}

int test_DDT(Distributed_triangulation& tri, unsigned int N = 16)
{
    int errors = 0;
    ddt_assert(tri.is_valid());
    ddt_assert_eq(tri.number_of_finite_vertices(), N);

    unsigned int i = 0;
    for(auto vertex = tri.vertices_begin(); vertex != tri.vertices_end(); ++vertex, ++i)
    {
        ddt_assert(vertex.is_valid());
        ddt_assert(tri.is_valid(vertex));
        ddt_assert(tri.is_main(vertex));
        ddt_assert(!tri.is_foreign(vertex));
        ddt_assert(!tri.is_infinite(vertex));
        ddt_assert(tri.is_local(vertex));
        ddt_assert(tri.cell(vertex).is_valid());
        ddt_assert(tri.has_vertex(tri.cell(vertex), vertex));
        for(unsigned int j = i; j < tri.number_of_finite_vertices(); ++j) {
            auto v = vertex, w = vertex;
            v += (j-i);
            for(unsigned int k = i; k < j; ++k) ++w;
            ddt_assert(v == tri.vertices_end() || tri.is_main(v));
            ddt_assert(w == tri.vertices_end() || tri.is_main(w));
            ddt_assert_eq(v,w);
        }
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
        ddt_assert_eq(tri.mirror_facet(facet2), facet);
        ddt_assert_neq(cell2, cell);
        ddt_assert_neq(facet2, facet);
        ddt_assert_eq(tri.index_of_covertex(facet2), tri.mirror_index(facet));
        ddt_assert_eq(tri.mirror_index(facet2), tri.index_of_covertex(facet));
        ddt_assert_eq(tri.relocate(tri.facet(cell, tri.index_of_covertex(facet)), tri.tile_id(facet)), facet);
        ddt_assert_eq(tri.relocate(tri.neighbor(cell2, tri.mirror_index(facet)), tri.tile_id(cell)), cell);
        ddt_assert_eq(tri.covertex(facet), tri.mirror_vertex(facet2));
        ddt_assert_eq(tri.covertex(facet2), tri.mirror_vertex(facet));
    }

    int cid = 0;
    for(auto cell = tri.cells_begin(); cell != tri.cells_end(); ++cell, ++cid)
    {
        ddt_assert(cell.is_valid());
        ddt_assert(tri.is_valid(cell));
        ddt_assert(tri.is_main(cell));
        ddt_assert(!tri.is_foreign(cell));
        for(int d = 0; d <= tri.maximal_dimension(); ++d)
        {
            auto vd = tri.vertex(cell, d);
            auto fd = tri.facet(cell, d);
            auto cd = tri.neighbor(cell, d);
            ddt_assert_eq(vd, (tri.is_infinite(vd) ? tri.infinite_vertex() : tri.main(vd)));
            ddt_assert_eq(cd, tri.main(cd));
            ddt_assert_eq(tri.cell(tri.mirror_facet(tri.mirror_facet(fd))), cell);
            ddt_assert_neq(cell, cd);
            ddt_assert_neq(cell, tri.main(cd));
            ddt_assert_neq(cd, tri.cells_end());
            ddt_assert_neq(tri.main(cd), tri.cells_end());
            ddt_assert(tri.is_main(tri.main(cd)));
            ddt_assert_eq(tri.neighbor(cd, tri.mirror_index(cell, d)), cell);
            ddt_assert_eq(tri.mirror_vertex(tri.mirror_facet(fd)), vd);
            ddt_assert_eq(tri.covertex(tri.mirror_facet(fd)), tri.vertex(cd, tri.mirror_index(fd)));
            ddt_assert_eq(tri.covertex(fd), tri.vertex(cell, tri.mirror_index(tri.mirror_facet(fd))));
            ddt_assert(tri.has_vertex(tri.cell(vd), vd));
        }
    }

    return errors;
}

int main(int argc, char **argv)
{
    int max_number_of_tiles_in_mem = (argc>1) ? atoi(argv[1]) : 0;
    std::cout << argv[0] << " [max_number_of_tiles_in_mem=" << max_number_of_tiles_in_mem << "]" << std::endl;

    int errors = 0;
    int N = 2;
    int ND[] = {2, 2};

    std::vector<Point> points;

    points.emplace_back(-2, -2);
    points.emplace_back(-2, -1);
    points.emplace_back(-1, -2);
    points.emplace_back(-1, -1);

    points.emplace_back(-2, 2);
    points.emplace_back(-2, 1);
    points.emplace_back(-1, 2);
    points.emplace_back(-1, 1);

    points.emplace_back(2, 2);
    points.emplace_back(2, 1);
    points.emplace_back(1, 2);
    points.emplace_back(1, 1);

    points.emplace_back(2, -2);
    points.emplace_back(2, -1);
    points.emplace_back(1, -2);
    points.emplace_back(1, -1);

    double range = 3;
    Bbox bbox(-range, -range, range, range);
    Partitioner partitioner(1, bbox, ND, ND+N);
    Serializer serializer("tmp_");
    Scheduler scheduler;

    Distributed_triangulation tri1(N, {}, max_number_of_tiles_in_mem, serializer);
    Distributed_point_set pointset(points, partitioner);
    tri1.insert(pointset, scheduler);
    errors += test_DDT(tri1);

    Distributed_triangulation tri2(N, {}, max_number_of_tiles_in_mem, serializer);
    for(auto& p : points)
    {
        auto inserted = tri2.insert(p, partitioner(p), scheduler);
        ddt_assert(inserted.second);
    }
    errors += test_DDT(tri2);

    for(auto& p : points)
    {
        auto inserted = tri1.insert(p, partitioner(p), scheduler);
        ddt_assert(!inserted.second);
    }
    errors += test_DDT(tri1);

    tri1.write(CGAL::DDT::VRT_serializer("test_DDT_batch_out"), scheduler);
    tri2.write(CGAL::DDT::VRT_serializer("test_DDT_incr_out"), scheduler);

    int ND2[] = {16 , 16};
    Partitioner partitioner2(1, bbox, ND2, ND2+N);
    Serializer serializer2("tmp2_");
    Distributed_triangulation tri3(N, {}, max_number_of_tiles_in_mem, serializer2);
    tri3.partition(partitioner2, tri1, scheduler);
    tri3.write(CGAL::DDT::VRT_serializer("test_DDT_retile_out"), scheduler);
    errors += test_DDT(tri3);

    if (errors)
        std::cerr << errors << " errors occured !" << std::endl;
    else
        std::cout << "No errors !" << std::endl;
    return -errors;
}
