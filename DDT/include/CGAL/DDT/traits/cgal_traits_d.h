// Copyright (c) 2022 Institut Géographique National - IGN (France)
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Mathieu Brédif and Laurent Caraffa

#ifndef CGAL_DDT_CGAL_TRAITS_D_H
#define CGAL_DDT_CGAL_TRAITS_D_H


#include <CGAL/Epick_d.h>
#include <CGAL/Delaunay_triangulation.h>
#include <CGAL/point_generators_d.h>
#include <CGAL/Triangulation_ds_vertex.h>
#include <CGAL/Spatial_sort_traits_adapter_d.h>

#include <CGAL/DDT/traits/Facet_const_iterator_d.h>
#include <CGAL/DDT/traits/Bbox.h>
#include <CGAL/DDT/traits/Data.h>

namespace CGAL {
namespace DDT {

namespace Impl {

template<typename Dim, typename I, typename F>
struct Cgal_traits_d
{
    typedef Dim                                                    Dim_tag;
    typedef I                                                      Id;
    typedef F                                                      Info;
    typedef CGAL::DDT::Data<Id, Info>                              Data;
    typedef CGAL::Epick_d<Dim_tag>                                 Geom_traits;
    typedef CGAL::Triangulation_vertex<Geom_traits,Data>           Vb;
    typedef CGAL::Triangulation_full_cell<Geom_traits>             Cb;
    typedef CGAL::Triangulation_data_structure<Dim_tag,Vb,Cb>      TDS;
    typedef typename Geom_traits::Point_d                          Point;

    typedef typename TDS::Vertex_const_iterator                    Vertex_const_iterator;
    typedef typename TDS::Vertex_const_handle                      Vertex_const_handle;
    typedef typename TDS::Vertex_iterator                          Vertex_iterator;
    typedef typename TDS::Vertex_handle                            Vertex_handle;

    typedef typename TDS::Full_cell_const_iterator                 Cell_const_iterator;
    typedef typename TDS::Full_cell_const_handle                   Cell_const_handle;
    typedef typename TDS::Full_cell_iterator                       Cell_iterator;
    typedef typename TDS::Full_cell_handle                         Cell_handle;

    typedef std::pair<Cell_const_handle, int>                      Facet;
    typedef Facet_const_iterator_d<TDS>                            Facet_const_iterator;
    typedef Facet_const_iterator                                   Facet_const_handle;
    typedef Facet_const_iterator                                   Facet_iterator;
    typedef Facet_const_iterator                                   Facet_handle;

    typedef CGAL::Delaunay_triangulation<Geom_traits,TDS>          Delaunay_triangulation;
    typedef CGAL::Random_points_in_cube_d<Point>                   Random_points_in_box;

    inline Id    id  (Vertex_const_handle v) const
    {
        return v->data().id;
    }
    inline Info& info(Vertex_const_handle v) const
    {
        return v->data().info;
    }
    inline int current_dimension(const Delaunay_triangulation& dt) const
    {
        return dt.current_dimension();
    }
    inline int maximal_dimension(const Delaunay_triangulation& dt) const
    {
        return dt.maximal_dimension();
    }
    inline std::size_t number_of_cells(const Delaunay_triangulation& dt) const
    {
        return dt.number_of_full_cells();
    }
    inline std::size_t number_of_vertices(const Delaunay_triangulation& dt) const
    {
        return dt.number_of_vertices();
    }
    inline Vertex_const_handle vertex(const Delaunay_triangulation& dt, Cell_const_handle c, int i) const
    {
        return c->vertex(i);
    }
    inline Vertex_const_iterator vertices_begin(const Delaunay_triangulation& dt) const
    {
        return dt.vertices_begin();
    }
    inline Vertex_const_iterator vertices_end(const Delaunay_triangulation& dt) const
    {
        return dt.vertices_end();
    }
    inline Vertex_iterator vertices_begin(Delaunay_triangulation& dt) const
    {
        return dt.vertices_begin();
    }
    inline Vertex_iterator vertices_end(Delaunay_triangulation& dt) const
    {
        return dt.vertices_end();
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
        return dt.full_cells_begin();
    }
    inline Cell_const_iterator cells_end(const Delaunay_triangulation& dt) const
    {
        return dt.full_cells_end();
    }

    inline Vertex_handle infinite_vertex(const Delaunay_triangulation& dt) const
    {
        return dt.infinite_vertex();
    }

    inline void clear(Delaunay_triangulation& dt) const
    {
        return dt.clear();
    }

    void spatial_sort(const Delaunay_triangulation& dt, std::vector<std::size_t>& indices, const std::vector<Point>& points) const
    {
        typedef typename Pointer_property_map<Point>::const_type Pmap;
        typedef Spatial_sort_traits_adapter_d<Geom_traits,Pmap> Search_traits;
        CGAL::spatial_sort(indices.begin(), indices.end(), Search_traits(make_property_map(points), dt.geom_traits()));
    }

    template<typename OutputIterator>
    OutputIterator incident_cells(const Delaunay_triangulation& dt, Vertex_const_handle v, OutputIterator out) const
    {
        return dt.incident_full_cells(v, out);
    }

    template<typename OutputIterator>
    OutputIterator adjacent_vertices(const Delaunay_triangulation& dt, Vertex_const_handle v, OutputIterator out) const
    {
        std::vector<Cell_handle> cells;
        incident_cells(dt, v, std::back_inserter(cells));

        std::set<Vertex_handle> vertices;
        for(Cell_handle c : cells)
        {
            for( int i = 0; i <= dt.current_dimension(); ++i ) {
                Vertex_handle w = c->vertex(i);
                if (w != v && vertices.insert(w).second)
                    *out++ = w;
            }
        }
        return out;
    }

    Vertex_const_handle locate_vertex(const Delaunay_triangulation& dt, const Point& p, Vertex_handle hint = Vertex_handle()) const
    {
        if (hint == Vertex_handle()) hint = dt.infinite_vertex();
        typename Delaunay_triangulation::Locate_type  lt;
        typename Delaunay_triangulation::Face f(dt.maximal_dimension());
        typename Delaunay_triangulation::Facet ft;
        Cell_handle c = dt.locate(p, lt, f, ft, hint);
        return (lt==Delaunay_triangulation::ON_VERTEX) ? f.vertex(0) : Vertex_const_handle();
    }

    std::pair<Vertex_handle, bool> insert(Delaunay_triangulation& dt, const Point& p, Id id, Vertex_handle hint = Vertex_handle()) const
    {
        typename Delaunay_triangulation::Locate_type lt;
        typename Delaunay_triangulation::Face f(dt.maximal_dimension());
        typename Delaunay_triangulation::Facet ft;
        if (hint == Vertex_handle()) hint = dt.infinite_vertex();
        Cell_handle c = dt.locate(p, lt, f, ft, hint);
        if(lt == Delaunay_triangulation::ON_VERTEX) {
            Vertex_handle v = c->vertex(f.index(0));
            v->set_point(p);
            assert(id == v->data().id);
            return std::make_pair(v, false);
        }
        Vertex_handle v = dt.insert(p, lt, f, ft, c);
        v->data().id = id;
        return std::make_pair(v, true);
    }

    inline void remove(Delaunay_triangulation& dt, Vertex_handle v) const
    {
        dt.remove(v);
    }

    inline bool vertex_is_infinite(const Delaunay_triangulation& dt, Vertex_const_handle v) const
    {
        return dt.is_infinite(v);
    }

    inline bool facet_is_infinite(const Delaunay_triangulation& dt, Facet_const_handle f) const
    {
        for(int i = 0; i<=dt.current_dimension(); ++i)
            if(i!=f->second && dt.is_infinite(vertex(dt, f->first, i)))
                return true;
        return false;
    }

    inline bool cell_is_infinite(const Delaunay_triangulation& dt, Cell_const_handle c) const
    {
        for(int i = 0; i<=dt.current_dimension(); ++i)
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

    bool are_vertices_equal(const Delaunay_triangulation& t1, Vertex_const_handle v1, const Delaunay_triangulation& t2, Vertex_const_handle v2) const
    {
        bool inf1 = vertex_is_infinite(t1, v1);
        bool inf2 = vertex_is_infinite(t2, v2);
        return (inf1 || inf2) ? (inf1 == inf2) : v1->point() == v2->point();
    }

    bool are_facets_equal(const Delaunay_triangulation& t1, Facet_const_handle f1, const Delaunay_triangulation& t2, Facet_const_handle f2) const
    {
        auto c1 = f1->first;
        auto c2 = f2->first;
        int icv1 = f1->second;
        int icv2 = f2->second;
        for(int i1 = 0; i1 < t1.current_dimension(); ++i1 )
        {
            if(i1 == icv1) continue;
            auto v1 = c1->vertex(i1);
            bool found = false;
            for(int i2 = 0; i2 < t2.current_dimension(); ++i2 )
            {
                if(i2 == icv2) continue;
                auto v2 = c2->vertex(i2);
                if(are_vertices_equal(t1, v1, t2, v2))
                {
                    found = true;
                    break;
                }
            }
            if(!found)
                return false;
        }
        return true;
    }

    bool are_cells_equal(const Delaunay_triangulation& t1, Cell_const_handle c1, const Delaunay_triangulation& t2, Cell_const_handle c2) const
    {
        for(auto v1 = (c1)->vertices_begin(); v1 != (c1)->vertices_end(); ++v1 )
        {
            bool is_equal = false;
            for(auto v2 = (c2)->vertices_begin(); v2 != (c2)->vertices_end(); ++v2 )
            {
                if(are_vertices_equal(t1, *v1, t2, *v2))
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

    inline Vertex_const_handle covertex(const Delaunay_triangulation& dt, Facet_const_handle f) const
    {
        return vertex(dt, f->first, f->second);
    }

    inline Vertex_const_handle mirror_vertex(const Delaunay_triangulation& dt, Facet_const_handle f) const
    {
        Cell_const_iterator c = f->first;
        return vertex(dt, c->neighbor(f->second), c->mirror_index(f->second));
    }

    inline Cell_const_handle cell(const Delaunay_triangulation& dt, Facet_const_handle f) const
    {
        return f->first;
    }

    inline Cell_const_handle cell(const Delaunay_triangulation& dt, Vertex_const_handle v) const
    {
        return v->full_cell();
    }

    Facet_const_handle mirror_facet(const Delaunay_triangulation& dt, Facet_const_handle f) const
    {
        Cell_const_iterator c = f->first;
        Facet g(c->neighbor(f->second), c->mirror_index(f->second));
        return Facet_const_iterator(dt.tds(), g);
    }

    inline int mirror_index(const Delaunay_triangulation& dt, Facet_const_handle f) const
    {
        return f->first->mirror_index(f->second);
    }

    inline int mirror_index(const Delaunay_triangulation& dt, Cell_const_handle c, int i) const
    {
        return c->mirror_index(i);
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

    inline bool is_valid(const Delaunay_triangulation& dt, bool verbose = false, int level = 0) const
    {
        return dt.is_valid(verbose, level);
    }
};

}


/// \ingroup PkgDDTTraitsClasses
/// D dimensional triangulation traits, where D is specified dynamically at runtime.
/// \cgalModels TriangulationTraits
template<typename I, typename F = No_info>
struct Cgal_traits_d : public Impl::Cgal_traits_d<CGAL::Dynamic_dimension_tag,I,F>
{
private:
    int dim;
    typedef Impl::Cgal_traits_d<CGAL::Dynamic_dimension_tag,I,F> Base;

public:
    typedef typename Base::Delaunay_triangulation Delaunay_triangulation;
    typedef typename Base::Point Point;
    enum { D = 0 };
    Cgal_traits_d(int d) : dim(d) { assert(d >= 2); }
    inline constexpr int dimension() const { return dim; }
    Delaunay_triangulation triangulation() const { return Delaunay_triangulation(dimension()); }
    struct Bbox : public CGAL::DDT::Bbox<0, double, Bbox, Point> {
        Bbox(int d              ) { this->init_values(d); this->init(d       ); }
        Bbox(int d, double range) { this->init_values(d); this->init(d, range); }
        Bbox(const Point& p     ) {
            this->init_values(p.dimension());
            int i = 0;
            for(auto it = p.cartesian_begin(); it != p.cartesian_end() ; ++it, ++i) {
                std::pair<double, double> interval = CGAL::to_interval(*it);
                this->min_values[i] = interval.first;
                this->max_values[i] = interval.second;
            }
        }
        Bbox& add_point(const Point& p)
        {
            int i = 0;
            for(auto it = p.cartesian_begin(); it != p.cartesian_end() ; ++it, ++i) {
                std::pair<double, double> interval = CGAL::to_interval(*it);
                if (interval.first  < this->min_values[i]) this->min_values[i] = interval.first;
                if (interval.second > this->max_values[i]) this->max_values[i] = interval.second;
            }
            return *this;
        }
    };
};

/// \ingroup PkgDDTTraitsClasses
/// D dimensional triangulation traits, where D is specified statically at compile-time.
/// \cgalModels TriangulationTraits
template<unsigned int N, typename I, typename F = No_info>
struct Cgal_traits : public Impl::Cgal_traits_d<CGAL::Dimension_tag<N>,I,F>
{
private:
    typedef Impl::Cgal_traits_d<CGAL::Dimension_tag<N>,I,F> Base;

public:
    typedef typename Base::Delaunay_triangulation Delaunay_triangulation;
    typedef typename Base::Point Point;
    enum { D = N };
    Cgal_traits(int d = 0) { assert(d==0 || d==D); }
    inline constexpr int dimension() const { return D; }
    Delaunay_triangulation triangulation() const { return Delaunay_triangulation(dimension()); }
    struct Bbox : public CGAL::DDT::Bbox<D, double, Bbox, Point> {
        Bbox(int d = D          ) { assert(d==D) ; this->init(D       ); }
        Bbox(int d, double range) { assert(d==D) ; this->init(D, range); }
        Bbox(const Point& p     ) {
            assert(D == p.dimension());
            int i = 0;
            for(auto it = p.cartesian_begin(); it != p.cartesian_end() ; ++it, ++i) {
                std::pair<double, double> interval = CGAL::to_interval(*it);
                this->min_values[i] = interval.first;
                this->max_values[i] = interval.second;
            }
        }
        Bbox& add_point(const Point& p)
        {
            assert(D == p.dimension());
            for(int i = 0; i < D; ++i) {
                std::pair<double, double> interval = CGAL::to_interval(p[i]);
                if (interval.first  < this->min_values[i]) this->min_values[i] = interval.first;
                if (interval.second > this->max_values[i]) this->max_values[i] = interval.second;
            }
            return *this;
        }
    };
};

}
}

#endif // CGAL_DDT_CGAL_TRAITS_D_H
