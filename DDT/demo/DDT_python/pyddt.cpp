#include <CGAL/DDT/Tile.h>
#include <CGAL/DDT/IO/read.h>
#include <CGAL/DDT/IO/write.h>

#include <vector>
#include <boost/python.hpp>
#include <sstream>

typedef int Id;
typedef int Flag;
//

#include <CGAL/DDT/traits/Triangulation_traits_d.h>
typedef CGAL::DDT::Triangulation_traits<2,Id,Flag> Traits;
typedef Traits::Random_points_in_box Random_points;


typedef CGAL::DDT::Tile<Traits> Tile;

class py_vertex_iterator
{
private:
    Tile::Vertex_const_iterator v;
    const Tile& tile;

public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = py_vertex_iterator;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    py_vertex_iterator(Tile::Vertex_const_iterator i, const Tile& t) : v(i), tile(t) {}
    py_vertex_iterator(const py_vertex_iterator& i) : v(i.v), tile(i.tile) {}

    py_vertex_iterator& operator++()
    {
        ++v;
        return *this;
    }

    py_vertex_iterator operator++(int)
    {
        py_vertex_iterator tmp(*this);
        operator++();
        return tmp;
    }

    bool operator==(const py_vertex_iterator& rhs) const { return v==rhs.v; }
    bool operator!=(const py_vertex_iterator& rhs) const { return v!=rhs.v; }

    py_vertex_iterator& operator*() { return *this; }

    boost::python::tuple point() const
    {
        const Tile::Point& p = tile.point(v);
        return boost::python::make_tuple(p[0], p[1], tile.vertex_id(v));
    }
};

class py_cell_iterator
{
private:
    Tile::Cell_const_iterator c;
    const Tile& tile;

public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = py_cell_iterator;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    py_cell_iterator(Tile::Cell_const_iterator i, const Tile& t) : c(i), tile(t) {}
    py_cell_iterator(const py_cell_iterator& i) : c(i.c), tile(i.tile) {}

    py_cell_iterator& operator++()
    {
        ++c;
        return *this;
    }

    py_cell_iterator operator++(int)
    {
        py_cell_iterator tmp(*this);
        operator++();
        return tmp;
    }

    bool operator==(const py_cell_iterator& rhs) const { return c==rhs.c; }
    bool operator!=(const py_cell_iterator& rhs) const { return c!=rhs.c; }

    py_cell_iterator& operator*()
    {
        return *this;
    }

    boost::python::tuple point(int iv) const
    {
        Tile::Vertex_const_handle v = tile.vertex(c, iv);
        Tile::Point p = tile.point(v);
        return boost::python::make_tuple(p[0], p[1], tile.vertex_id(v));
    }

    inline bool is_infinite() const
    {
        return tile.cell_is_infinite(c);
    }
};

class py_point_iterator
{
private:
    const boost::python::list& points;
    int i;
    Tile::Point p;

public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = Tile::Point;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    py_point_iterator(const boost::python::list& p) : points(p), i(0) {}
    py_point_iterator(const py_point_iterator& p) : points(p.points), i(p.i) {}

    py_point_iterator& operator++()
    {
        i+=2;
        return *this;
    }

    py_point_iterator operator++(int)
    {
        py_point_iterator tmp(*this);
        operator++();
        return tmp;
    }

    bool operator==(const py_point_iterator& rhs) const { return i==rhs.i; }
    bool operator!=(const py_point_iterator& rhs) const { return i!=rhs.i; }

    Tile::Point operator*()
    {
        double x = boost::python::extract<double>(points[i  ]);
        double y = boost::python::extract<double>(points[i+1]);
        return p = Tile::Point(x,y);
    }
};

class py_tile : public Tile
{
public:
    py_tile(int id, int dimension = D) : Tile(id, dimension) {}

    py_vertex_iterator py_vertices_begin() const { return py_vertex_iterator(vertices_begin(), *this); }
    py_vertex_iterator py_vertices_end  () const { return py_vertex_iterator(vertices_end  (), *this); }

    py_cell_iterator py_cells_begin() const { return py_cell_iterator(cells_begin(), *this); }
    py_cell_iterator py_cells_end()   const { return py_cell_iterator(cells_end  (), *this); }
};

#include <CGAL/DDT/scheduler/Multithread_scheduler.h>
typedef CGAL::DDT::Multithread_scheduler<py_tile> Scheduler;

#include <CGAL/DDT.h>
class py_DDT : public CGAL::DDT::DDT<Traits, Scheduler, py_tile>
{
private:
    typedef CGAL::DDT::DDT<Traits, Scheduler, py_tile> DDT;

public:
    py_DDT(int max_concurrency=0) : DDT(max_concurrency) {}

    boost::python::tuple py_vertex(int id) const
    {
        Vertex_const_iterator v = vertices_begin();
        v += id;
        Point p = point(v);
        return boost::python::make_tuple(p[0], p[1], tile_id(v));
    }

    boost::python::tuple py_cell(int id) const
    {
        Cell_const_iterator c = cells_begin();
        c += id;
        Tile_const_iterator tile = c.tile();
        std::vector<int> vid;
        assert(tile->current_dimension() == 2);
        for(int d = 0; d <= tile->current_dimension(); d++)
        {
            Vertex_const_iterator vit = main(vertex(c, d));
            vid.push_back(vertex_id(vit));
        }

        return boost::python::make_tuple(vid[0], vid[1], vid[2]);
    }

//    void write_vrt_ring(int id, int deg)
//    {
//        auto c = cells_begin();
//        c += id;
//        std::string filename(std::string("./vrt/ring_") +
//                             std::to_string(id)+ "_" + std::to_string(deg) +
//                             std::string(".vrt"));

//        std::set<Cell_const_iterator> lnb;
//        get_ring(c, deg, lnb);
//        write_vrt_cell_range(lnb.begin(), lnb.end(), filename);
//    }

};

#include <CGAL/DDT/partitioner/Const_partitioner.h>
#include <CGAL/DDT/partitioner/Grid_partitioner.h>
#include <CGAL/DDT/partitioner/Random_partitioner.h>
typedef CGAL::DDT::Const_partitioner<Traits> Const_partitioner;
typedef CGAL::DDT::Grid_partitioner<Traits> Grid_partitioner;
typedef CGAL::DDT::Random_partitioner<Traits> Random_partitioner;

typedef CGAL::DDT::Bbox<Traits::D, double> Bbox;

template<typename Iterator, typename Param, typename Partitioner>
inline void send_points(py_DDT& tri, Param p, int count, Partitioner& part)
{
    tri.send_points(Iterator(p), count, part);
}

using namespace boost::python;
BOOST_PYTHON_MODULE(pyddt)
{
    class_<Bbox>("Bbox", init<double,double>())
    ;

    class_<Const_partitioner>("Const_partitioner", init<Id>())
    ;

    class_<Grid_partitioner>("Grid_partitioner", init<Bbox,int>())
    ;

    class_<Random_partitioner>("Random_partitioner", init<Id,Id,unsigned int>())
    ;

    class_<Random_points>("Random_points", init<int,double>())
    ;

    class_<py_cell_iterator>("Cell", no_init)
    .def("is_infinite", &py_cell_iterator::is_infinite)
    .def("point", &py_cell_iterator::point)
    ;

    Id (py_tile::*Tile_id)() const = &py_tile::id;
    class_<py_tile>("Tile", no_init)
    .def("id", Tile_id)
    .def("number_of_vertices", &py_tile::number_of_vertices)
    .def("number_of_cells", &py_tile::number_of_cells)
    .def("cells", range(&py_tile::py_cells_begin, &py_tile::py_cells_end))
    .def("vertices", range(&py_tile::py_vertices_begin, &py_tile::py_vertices_end))
    ;

    py_DDT::Tile_const_iterator (py_DDT::*DDT_tiles_begin)() const = &py_DDT::tiles_begin;
    py_DDT::Tile_const_iterator (py_DDT::*DDT_tiles_end)()   const = &py_DDT::tiles_end;
    bool (py_DDT::*DDT_is_valid)()   const = &py_DDT::is_valid;

    class_<py_DDT>("DDT", init<int>())
    .def("send_points", &send_points<py_point_iterator, list, Random_partitioner>)
    .def("send_points", &send_points<py_point_iterator, list, Grid_partitioner>)
    .def("send_points", &py_DDT::send_points<Random_points, Random_partitioner>)
    .def("send_points", &py_DDT::send_points<Random_points, Grid_partitioner>)
    //.def("insert_received_points", &py_DDT::insert_received_points)
    //.def("send_all_axis_extreme_points", &py_DDT::send_all_axis_extreme_points)
    //.def("splay_stars", &py_DDT::splay_stars)
    .def("read_cgal", &CGAL::DDT::read_cgal<py_DDT>)
    .def("write_ply", &CGAL::DDT::write_ply<py_DDT>)
    .def("write_cgal", &CGAL::DDT::write_cgal<py_DDT>)
    .def("write_vrt_vert", &CGAL::DDT::write_vrt_vert<py_DDT>)
    .def("write_vrt_cell", &CGAL::DDT::write_vrt_cell<py_DDT>)
    .def("write_vrt_facet", &CGAL::DDT::write_vrt_facet<py_DDT>)
    .def("write_vrt_cells", &CGAL::DDT::write_vrt_cells<py_DDT>)
    .def("write_vrt_verts", &CGAL::DDT::write_vrt_verts<py_DDT>)
    .def("write_vrt_bbox_vert", &CGAL::DDT::write_vrt_bbox_vert<py_DDT>)
    .def("write_vrt_bbox", &CGAL::DDT::write_vrt_bbox<py_DDT>)
    .def("write_vrt_tin", &CGAL::DDT::write_vrt_tin<py_DDT>)
    .def("write_json_tri",&CGAL::DDT::write_geojson_tri<py_DDT>)
    .def("write_adjacency_graph_dot", &CGAL::DDT::write_adjacency_graph_dot<py_DDT>)
//    .def("write_vrt_ring", &py_DDT::write_vrt_ring)
    .def("number_of_cells", &py_DDT::number_of_cells)
    .def("number_of_vertices", &py_DDT::number_of_vertices)
    .def("number_of_facets", &py_DDT::number_of_facets)
    .def("is_adjacency_graph_symmetric", &py_DDT::is_adjacency_graph_symmetric)
    .def("tiles", range(DDT_tiles_begin, DDT_tiles_end))
    // .def("cells", range(&py_DDT::cells_begin,&py_DDT::cells_end)) // Cell_iterator::operator* does not exist
    .def("vertex", &py_DDT::py_vertex)
    .def("cell", &py_DDT::py_cell)
    .def("is_valid", DDT_is_valid)
    .def("finalize", &py_DDT::finalize)
    ;



}
