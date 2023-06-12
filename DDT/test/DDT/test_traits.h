#ifndef DDT_TEST_HPP
#define DDT_TEST_HPP

#include <vector>
#include <boost/filesystem.hpp>
#include <CGAL/DDT/IO/write_vrt.h>
#include <CGAL/DDT/IO/write_ply.h>
#include <CGAL/DDT/IO/write_cgal.h>
#include <CGAL/DDT/IO/read_cgal.h>
#include <CGAL/DDT/partitioner/Grid_partitioner.h>
#include <CGAL/DDT/scheduler/Sequential_scheduler.h>
#include <CGAL/DDT/insert.h>
#include <CGAL/DDT/Tile_container.h>
#include <CGAL/Distributed_triangulation.h>


template <typename T, typename Scheduler>
int dump_2d_vrt(T& tri, Scheduler& scheduler, const std::string& vrt)
{
    std::cout << "== write_vrt == " << vrt << "_*.vrt" << std::endl;
    CGAL::DDT::write_vrt_verts(tri.tiles, scheduler, vrt+"_v");
    CGAL::DDT::write_vrt_facets(tri.tiles, scheduler, vrt+"_f");
    CGAL::DDT::write_vrt_cells(tri.tiles, scheduler, vrt+"_c");
    CGAL::DDT::write_vrt_bboxes(tri.tiles, vrt+"_b");
    CGAL::DDT::write_vrt_tins(tri.tiles, scheduler, vrt+"_t");
    return 0;

}

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

template <typename Triangulation, typename TileIndexProperty, typename Partitioner = CGAL::DDT::Grid_partitioner<Triangulation, TileIndexProperty>>
int test_traits(const Partitioner& partitioner, const std::string& testname, int NP, int dim = CGAL::DDT::Triangulation_traits<Triangulation>::D, int NT = -1, double range = 1, bool do_test_io = true)
{
    std::cout << "Test " << testname << std::endl;
    int result = 0;

    typedef CGAL::DDT::Sequential_scheduler Scheduler;
    typedef CGAL::DDT::Tile_container<Triangulation, TileIndexProperty> TileContainer;
    typedef CGAL::Distributed_triangulation<TileContainer> Distributed_triangulation;
    typedef typename CGAL::DDT::Triangulation_traits<Triangulation>::Random_points_in_box Random_points;

    std::cout << "== Delaunay ==" << std::endl;
    Random_points points(dim, range);
    Scheduler scheduler;
    Distributed_triangulation tri1(dim, NT);
    tri1.insert(scheduler, points, NP, partitioner);
    if(!tri1.is_valid())
    {
        std::cerr << "tri is not valid" << std::endl;
        return 1;
    }

    boost::filesystem::create_directories(testname);
    if (dim <= 3)
    {
        std::cout << "== write_ply ==" << std::endl;
        CGAL::DDT::write_ply(tri1.tiles, testname + "/out.ply");
    }
    if (dim == 2)
    {
        result += dump_2d_vrt(tri1, scheduler, testname + "/tri1");
        if(!is_euler_valid(tri1))
            return 1;
    }

    if(do_test_io)
    {
        std::cout << "== test io == " << std::endl;
        boost::filesystem::create_directories(testname + "/cgal");
        boost::filesystem::create_directories(testname + "/cgal2");
        std::cout << "write..." << std::endl;
        CGAL::DDT::write_cgal(tri1.tiles, testname + "/cgal");

        Distributed_triangulation tri2(dim);
        std::cout << "read..." << std::endl;
        CGAL::DDT::read_cgal(tri2.tiles, testname + "/cgal");
        std::cout << "write again..." << std::endl;
        CGAL::DDT::write_cgal(tri2.tiles, testname + "/cgal2");

        result += dump_2d_vrt(tri2, scheduler, testname + "/tri2");
        if (dim == 2)
        {
            result += dump_2d_vrt(tri1, scheduler, testname + "/tri1");
            if(!is_euler_valid(tri2))
                result += 1;
        }
    }
    return result;
}

template <typename Triangulation, typename TileIndexProperty>
int test_traits_grid(const std::string& testname, int ND, int NP, int dim = CGAL::DDT::Triangulation_traits<Triangulation>::D, int NT = -1, double range = 1, bool do_test_io = true)
{
    typedef CGAL::DDT::Triangulation_traits<Triangulation> Traits;
    typedef typename Traits::Bbox Bbox;
    Bbox bbox = Traits::bbox(dim, range);
    CGAL::DDT::Grid_partitioner<Triangulation, TileIndexProperty> partitioner(bbox, ND);
    return test_traits<Triangulation, TileIndexProperty>(partitioner, testname, NP, dim, NT, range, do_test_io);
}

#endif // DDT_TEST_HPP
