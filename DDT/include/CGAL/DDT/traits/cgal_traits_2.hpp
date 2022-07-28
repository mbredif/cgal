#ifndef DDT_CGAL_TRAITS_2_HPP
#define DDT_CGAL_TRAITS_2_HPP

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/point_generators_2.h>

#include "../data.hpp"
#include "../bbox.hpp"
#include "../iterator/Facet_const_iterator_2.hpp"

namespace ddt
{

template<typename I, typename F>
struct Cgal_traits_2
{
    enum { D = 2 };
    typedef I                                                      Id;
    typedef F                                                      Flag;
    typedef ddt::Data<Id, Flag>                                    Data;
    typedef CGAL::Exact_predicates_inexact_constructions_kernel    K;
    typedef CGAL::Triangulation_vertex_base_with_info_2<Data, K>   Vb;
    typedef CGAL::Triangulation_data_structure_2<Vb>               TDS;
    typedef typename K::Point_2                                    Point;

    typedef typename TDS::Vertex_iterator                          Vertex_const_iterator;
    typedef typename TDS::Vertex_handle                            Vertex_const_handle;
    typedef typename TDS::Vertex_iterator                          Vertex_iterator;
    typedef typename TDS::Vertex_handle                            Vertex_handle;

    typedef typename TDS::Face_iterator                            Cell_const_iterator;
    typedef typename TDS::Face_handle                              Cell_const_handle;
    typedef typename TDS::Face_iterator                            Cell_iterator;
    typedef typename TDS::Face_handle                              Cell_handle;

    typedef std::pair<Cell_const_handle, int>                      Facet;
    typedef Facet_const_iterator_2<TDS>                            Facet_const_iterator;
    typedef Facet_const_iterator                                   Facet_const_handle;
    typedef Facet_const_iterator                                   Facet_iterator;
    typedef Facet_const_iterator                                   Facet_handle;

    typedef CGAL::Delaunay_triangulation_2<K, TDS>                 Delaunay_triangulation;
    typedef CGAL::Random_points_in_disc_2<Point>                   Random_points_in_ball;

    struct Random_points_in_box : CGAL::Random_points_in_square_2<Point>
    {
        Random_points_in_box(int d, double g) : CGAL::Random_points_in_square_2<Point>(g)
        {
            CGAL_assertion(d==2);
        }
        Random_points_in_box(double g) : CGAL::Random_points_in_square_2<Point>(g) {}
    };

    Delaunay_triangulation triangulation(int dimension) const
    {
        assert(dimension == D);
        return Delaunay_triangulation();
    }

    inline Id    id  (Vertex_const_handle v) const
    {
        return v->info().id;
    }
    inline Flag& flag(Vertex_const_handle v) const
    {
        return v->info().flag;
    }
    inline int current_dimension(const Delaunay_triangulation& dt) const
    {
        return dt.dimension();
    }
    inline int maximal_dimension(const Delaunay_triangulation& dt) const
    {
        return D;
    }
    inline size_t number_of_cells(const Delaunay_triangulation& dt) const
    {
        return dt.number_of_faces();
    }
    inline size_t number_of_vertices(const Delaunay_triangulation& dt) const
    {
        return dt.number_of_vertices();
    }
    inline Vertex_const_handle vertex(const Delaunay_triangulation& dt, Cell_const_handle c, int i) const
    {
        return c->vertex(i);
    }
    inline Vertex_const_iterator vertices_begin(const Delaunay_triangulation& dt) const
    {
        return dt.all_vertices_begin();
    }
    inline Vertex_const_iterator vertices_end(const Delaunay_triangulation& dt) const
    {
        return dt.all_vertices_end();
    }
    inline Vertex_iterator vertices_begin(Delaunay_triangulation& dt) const
    {
        return dt.all_vertices_begin();
    }
    inline Vertex_iterator vertices_end(Delaunay_triangulation& dt) const
    {
        return dt.all_vertices_end();
    }
    inline Facet_const_iterator facets_begin(const Delaunay_triangulation& dt) const
    {
        return Facet_const_iterator(dt.tds());
    }
    inline Facet_const_iterator facets_end(const Delaunay_triangulation& dt) const
    {
        return Facet_const_iterator();
    }
    inline Cell_const_iterator cells_begin(const Delaunay_triangulation& dt) const
    {
        return dt.all_faces_begin();
    }
    inline Cell_const_iterator cells_end(const Delaunay_triangulation& dt) const
    {
        return dt.all_faces_end();
    }

    inline Vertex_handle infinite_vertex(const Delaunay_triangulation& dt) const
    {
        return dt.infinite_vertex();
    }

    inline void clear(Delaunay_triangulation& dt) const
    {
        return dt.clear();
    }

    template<class It> inline void insert(Delaunay_triangulation& dt, It begin, It end) const
    {
        dt.insert(begin, end);
    }

    template<class It> inline void remove(Delaunay_triangulation& dt, It begin, It end) const
    {
        for(It it=begin; it!=end; ++it) dt.remove(*it);
    }

    inline Point circumcenter(const Delaunay_triangulation& dt, Cell_const_handle c) const
    {
        return dt.dual(c);
    }

    inline bool vertex_is_infinite(const Delaunay_triangulation& dt, Vertex_const_handle v) const
    {
        return dt.is_infinite(v);
    }

    inline bool facet_is_infinite(const Delaunay_triangulation& dt, Facet_const_handle f) const
    {
        for(int i = 0; i<=D; ++i)
            if(i!=f->second && dt.is_infinite(f->first->vertex(i)))
                return true;
        return false;
    }

    inline bool cell_is_infinite(const Delaunay_triangulation& dt, Cell_const_handle c) const
    {
        for(int i = 0; i<=D; ++i)
            if(dt.is_infinite(c->vertex(i)))
                return true;
        return false;
    }

    inline const Point& point(const Delaunay_triangulation& dt, Vertex_const_handle v) const
    {
        return v->point();
    }

    inline double coord(const Delaunay_triangulation& dt, const Point& p, int i) const
    {
        return CGAL::to_double(p[i]);
    }

    template<typename V>
    bool are_vertices_equal(const Delaunay_triangulation& t1, V v1, const Delaunay_triangulation& t2, V v2) const
    {
        bool inf1 = vertex_is_infinite(t1, v1);
        bool inf2 = vertex_is_infinite(t2, v2);
        return (inf1 || inf2) ? (inf1 == inf2) : v1->point() == v2->point();
    }

    template<typename C>
    bool are_cells_equal(const Delaunay_triangulation& t1, C c1, const Delaunay_triangulation& t2, C c2) const
    {
        for(int i1=0; i1<=D; ++i1)
        {
            Vertex_handle v1 = c1->vertex(i1);
            bool is_equal = false;
            for(int i2=0; i2<=D; ++i2)
            {
                Vertex_handle v2 = c2->vertex(i2);
                if(are_vertices_equal(t1, v1, t2, v2))
                {
                    is_equal = true;
                    break;
                }
            }
            if(!is_equal)
                return false;
        }
        return true;
    }

    inline int index_of_covertex(const Delaunay_triangulation& dt, Facet_const_handle f) const
    {
        return f->second;
    }

    inline Cell_const_handle full_cell(const Delaunay_triangulation& dt, Facet_const_handle f) const
    {
        return f->first;
    }

    inline int mirror_index(const Delaunay_triangulation& dt, Cell_const_handle c, int i) const
    {
        return c->neighbor(i)->index(c);
    }

    inline Cell_const_iterator neighbor(const Delaunay_triangulation& dt, Cell_const_iterator c, int i) const
    {
        return c->neighbor(i);
    }

    Facet_const_iterator facet(const Delaunay_triangulation& dt, Cell_const_iterator c, int i) const
    {
        Facet f(c, i);
        return Facet_const_iterator(dt.tds(), f);
    }
};

}

#endif // DDT_CGAL_TRAITS_2_HPP
