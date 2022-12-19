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
#include <CGAL/Triangulation_ds_vertex.h>
#include <CGAL/Delaunay_triangulation.h>
#include <CGAL/point_generators_d.h>
#include <CGAL/Spatial_sort_traits_adapter_d.h>

#include <CGAL/DDT/traits/Facet_index.h>
#include <CGAL/DDT/traits/Data.h>
#include <CGAL/DDT/traits/Bbox.h>

namespace CGAL {
namespace DDT {

namespace Impl {

template<unsigned int N, typename Dim, typename I, typename F, typename Derived>
struct Cgal_traits_d
{
    typedef Dim                                                    Dim_tag;
    typedef I                                                      Tile_index;
    typedef F                                                      Info;
    typedef CGAL::DDT::Data<Tile_index, Info>                      Data;
    typedef CGAL::Epick_d<Dim_tag>                                 Geom_traits;
    typedef CGAL::Triangulation_vertex<Geom_traits,Data>           Vb;
    typedef CGAL::Triangulation_full_cell<Geom_traits>             Cb;
    typedef CGAL::Triangulation_data_structure<Dim_tag,Vb,Cb>      TDS;
    typedef typename Geom_traits::Point_d                          Point;
    typedef CGAL::DDT::Bbox<N, double, Point>                      Bbox;

    typedef typename TDS::Vertex_const_iterator                    Vertex_index;
    typedef typename TDS::Full_cell_const_iterator                 Cell_index;
    typedef CGAL::DDT::Facet_index<N, Cell_index>                  Facet_index;

    typedef CGAL::Delaunay_triangulation<Geom_traits,TDS>          Delaunay_triangulation;
    typedef CGAL::Random_points_in_cube_d<Point>                   Random_points_in_box;

private:
    typedef typename TDS::Vertex_iterator                          Vertex_iterator;
    typedef typename TDS::Full_cell_iterator                       Cell_iterator;

    Vertex_iterator remove_const_workaround(Vertex_index v) const {
        return Vertex_iterator(const_cast<typename Vertex_iterator::pointer>(v.operator->()));
    }

public:
    inline Tile_index    id  (Vertex_index v) const
    {
        return v->data().id;
    }
    inline Info& info(Vertex_index v) const
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
    inline Vertex_index vertex(const Delaunay_triangulation& dt, Cell_index c, int i) const
    {
        return c->vertex(i);
    }
    inline Vertex_index vertices_begin(const Delaunay_triangulation& dt) const
    {
        return dt.vertices_begin();
    }
    inline Vertex_index vertices_end(const Delaunay_triangulation& dt) const
    {
        return dt.vertices_end();
    }
    inline Facet_index facets_begin(const Delaunay_triangulation& dt) const
    {
        return facet(dt, cells_begin(dt), 0);
    }
    inline Facet_index facets_end(const Delaunay_triangulation& dt) const
    {
        return facet(dt, cells_end(dt), 0);
    }
    inline Cell_index cells_begin(const Delaunay_triangulation& dt) const
    {
        return dt.full_cells_begin();
    }
    inline Cell_index cells_end(const Delaunay_triangulation& dt) const
    {
        return dt.full_cells_end();
    }

    inline Vertex_index infinite_vertex(const Delaunay_triangulation& dt) const
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
    inline OutputIterator incident_cells(const Delaunay_triangulation& dt, Vertex_index v, OutputIterator out) const
    {
        return dt.incident_full_cells(v, out);
    }

    template<typename OutputIterator>
    OutputIterator adjacent_vertices(const Delaunay_triangulation& dt, Vertex_index v, OutputIterator out) const
    {
        std::vector<Cell_index> cells;
        incident_cells(dt, v, std::back_inserter(cells));

        std::set<Vertex_index> vertices;
        for(Cell_index c : cells)
        {
            for( int i = 0; i <= dt.current_dimension(); ++i ) {
                Vertex_index w = c->vertex(i);
                if (w != v && vertices.insert(w).second)
                    *out++ = w;
            }
        }
        return out;
    }

    Vertex_index locate_vertex(const Delaunay_triangulation& dt, const Point& p, Vertex_index hint = Vertex_index()) const
    {
        if (hint == Vertex_index()) hint = dt.infinite_vertex();
        typename Delaunay_triangulation::Locate_type  lt;
        typename Delaunay_triangulation::Face f(dt.maximal_dimension());
        typename Delaunay_triangulation::Facet ft;
        dt.locate(p, lt, f, ft, remove_const_workaround(hint));
        return (lt==Delaunay_triangulation::ON_VERTEX) ? f.vertex(0) : vertices_end(dt);
    }

    std::pair<Vertex_index, bool> insert(Delaunay_triangulation& dt, const Point& p, Tile_index id, Vertex_index hint = Vertex_index()) const
    {
        typename Delaunay_triangulation::Locate_type lt;
        typename Delaunay_triangulation::Face f(dt.maximal_dimension());
        typename Delaunay_triangulation::Facet ft;
        if (hint == Vertex_index()) hint = dt.infinite_vertex();
        Cell_iterator c = dt.locate(p, lt, f, ft, remove_const_workaround(hint));
        if(lt == Delaunay_triangulation::ON_VERTEX) {
            Vertex_iterator v = c->vertex(f.index(0));
            v->set_point(p);
            assert(id == v->data().id);
            return std::make_pair(Vertex_index(v), false);
        }
        Vertex_iterator v = dt.insert(p, lt, f, ft, c);
        v->data().id = id;
        return std::make_pair(Vertex_index(v), true);
    }

    inline void remove(Delaunay_triangulation& dt, Vertex_index v) const
    {
        dt.remove(remove_const_workaround(v));
    }

    inline bool vertex_is_infinite(const Delaunay_triangulation& dt, Vertex_index v) const
    {
        return dt.is_infinite(v);
    }

    inline bool facet_is_infinite(const Delaunay_triangulation& dt, Facet_index f) const
    {
        for(int i = 0; i<=dt.current_dimension(); ++i)
            if(i!=f.index_of_covertex() && dt.is_infinite(vertex(dt, f.cell(), i)))
                return true;
        return false;
    }

    inline bool cell_is_infinite(const Delaunay_triangulation& dt, Cell_index c) const
    {
        for(int i = 0; i<=dt.current_dimension(); ++i)
            if(dt.is_infinite(c->vertex(i)))
                return true;
        return false;
    }

    inline const Point& point(const Delaunay_triangulation& dt, Vertex_index v) const
    {
        return v->point();
    }

    inline double coord(const Delaunay_triangulation& dt, const Point& p, int i) const
    {
        return CGAL::to_double(p[i]);
    }

    bool are_vertices_equal(const Delaunay_triangulation& t1, Vertex_index v1, const Delaunay_triangulation& t2, Vertex_index v2) const
    {
        bool inf1 = vertex_is_infinite(t1, v1);
        bool inf2 = vertex_is_infinite(t2, v2);
        return (inf1 || inf2) ? (inf1 == inf2) : v1->point() == v2->point();
    }

    bool are_facets_equal(const Delaunay_triangulation& t1, Facet_index f1, const Delaunay_triangulation& t2, Facet_index f2) const
    {
        Cell_index c1 = f1.cell();
        Cell_index c2 = f2.cell();
        int icv1 = f1.index_of_covertex();
        int icv2 = f2.index_of_covertex();
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

    bool are_cells_equal(const Delaunay_triangulation& t1, Cell_index c1, const Delaunay_triangulation& t2, Cell_index c2) const
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

    inline int index_of_covertex(const Delaunay_triangulation& dt, Facet_index f) const
    {
        return f.index_of_covertex();
    }

    inline Vertex_index covertex(const Delaunay_triangulation& dt, Facet_index f) const
    {
        return vertex(dt, f.cell(), f.index_of_covertex());
    }

    inline Vertex_index mirror_vertex(const Delaunay_triangulation& dt, Facet_index f) const
    {
        Cell_index c = f.cell();
        Cell_index n = c->neighbor(f.index_of_covertex());
        return vertex(dt, n, c->mirror_index(f.index_of_covertex()));
    }

    inline Cell_index cell(const Delaunay_triangulation& dt, Facet_index f) const
    {
        return f.cell();
    }

    inline Cell_index cell(const Delaunay_triangulation& dt, Vertex_index v) const
    {
        return v->full_cell();
    }

    Facet_index mirror_facet(const Delaunay_triangulation& dt, Facet_index f) const
    {
        Cell_index c = f.cell();
        Cell_index n = c->neighbor(f.index_of_covertex());
        return facet(dt, n, c->mirror_index(f.index_of_covertex()));
    }

    inline int mirror_index(const Delaunay_triangulation& dt, Facet_index f) const
    {
        return mirror_index(f.cell(), f.index_of_covertex());
    }

    inline int mirror_index(const Delaunay_triangulation& dt, Cell_index c, int i) const
    {
        return c->mirror_index(i);
    }

    inline Cell_index neighbor(const Delaunay_triangulation& dt, Cell_index c, int i) const
    {
        return c->neighbor(i);
    }

    inline Facet_index facet(const Delaunay_triangulation& dt, Cell_index c, int i) const
    {
        return static_cast<const Derived*>(this)->facet(dt, c, i);
    }

    inline bool is_valid(const Delaunay_triangulation& dt, bool verbose = false, int level = 0) const
    {
        return dt.is_valid(verbose, level);
    }

    Bbox bbox(const Point& p) const {
        int d = static_cast<const Derived*>(this)->dimension();
        assert(p.dimension() == d);
        Bbox b(d);
        int i = 0;
        for(int i = 0; i < d; ++i) {
            std::pair<double, double> interval = CGAL::to_interval(p[i]);
            b.min(i) = interval.first;
            b.max(i) = interval.second;
        }
        return b;
    }

    inline std::ostream& write(std::ostream& out, const Delaunay_triangulation& dt) const { return out << dt; }
    inline std::istream& read(std::istream& in, Delaunay_triangulation& dt) const { return in >> dt; }
};

}

/// \ingroup PkgDDTTraitsClasses
/// D dimensional triangulation traits, where D is specified dynamically at runtime.
/// \cgalModels TriangulationTraits
template<typename I, typename F = No_info>
class Cgal_traits_d : public Impl::Cgal_traits_d<0, CGAL::Dynamic_dimension_tag,I,F,Cgal_traits_d<I,F>>
{
    int dim_;
    typedef Impl::Cgal_traits_d<0,CGAL::Dynamic_dimension_tag,I,F,Cgal_traits_d<I,F>> Base;

public:
    typedef typename Base::Cell_index Cell_index;
    typedef typename Base::Facet_index Facet_index;
    typedef typename Base::Delaunay_triangulation Delaunay_triangulation;
    enum { D = 0 };
    Cgal_traits_d(int d) : dim_(d)
    {
        assert(d >= 2);
    }
    inline constexpr int dimension() const {
        return dim_;
    }
    inline Delaunay_triangulation triangulation() const {
        return Delaunay_triangulation(dimension());
    }
    inline Facet_index facet(const Delaunay_triangulation& dt, Cell_index c, int i) const
    {
        return Facet_index(dimension(), c, Base::cells_end(dt), i);
    }
};

/// \ingroup PkgDDTTraitsClasses
/// D dimensional triangulation traits, where D is specified statically at compile-time.
/// \cgalModels TriangulationTraits
template<unsigned int N, typename I, typename F = No_info>
class Cgal_traits : public Impl::Cgal_traits_d<N,CGAL::Dimension_tag<N>,I,F,Cgal_traits<N,I,F>>
{
    typedef Impl::Cgal_traits_d<N,CGAL::Dimension_tag<N>,I,F,Cgal_traits<N,I,F>> Base;

public:
    typedef typename Base::Cell_index Cell_index;
    typedef typename Base::Facet_index Facet_index;
    typedef typename Base::Delaunay_triangulation Delaunay_triangulation;
    enum { D = N };
    Cgal_traits(int d = 0)
    {
        assert(d==0 || d==D);
    }
    inline constexpr int dimension() const { return D; }
    inline Delaunay_triangulation triangulation() const
    {
        return Delaunay_triangulation(dimension());
    }
    inline Facet_index facet(const Delaunay_triangulation& dt, Cell_index c, int i) const
    {
        return Facet_index(c, Base::cells_end(dt), i);
    }
};

}
}

#endif // CGAL_DDT_CGAL_TRAITS_D_H
