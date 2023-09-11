#ifndef DDT_TEST_HPP
#define DDT_TEST_HPP

#include <vector>
#include <boost/filesystem.hpp>
#include <CGAL/DDT/serializer/VRT_file_serializer.h>
#include <CGAL/DDT/IO/write_ply.h>
#include <CGAL/DDT/serializer/File_serializer.h>
#include <CGAL/DDT/partitioner/Grid_partitioner.h>
#include <CGAL/DDT/scheduler/Sequential_scheduler.h>
#include <CGAL/Distributed_triangulation.h>

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

template <typename Triangulation, typename TileIndexProperty,
    typename Partitioner = CGAL::DDT::Grid_partitioner<Triangulation, TileIndexProperty>,
    typename Scheduler = CGAL::DDT::Sequential_scheduler>
int test_traits(const Partitioner& partitioner1, const Partitioner& partitioner2, const std::string& testname, int NP, int dim = CGAL::DDT::Triangulation_traits<Triangulation>::D, int NT = -1, double range = 1, bool do_test_io = true)
{
    std::cout << "Test " << testname << std::endl;
    int result = 0;

    typedef CGAL::Distributed_triangulation<Triangulation, TileIndexProperty> Distributed_triangulation;
    typedef typename CGAL::DDT::Triangulation_traits<Triangulation>::Point Point;
    typedef typename TileIndexProperty::value_type Tile_index;
    typedef typename CGAL::DDT::Triangulation_traits<Triangulation>::Random_points_in_box Random_points;
    typedef CGAL::Distributed_point_set<Tile_index, Point> Distributed_point_set;

    std::cout << "== Delaunay ==" << std::endl;
    Random_points generator(dim, range);
    Scheduler scheduler;
    Distributed_point_set points(generator, NP, partitioner1);
    Distributed_triangulation tri1(dim);
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

        Distributed_triangulation tri2(dim);
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

    Distributed_triangulation tri3(dim);
    tri3.partition(partitioner2, tri1, scheduler);

    return result;
}

template <typename Triangulation, typename TileIndexProperty>
int test_traits_grid(const std::string& testname, int ND, int NP, int dim = CGAL::DDT::Triangulation_traits<Triangulation>::D, int NT = -1, double range = 1, bool do_test_io = true)
{
    typedef CGAL::DDT::Triangulation_traits<Triangulation> Traits;
    typedef typename Traits::Bbox Bbox;
    Bbox bbox = Traits::bbox(dim, range);
    CGAL::DDT::Grid_partitioner<Triangulation, TileIndexProperty> partitioner1(1, bbox, ND  );
    CGAL::DDT::Grid_partitioner<Triangulation, TileIndexProperty> partitioner2(1, bbox, ND+1);
    return test_traits<Triangulation, TileIndexProperty>(partitioner1, partitioner2, testname, NP, dim, NT, range, do_test_io);
}

#endif // DDT_TEST_HPP
