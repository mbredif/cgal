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

#ifndef CGAL_DDT_CGAL_TRAITS_2_H
#define CGAL_DDT_CGAL_TRAITS_2_H

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/point_generators_2.h>

#include <CGAL/DDT/traits/Facet_const_iterator_2.h>
#include <CGAL/DDT/traits/ddt_vertex_base_with_info_2.h>
#include <CGAL/DDT/traits/Data.h>

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTTraitsClasses
/// \cgalModels TriangulationTraits
/// @todo mettre le vertex en parametre template (dans tous les traits)
template<typename I, typename F = No_info>
struct Cgal_traits_2
{
    enum { D = 2 };
    typedef I                                                      Id;
    typedef F                                                      Info;
    typedef CGAL::DDT::Data<Id, Info>                              Data;
    typedef CGAL::Exact_predicates_inexact_constructions_kernel    K;
    typedef CGAL::DDT_vertex_base_with_info_2<Data, K>   Vb;
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

    Cgal_traits_2(int d = 0) { assert(d==0 || d==D); }
    inline constexpr int dimension() const { return D; }

    struct Bbox : public CGAL::Bbox_2
    {
        Bbox(unsigned int d, double range) : CGAL::Bbox_2(-range, -range, range, range) { CGAL_assertion(d==2); }
        Bbox(unsigned int d = 2) : CGAL::Bbox_2() { CGAL_assertion(d==2); }
        Bbox& operator+=(const CGAL::Bbox_2& bbox) { CGAL::Bbox_2::operator+=(bbox); return *this; }
        Bbox& operator+=(const Point& p) { CGAL::Bbox_2 bbox(p.x(), p.y(), p.x(), p.y()); *this += bbox; return *this; }
    };

    struct Random_points_in_box : CGAL::Random_points_in_square_2<Point>
    {
        Random_points_in_box(int d, double g) : CGAL::Random_points_in_square_2<Point>(g)
        {
            CGAL_assertion(d==2);
        }
        Random_points_in_box(double g) : CGAL::Random_points_in_square_2<Point>(g) {}
    };

    Delaunay_triangulation triangulation() const
    {
        return Delaunay_triangulation();
    }

    inline Id    id  (Vertex_const_handle v) const
    {
        return v->info().id;
    }
    inline Info& info(Vertex_const_handle v) const
    {
        return v->info().info;
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

    void spatial_sort(const Delaunay_triangulation& dt, std::vector<std::size_t>& indices, const std::vector<Point>& points) const
    {
        using Geom_traits = K;
        typedef typename Pointer_property_map<Point>::const_type Pmap;
        typedef Spatial_sort_traits_adapter_2<Geom_traits,Pmap> Search_traits;

        CGAL::spatial_sort(indices.begin(), indices.end(),
                     Search_traits(make_property_map(points), dt.geom_traits()));
    }

    template<typename OutputIterator>
    OutputIterator incident_cells(const Delaunay_triangulation& dt, Vertex_handle v, OutputIterator out) const
    {
        typename TDS::Face_circulator c = dt.incident_faces(v), done = c;
        if ( ! c.is_empty()) {
          do {
              *out++ = c;
          } while (++c != done);
        }
        return out;
    }

    template<typename OutputIterator>
    OutputIterator adjacent_vertices(const Delaunay_triangulation& dt, Vertex_handle v, OutputIterator out) const
    {
        typename TDS::Vertex_circulator c = dt.incident_vertices(v), done = c;
        if ( ! c.is_empty()) {
          do {
            *out++ = c;
          } while (++c != done);
        }
        return out;
    }

    Vertex_const_handle locate_vertex(const Delaunay_triangulation& dt, const Point& p, Vertex_handle hint = Vertex_handle()) const
    {
        typename Delaunay_triangulation::Locate_type  lt;
        int li;
        Cell_handle c = dt.locate(p, lt, li);
        return (lt==Delaunay_triangulation::VERTEX) ? vertex(dt, c, li) : Vertex_const_handle();
    }

    std::pair<Vertex_handle, bool> insert(Delaunay_triangulation& dt, const Point& p, Id id, Vertex_handle hint = Vertex_handle()) const
    {
        typename Delaunay_triangulation::Locate_type lt;
        int li;
        Cell_handle c = dt.locate(p, lt, li, hint == Vertex_handle() ? Cell_handle() : hint->face());
        if(lt == Delaunay_triangulation::VERTEX) {
            Vertex_handle v = c->vertex(li);
            assert(id == v->info().id);
            return std::make_pair(v, false);
        }
        Vertex_handle v = dt.insert(p, lt, c, li);
        v->info().id = id;
        return std::make_pair(v, true);
    }

    inline void remove(Delaunay_triangulation& dt, Vertex_handle v) const
    {
        dt.remove(v);
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
        for(int i1 = 0; i1 < t1.dimension(); ++i1 )
        {
            if(i1 == icv1) continue;
            auto v1 = c1->vertex(i1);
            bool found = false;
            for(int i2 = 0; i2 < t2.dimension(); ++i2 )
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

    inline Vertex_const_handle covertex(const Delaunay_triangulation& dt, Facet_const_handle f) const
    {
        return vertex(dt, f->first, f->second);
    }

    inline Vertex_const_handle mirror_vertex(const Delaunay_triangulation& dt, Facet_const_handle f) const
    {
        Cell_const_iterator c = f->first;
        Cell_const_iterator n = c->neighbor(f->second);
        return vertex(dt, n, n->index(c));
    }

    inline Cell_const_handle cell(const Delaunay_triangulation& dt, Facet_const_handle f) const
    {
        return f->first;
    }

    inline Cell_const_handle cell(const Delaunay_triangulation& dt, Vertex_const_handle v) const
    {
        return v->face();
    }

    Facet_const_handle mirror_facet(const Delaunay_triangulation& dt, Facet_const_handle f) const
    {
        Cell_const_iterator c = f->first;
        Cell_const_iterator n = c->neighbor(f->second);
        Facet g(n, n->index(c));
        return Facet_const_iterator(dt.tds(), g);
    }

    inline int mirror_index(const Delaunay_triangulation& dt, Facet_const_handle f) const
    {
        return mirror_index(f->first, f->second);
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

    inline bool is_valid(const Delaunay_triangulation& dt, bool verbose = false, int level = 0) const
    {
        return dt.is_valid(verbose, level);
    }
};

}
}

#endif // CGAL_DDT_CGAL_TRAITS_2_H
