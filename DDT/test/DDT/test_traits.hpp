#ifndef DDT_TEST_HPP
#define DDT_TEST_HPP

#include <vector>
#include <boost/filesystem.hpp>
#include <CGAL/DDT/IO/write.h>
#include <CGAL/DDT/IO/read.h>
#include <CGAL/DDT/partitioner/grid_partitioner.h>
#include <CGAL/DDT/Scheduler.h>
#include <CGAL/DDT/insert.h>
#include <CGAL/DDT/Tile_container.h>
#include <CGAL/Distributed_Delaunay_triangulation.h>

template <typename TileContainer, typename Scheduler>
int dump_2d_vrt(TileContainer& tiles, Scheduler& scheduler, const std::string& vrt)
{
    std::cout << "== write_vrt == " << vrt << "_*.vrt" << std::endl;
    CGAL::DDT::write_vrt_verts(tiles, scheduler, vrt+"_v");
    CGAL::DDT::write_vrt_facets(tiles, scheduler, vrt+"_f");
    CGAL::DDT::write_vrt_cells(tiles, scheduler, vrt+"_c");
    CGAL::DDT::write_vrt_bboxes(tiles, scheduler, vrt+"_b");
    CGAL::DDT::write_vrt_tins(tiles, scheduler, vrt+"_t");
    return 0;

}

template <typename T>
bool is_euler_valid(T & tri)
{
    std::cout << "== Euler ==" << std::endl;

    int nv = tri.number_of_finite_vertices();
    int nf = tri.number_of_finite_facets();
    int nc = tri.number_of_finite_cells();
    int finite_euler = nv - nf + nc;
    std::cout <<nv<<"-"<<nf<<"+"<<nc<<"="<<finite_euler <<  " (euler characteristic of finite elements should be 1)" << std::endl;

    nv = tri.number_of_vertices();
    nf = tri.number_of_facets();
    nc = tri.number_of_cells();
    int euler = nv - nf + nc;
    std::cout <<nv<<"-"<<nf<<"+"<<nc<<"="<<euler << " (euler characteristic of both finite and infinite elements should be 2)"  << std::endl;

    return  (finite_euler == 1 && euler == 2);
}

template <typename T>
int test_traits(const std::string& testname, int ND, int NP, int NT = -1, bool do_test_io = true)
{
    std::cout << "Test " << testname << std::endl;
    int result = 0;

    typedef T Traits;
    typedef CGAL::DDT::Tile<Traits> Tile;
    typedef CGAL::DDT::Scheduler<Tile> Scheduler;
    typedef CGAL::DDT::Tile_container<Traits> TileContainer;
    typedef CGAL::Distributed_Delaunay_triangulation<TileContainer> Distributed_Delaunay_triangulation;
    typedef CGAL::DDT::grid_partitioner<Traits> Partitioner;
    typedef typename Traits::Random_points_in_box Random_points;

    std::cout << "== Delaunay ==" << std::endl;
    double range = 1;
    CGAL::DDT::Bbox<Traits::D, double> bbox(range);
    Random_points points(Traits::D, range);
    Partitioner partitioner(bbox, ND);
    TileContainer tiles1(NT);
    Scheduler scheduler;
    CGAL::DDT::insert(tiles1, scheduler, points, NP, partitioner);
    Distributed_Delaunay_triangulation tri1(tiles1);
    if(!tri1.is_valid())
    {
        std::cerr << "tri is not valid" << std::endl;
        return 1;
    }

    boost::filesystem::create_directories(testname);
    if (Traits::D == 3)
    {
        std::cout << "== write_ply ==" << std::endl;
        CGAL::DDT::write_ply(tiles1, testname + "/out.ply");
    }
    else if (Traits::D == 2)
    {
        result += dump_2d_vrt(tiles1, scheduler, testname + "/tri1");
        if(!is_euler_valid(tri1))
            return 1;
    }

    if(do_test_io)
    {
        std::cout << "== test io == " << std::endl;
        boost::filesystem::create_directories(testname + "/cgal");
        boost::filesystem::create_directories(testname + "/cgal2");
        std::cout << "write..." << std::endl;
        CGAL::DDT::write_cgal(tiles1, testname + "/cgal");

        TileContainer tiles2;
        Distributed_Delaunay_triangulation tri2(tiles2);
        std::cout << "read..." << std::endl;
        CGAL::DDT::read_cgal(tiles2, testname + "/cgal");
        std::cout << "write again..." << std::endl;
        CGAL::DDT::write_cgal(tiles2, testname + "/cgal2");

        result += dump_2d_vrt(tiles2, scheduler, testname + "/tri2");
        if (Traits::D == 2)
        {
            result += dump_2d_vrt(tiles1, scheduler, testname + "/tri1");
            if(!is_euler_valid(tri2))
                result += 1;
        }
    }

    if (Traits::D == 2)
    {
        std::cout << "== get_ring ==" << std::endl;
        auto finite_cell = tri1.cells_begin();
        for(auto cell = tri1.cells_begin(); cell != tri1.cells_end(); ++cell)
        {
            if (tri1.is_infinite(cell)) continue;
            std::set<typename Distributed_Delaunay_triangulation::Cell_const_iterator> ring;
            tri1.get_ring(cell, 1, ring);
            finite_cell = cell;
            break;
        }


        for(int deg = 1; deg < 30; deg += 5)
        {
            std::set<typename Distributed_Delaunay_triangulation::Cell_const_iterator> ring;
            tri1.get_ring(finite_cell, deg, ring);
            boost::filesystem::create_directories(testname + "/ring/");
        }
    }

    std::cout << "== Tile.get_* ==" << std::endl;
    {
        Tile t(0);
        std::vector<typename Tile::Vertex_const_handle> points;
        std::vector<typename Tile::Vertex_const_handle_and_id> points_and_ids;
        t.get_axis_extreme_points(points);
    }
    return result;
}

#endif // DDT_TEST_HPP
