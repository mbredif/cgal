#ifndef DDT_TEST_HPP
#define DDT_TEST_HPP

#include <vector>
#include <boost/filesystem.hpp>
#include <CGAL/DDT/serializer/VRT_file_serializer.h>
#include <CGAL/DDT/IO/write_ply.h>
#include <CGAL/DDT/serializer/File_serializer.h>
#include <CGAL/DDT/partitioner/Grid_partitioner.h>
#include <CGAL/DDT/scheduler/Sequential_scheduler.h>
#include <CGAL/DDT/property_map/Partitioner_property_map.h>
#include <CGAL/Distributed_triangulation.h>
#include <CGAL/DDT/point_set/Random_points_in_bbox.h>

template <typename T>
bool is_euler_valid(const T& tri)
{
    std::cout << "== Euler ==" << std::endl;

    int nv = tri.number_of_finite_vertices();
    int nf = tri.number_of_finite_facets() / 2;
    int nc = tri.number_of_finite_cells();
    int finite_euler = nv - nf + nc;
    std::cout <<nv<<"-"<<nf<<"+"<<nc<<"="<<finite_euler <<  " (euler characteristic of finite elements should be 1)" << std::endl;

    nv = tri.number_of_vertices();
    nf = tri.number_of_facets() / 2;
    nc = tri.number_of_cells();
    int euler = nv - nf + nc;
    std::cout <<nv<<"-"<<nf<<"+"<<nc<<"="<<euler << " (euler characteristic of both finite and infinite elements should be 2)"  << std::endl;

    return  (finite_euler == 1 && euler == 2);
}

template <typename Triangulation,
    typename Scheduler,
    typename Partitioner1,
    typename Partitioner2,
    typename TileIndexProperty1,
    typename TileIndexProperty2>
int test_traits(Scheduler& scheduler,
    const Partitioner1& partitioner1, const Partitioner2& partitioner2,
    const TileIndexProperty1& pmap1, const TileIndexProperty2& pmap2,
    const std::string& testname, int NP, int dim = CGAL::DDT::Triangulation_traits<Triangulation>::D, int NT = -1, double range = 1, bool do_test_io = true)
{
    std::cout << "Test " << testname << std::endl;
    int result = 0;
    int seed = 0;

    typedef CGAL::Distributed_triangulation<Triangulation, TileIndexProperty1> Distributed_triangulation1;
    typedef CGAL::Distributed_triangulation<Triangulation, TileIndexProperty2> Distributed_triangulation2;
    typedef typename Partitioner1::Tile_index Tile_index;
    typedef typename CGAL::DDT::Triangulation_traits<Triangulation>::Point Point;
    typedef CGAL::DDT::Uniform_point_in_bbox<Point> Random_point_generator;
    typedef CGAL::DDT::Random_point_set<Random_point_generator> Point_set;
    typedef CGAL::Distributed_point_set<Point_set, CGAL::DDT::Partitioner_property_map<Point_set, Partitioner1>>  Distributed_point_set;

    std::cout << "== Delaunay ==" << std::endl;
    Point_set ps(NP, partitioner1.bbox(), seed);
    Distributed_point_set points(ps, partitioner1);
    Distributed_triangulation1 tri1(dim, pmap1);
    tri1.insert(points, scheduler);
    if(!tri1.is_valid())
    {
        std::cerr << "tri is not valid" << std::endl;
        return 1;
    }

    boost::filesystem::create_directories(testname);
    if (dim <= 3)
    {
        std::cout << "== write_ply == " << testname << "/out.ply" << std::endl;
        CGAL::DDT::write_ply(tri1, testname + "/out.ply");
    }
    if (dim == 2)
    {
        std::cout << "== write_vrt == " << testname << "/tri1_*.vrt" << std::endl;
        result += !tri1.write(CGAL::DDT::VRT_serializer(testname + "/tri1"), scheduler);
        if(!is_euler_valid(tri1))
            return 1;
    }

    if(do_test_io)
    {
        std::cout << "== test io == " << std::endl;
        boost::filesystem::create_directories(testname + "/cgal");
        boost::filesystem::create_directories(testname + "/cgal2");
        std::cout << "write..." << std::endl;
        tri1.write(CGAL::DDT::File_serializer(testname + "/cgal"), scheduler);

        Distributed_triangulation1 tri2(dim, pmap1);
        std::cout << "read..." << std::endl;
        tri2.read(CGAL::DDT::File_serializer(testname + "/cgal"), scheduler);
        std::cout << "write again..." << std::endl;
        tri2.write(CGAL::DDT::File_serializer(testname + "/cgal2"), scheduler);

        if (dim == 2)
        {
            result += !tri1.write(CGAL::DDT::VRT_serializer(testname + "/tri1"), scheduler);
            result += !tri2.write(CGAL::DDT::VRT_serializer(testname + "/tri2"), scheduler);
            if(!is_euler_valid(tri2))
                result += 1;
        }
    }

    Distributed_triangulation2 tri3(dim, pmap2);
    tri3.partition(partitioner2, tri1, scheduler);

    return result;
}

template <
    typename Triangulation, typename Property,
    typename Scheduler = CGAL::DDT::Sequential_scheduler, int D = CGAL::DDT::Triangulation_traits<Triangulation>::D>
int test_info(const std::string& testname, int dim = D, int ND = 2, int NP = 50, int NT = -1, double range = 1, bool do_test_io = true, Scheduler sch = {})
{
    typedef typename CGAL::DDT::Triangulation_traits<Triangulation>::Point  Point;
    typedef typename Property::value_type                                   Tile_index;
    typedef CGAL::DDT::Grid_partitioner<Tile_index, Point>                  Partitioner;
    typedef CGAL::DDT::Kernel_traits<Point>                                 Traits;
    typedef typename Traits::Bbox                                           Bbox;

    std::vector<double> coord0(dim, -1);
    std::vector<double> coord1(dim,  1);
    Point p0 = Traits::point(coord0.begin(), coord0.end());
    Point p1 = Traits::point(coord1.begin(), coord1.end());
    Bbox bbox = CGAL::DDT::make_bbox(p0, p1);
    Partitioner part1(1, bbox, ND  );
    Partitioner part2(1, bbox, ND+1);

    Property pmap;
    return test_traits<Triangulation>(sch, part1, part2, pmap, pmap, testname, NP, dim, NT, range, do_test_io);
}

template <
    typename Triangulation, typename TileIndex,
    typename Scheduler = CGAL::DDT::Sequential_scheduler, int D = CGAL::DDT::Triangulation_traits<Triangulation>::D>
int test_part(const std::string& testname, int dim = D, int ND = 2, int NP = 50, int NT = -1, double range = 1, bool do_test_io = true, Scheduler sch = {})
{
    typedef typename CGAL::DDT::Triangulation_traits<Triangulation>::Point  Point;
    typedef CGAL::DDT::Grid_partitioner<TileIndex, Point>                   Partitioner;
    typedef CGAL::DDT::Partitioner_property_map<Triangulation, Partitioner> Property;
    typedef CGAL::DDT::Kernel_traits<Point>                                 Traits;
    typedef typename Traits::Bbox                                           Bbox;

    std::vector<double> coord0(dim, -1);
    std::vector<double> coord1(dim,  1);
    Point p0 = Traits::point(coord0.begin(), coord0.end());
    Point p1 = Traits::point(coord1.begin(), coord1.end());
    Bbox bbox = CGAL::DDT::make_bbox(p0, p1);
    Partitioner part1(1, bbox, ND  );
    Partitioner part2(1, bbox, ND+1);

    Property pmap1(part1);
    Property pmap2(part2);

    return test_traits<Triangulation>(sch, part1, part2, pmap1, pmap2, testname, NP, dim, NT, range, do_test_io);
}

#endif // DDT_TEST_HPP
