#ifndef DDT_TEST_HPP
#define DDT_TEST_HPP

#include <vector>
#include <boost/filesystem.hpp>
#include <CGAL/DDT/IO/write.h>
#include <CGAL/DDT/IO/read.h>
#include <CGAL/DDT/partitioner/grid_partitioner.h>
#include <CGAL/DDT/scheduler.h>
#include <CGAL/DDT.h>

template <typename T>
int dump_2d_vrt(T & tri,const std::string& testname)
{
    boost::filesystem::create_directories(testname);
    std::cout << "== write_vrt ==" << std::endl;
    ddt::write_vrt_vert(tri, testname+"_vert.vrt");
    ddt::write_vrt_facet(tri, testname+"_facet.vrt");
    ddt::write_vrt_cell(tri, testname+"_cell.vrt");
    ddt::write_vrt_cells(tri, testname);
    ddt::write_vrt_verts(tri, testname);
    ddt::write_vrt_facets(tri, testname);
    return 0;

}

template <typename T>
bool is_euler_valid(T & tri)
{
    std::cout << "== Euler ==" << std::endl;
    int nv = tri.number_of_vertices();
    int nf = tri.number_of_facets();
    int nc = tri.number_of_cells();
    int euler = nv - nf + nc;
    std::cout <<nv<<"-"<<nf<<"+"<<nc<<"="<<euler <<  std::endl;
    // should have been 2, but the infinite vertices is not counted yet
    return  (euler == 1);
}

template <typename T>
int test_traits(const std::string& testname, int ND, int NP, bool do_test_io = false)
{
    std::cout << "Test " << testname << std::endl;
    int result = 0;

    typedef T Traits;
    typedef ddt::Tile<Traits> Tile;
    typedef ddt::Scheduler<Tile> Scheduler;
    typedef ddt::DDT<Traits, Scheduler> DDT;
    typedef ddt::grid_partitioner<Traits> Partitioner;
    typedef typename Traits::Random_points_in_box Random_points;

    std::cout << "== Delaunay ==" << std::endl;
    double range = 1;
    ddt::Bbox<Traits::D> bbox(range);
    Random_points points(Traits::D, range);
    Partitioner partitioner(bbox, ND);
    DDT tri1;
    tri1.send_points(points, NP, partitioner);
    tri1.insert_received_points();
    tri1.send_all_bbox_points();
    tri1.splay_stars();
    tri1.finalize();
    if(!tri1.is_valid())
    {
        std::cerr << "tri is not valid" << std::endl;
        return 1;
    }

    boost::filesystem::create_directories(testname);
    if (Traits::D == 3)
    {
        std::cout << "== write_ply ==" << std::endl;
        ddt::write_ply(tri1, testname+".ply");
    }
    else if (Traits::D == 2)
    {
        result += dump_2d_vrt(tri1,testname + "/tri1");
        if(!is_euler_valid(tri1))
            return 1;
    }

    if(do_test_io)
    {
        std::cout << "== test io == " << std::endl;
        boost::filesystem::create_directories(testname + "/cgal");
        std::cout << "write..." << std::endl;
        ddt::write_cgal(tri1,testname + "/cgal");

        DDT tri2;
        std::cout << "read..." << std::endl;
        ddt::read_cgal(tri2,testname + "/cgal");

        result += dump_2d_vrt(tri2,testname + "/tri2");
        if (Traits::D == 2)
        {
            result += dump_2d_vrt(tri1,testname + "/tri1");
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
            if (cell->is_infinite()) continue;
            std::set<typename DDT::Cell_const_iterator> ring;
            tri1.get_ring(cell, 1, ring);
            finite_cell = cell;
            break;
        }


        for(int deg = 1; deg < 30; deg += 5)
        {
            std::set<typename DDT::Cell_const_iterator> ring;
            tri1.get_ring(finite_cell, deg, ring);
            boost::filesystem::create_directories(testname + "/ring/");
            write_vrt_cell_range(ring.begin(), ring.end(), testname + "/ring/" +std::to_string(deg)+".vrt");
        }
    }

    std::cout << "== Tile.get_* ==" << std::endl;
    {
        Tile t(0);
        std::vector<typename Tile::Vertex_const_handle> points;
        std::vector<typename Tile::Vertex_const_handle_and_id> points_and_ids;
        t.get_bbox_points(points);
        t.get_local_neighbors(points_and_ids);
    }
    return result;
}

#endif // DDT_TEST_HPP

