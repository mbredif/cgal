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

#ifndef CGAL_DDT_TRIANGULATION_TRAITS_2_H
#define CGAL_DDT_TRIANGULATION_TRAITS_2_H

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/point_generators_2.h>

#include <CGAL/DDT/traits/Facet_index.h>
#include <CGAL/DDT/traits/Data.h>

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTTraitsClasses
/// \cgalModels TriangulationTraits
template<typename I, typename F = No_info>
struct Triangulation_traits_2
{
    enum { D = 2 };
    typedef I                                                        Tile_index;
    typedef F                                                        Info;
    typedef CGAL::DDT::Data<Tile_index, Info>                        Data;
    typedef CGAL::Exact_predicates_inexact_constructions_kernel      Geom_traits;
    typedef Triangulation_vertex_base_with_info_2<Data, Geom_traits> Vb;
    typedef CGAL::Triangulation_data_structure_2<Vb>                 TDS;
    typedef typename Geom_traits::Point_2                            Point;

    typedef typename TDS::Vertex_iterator                            Vertex_index;
    typedef typename TDS::Face_iterator                              Cell_index;
    typedef CGAL::DDT::Facet_index<2, Cell_index>                    Facet_index;

    typedef CGAL::Delaunay_triangulation_2<Geom_traits, TDS>         Delaunay_triangulation;
    typedef CGAL::Random_points_in_disc_2<Point>                     Random_points_in_ball;

    Triangulation_traits_2(int d = 0) { assert(d==0 || d==D); }
    inline constexpr int dimension() const { return D; }

    /// Bbox type
    typedef CGAL::Bbox_2 Bbox;

    inline Bbox bbox(const Point& p) const {
        return CGAL::Bbox_2(p.x(), p.y(), p.x(), p.y());
    }

    static inline Bbox bbox(unsigned int d, double range) {
      CGAL_assertion(d==2);
      return Bbox_2(-range, -range, range, range);
    }

    static inline Bbox bbox(unsigned int d) {
      CGAL_assertion(d==2);
      return CGAL::Bbox_2();
    }

    struct Random_points_in_box : CGAL::Random_points_in_square_2<Point>
    {
        Random_points_in_box(int d, double g) : CGAL::Random_points_in_square_2<Point>(g)
        {
            CGAL_assertion(d==2);
        }
        Random_points_in_box(double g) : CGAL::Random_points_in_square_2<Point>(g) {}
    };

    inline Delaunay_triangulation triangulation() const
    {
        return Delaunay_triangulation();
    }

    inline Tile_index    id  (Vertex_index v) const
    {
        return v->info().id;
    }
    inline Info& info(Vertex_index v) const
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
    inline std::size_t number_of_cells(const Delaunay_triangulation& dt) const
    {
        return dt.number_of_faces();
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
        return dt.all_vertices_begin();
    }
    inline Vertex_index vertices_end(const Delaunay_triangulation& dt) const
    {
        return dt.all_vertices_end();
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
        return dt.all_faces_begin();
    }
    inline Cell_index cells_end(const Delaunay_triangulation& dt) const
    {
        return dt.all_faces_end();
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
        typedef Spatial_sort_traits_adapter_2<Geom_traits,Pmap> Search_traits;
        CGAL::spatial_sort(indices.begin(), indices.end(), Search_traits(make_property_map(points), dt.geom_traits()));
    }

    template<typename OutputIterator>
    OutputIterator incident_cells(const Delaunay_triangulation& dt, Vertex_index v, OutputIterator out) const
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
    OutputIterator adjacent_vertices(const Delaunay_triangulation& dt, Vertex_index v, OutputIterator out) const
    {
        typename TDS::Vertex_circulator c = dt.incident_vertices(v), done = c;
        if ( ! c.is_empty()) {
          do {
            *out++ = c;
          } while (++c != done);
        }
        return out;
    }

    Vertex_index locate_vertex(const Delaunay_triangulation& dt, const Point& p, Vertex_index hint = Vertex_index()) const
    {
        typename Delaunay_triangulation::Locate_type  lt;
        int li;
        Cell_index c = dt.locate(p, lt, li, hint == Vertex_index() ? Cell_index() : hint->face());
        return (lt==Delaunay_triangulation::VERTEX) ? vertex(dt, c, li) : vertices_end(dt);
    }

    std::pair<Vertex_index, bool> insert(Delaunay_triangulation& dt, const Point& p, Tile_index id, Vertex_index hint = Vertex_index()) const
    {
        typename Delaunay_triangulation::Locate_type lt;
        int li;
        Cell_index c = dt.locate(p, lt, li, hint == Vertex_index() ? Cell_index() : hint->face());
        if(lt == Delaunay_triangulation::VERTEX) {
            Vertex_index v = c->vertex(li);
            assert(id == v->info().id);
            return std::make_pair(v, false);
        }
        Vertex_index v = dt.insert(p, lt, c, li);
        v->info().id = id;
        return std::make_pair(v, true);
    }

    inline void remove(Delaunay_triangulation& dt, Vertex_index v) const
    {
        dt.remove(v);
    }

    inline bool vertex_is_infinite(const Delaunay_triangulation& dt, Vertex_index v) const
    {
        return dt.is_infinite(v);
    }

    inline bool facet_is_infinite(const Delaunay_triangulation& dt, Facet_index f) const
    {
        for(int i = 0; i<=D; ++i)
            if(i!=f.index_of_covertex() && dt.is_infinite(f.cell()->vertex(i)))
                return true;
        return false;
    }

    inline bool cell_is_infinite(const Delaunay_triangulation& dt, Cell_index c) const
    {
        for(int i = 0; i<=D; ++i)
            if(dt.is_infinite(c->vertex(i)))
                return true;
        return false;
    }

    inline const Point& point(const Delaunay_triangulation& dt, Vertex_index v) const
    {
        return v->point();
    }

    static inline double approximate_cartesian_coordinate(const Point& p, int i)
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

    bool are_cells_equal(const Delaunay_triangulation& t1, Cell_index c1, const Delaunay_triangulation& t2, Cell_index c2) const
    {
        for(int i1=0; i1<=D; ++i1)
        {
            Vertex_index v1 = c1->vertex(i1);
            bool is_equal = false;
            for(int i2=0; i2<=D; ++i2)
            {
                Vertex_index v2 = c2->vertex(i2);
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
        return vertex(dt, n, n->index(c));
    }

    inline Cell_index cell(const Delaunay_triangulation& dt, Facet_index f) const
    {
        return f.cell();
    }

    inline Cell_index cell(const Delaunay_triangulation& dt, Vertex_index v) const
    {
        return v->face();
    }

    Facet_index mirror_facet(const Delaunay_triangulation& dt, Facet_index f) const
    {
        Cell_index c = f.cell();
        Cell_index n = c->neighbor(f.index_of_covertex());
        return facet(dt, n, n->index(c));
    }

    inline int mirror_index(const Delaunay_triangulation& dt, Facet_index f) const
    {
        return mirror_index(f.cell(), f.index_of_covertex());
    }

    inline int mirror_index(const Delaunay_triangulation& dt, Cell_index c, int i) const
    {
        return c->neighbor(i)->index(c);
    }

    inline Cell_index neighbor(const Delaunay_triangulation& dt, Cell_index c, int i) const
    {
        return c->neighbor(i);
    }

    inline Facet_index facet(const Delaunay_triangulation& dt, Cell_index c, int i) const
    {
        return Facet_index(c, cells_end(dt), i);
    }

    inline bool is_valid(const Delaunay_triangulation& dt, bool verbose = false, int level = 0) const
    {
        return dt.is_valid(verbose, level);
    }

    inline bool less_coordinate(const Point& p, const Point& q, int i) const {
        return p[i] < q[i];
    }

    inline std::ostream& write(std::ostream& out, const Delaunay_triangulation& dt) const { return out << dt; }
    inline std::istream& read(std::istream& in, Delaunay_triangulation& dt) const { return in >> dt; }
};

}
}

#endif // CGAL_DDT_TRIANGULATION_TRAITS_2_H
