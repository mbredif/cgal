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

#include <CGAL/DDT/traits/Triangulation_traits.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/point_generators_2.h>
#include <CGAL/Spatial_sort_traits_adapter_2.h>
#include <CGAL/spatial_sort.h>
#include <CGAL/Bbox_2.h>
#include <CGAL/DDT/traits/Facet_index.h>

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTTraitsClasses
/// 2-dimensional triangulation traits.
/// \cgalModels TriangulationTraits
template<typename GT, typename TDS>
struct Triangulation_traits<CGAL::Delaunay_triangulation_2<GT, TDS>>
{
    static constexpr int D = 2;
    using Triangulation = CGAL::Delaunay_triangulation_2<GT, TDS>;
    typedef typename GT::Point_2                            Point;

    typedef typename TDS::Vertex_iterator                            Vertex_index;
    typedef typename TDS::Face_iterator                              Cell_index;
    typedef CGAL::DDT::Facet_index<2, Cell_index>                    Facet_index;

    static inline Triangulation triangulation(int /*unused*/)
    {
        return Triangulation();
    }


    static inline int current_dimension(const Triangulation& tri)
    {
        return tri.dimension();
    }
    static inline int maximal_dimension(const Triangulation& tri)
    {
        return D;
    }
    static inline std::size_t number_of_cells(const Triangulation& tri)
    {
        return tri.number_of_faces();
    }
    static inline std::size_t number_of_vertices(const Triangulation& tri)
    {
        return tri.number_of_vertices();
    }
    static inline Vertex_index vertex(const Triangulation& tri, Cell_index c, int i)
    {
        return c->vertex(i);
    }
    static inline Vertex_index vertices_begin(const Triangulation& tri)
    {
        return tri.all_vertices_begin();
    }
    static inline Vertex_index vertices_end(const Triangulation& tri)
    {
        return tri.all_vertices_end();
    }
    static inline Facet_index facets_begin(const Triangulation& tri)
    {
        return facet(tri, cells_begin(tri), 0);
    }
    static inline Facet_index facets_end(const Triangulation& tri)
    {
        return facet(tri, cells_end(tri), 0);
    }
    static inline Cell_index cells_begin(const Triangulation& tri)
    {
        return tri.all_faces_begin();
    }
    static inline Cell_index cells_end(const Triangulation& tri)
    {
        return tri.all_faces_end();
    }

    static inline Vertex_index infinite_vertex(const Triangulation& tri)
    {
        return tri.infinite_vertex();
    }

    static inline void clear(Triangulation& tri)
    {
        return tri.clear();
    }

    static void spatial_sort(const Triangulation& tri, std::vector<std::size_t>& indices, const std::vector<Point>& points)
    {
        typedef typename Pointer_property_map<Point>::const_type Pmap;
        typedef Spatial_sort_traits_adapter_2<GT,Pmap> Search_traits;
        CGAL::spatial_sort(indices.begin(), indices.end(), Search_traits(make_property_map(points), tri.geom_traits()));
    }

    template<typename OutputIterator>
    static OutputIterator incident_cells(const Triangulation& tri, Vertex_index v, OutputIterator out)
    {
        typename TDS::Face_circulator c = tri.incident_faces(v), done = c;
        if ( ! c.is_empty()) {
          do {
              *out++ = c;
          } while (++c != done);
        }
        return out;
    }

    template<typename OutputIterator>
    static OutputIterator adjacent_vertices(const Triangulation& tri, Vertex_index v, OutputIterator out)
    {
        typename TDS::Vertex_circulator c = tri.incident_vertices(v), done = c;
        if ( ! c.is_empty()) {
          do {
            *out++ = c;
          } while (++c != done);
        }
        return out;
    }

    static Vertex_index locate_vertex(const Triangulation& tri, const Point& p, Vertex_index hint = Vertex_index())
    {
        typename Triangulation::Locate_type  lt;
        int li;
        Cell_index c = tri.locate(p, lt, li, hint == Vertex_index() ? Cell_index() : hint->face());
        return (lt==Triangulation::VERTEX) ? vertex(tri, c, li) : vertices_end(tri);
    }

    static std::pair<Vertex_index, bool> insert(Triangulation& tri, const Point& p, Vertex_index hint = Vertex_index())
    {
        typename Triangulation::Locate_type lt;
        int li;
        Cell_index c = tri.locate(p, lt, li, hint == Vertex_index() ? Cell_index() : hint->face());
        if(lt == Triangulation::VERTEX) {
            Vertex_index v = c->vertex(li);
            return std::make_pair(v, false);
        }
        Vertex_index v = tri.insert(p, lt, c, li);
        return std::make_pair(v, true);
    }

    static inline void remove(Triangulation& tri, Vertex_index v)
    {
        tri.remove(v);
    }

    static inline bool vertex_is_infinite(const Triangulation& tri, Vertex_index v)
    {
        return tri.is_infinite(v);
    }

    static inline bool facet_is_infinite(const Triangulation& tri, Facet_index f)
    {
        for(int i = 0; i<=D; ++i)
            if(i!=f.index_of_covertex() && tri.is_infinite(f.cell()->vertex(i)))
                return true;
        return false;
    }

    static inline bool cell_is_infinite(const Triangulation& tri, Cell_index c)
    {
        for(int i = 0; i<=D; ++i)
            if(tri.is_infinite(c->vertex(i)))
                return true;
        return false;
    }

    static inline const Point& point(const Triangulation& tri, Vertex_index v)
    {
        return v->point();
    }

    static inline double approximate_cartesian_coordinate(const Point& p, int i)
    {
        return CGAL::to_double(p[i]);
    }

    static bool are_vertices_equal(const Triangulation& t1, Vertex_index v1, const Triangulation& t2, Vertex_index v2)
    {
        bool inf1 = vertex_is_infinite(t1, v1);
        bool inf2 = vertex_is_infinite(t2, v2);
        return (inf1 || inf2) ? (inf1 == inf2) : v1->point() == v2->point();
    }

    static bool are_facets_equal(const Triangulation& t1, Facet_index f1, const Triangulation& t2, Facet_index f2)
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

    static bool are_cells_equal(const Triangulation& t1, Cell_index c1, const Triangulation& t2, Cell_index c2)
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

    static inline int index_of_covertex(const Triangulation& tri, Facet_index f)
    {
        return f.index_of_covertex();
    }

    static inline Vertex_index covertex(const Triangulation& tri, Facet_index f)
    {
        return vertex(tri, f.cell(), f.index_of_covertex());
    }

    static inline Vertex_index mirror_vertex(const Triangulation& tri, Facet_index f)
    {
        Cell_index c = f.cell();
        Cell_index n = c->neighbor(f.index_of_covertex());
        return vertex(tri, n, n->index(c));
    }

    static inline Cell_index cell(const Triangulation& tri, Facet_index f)
    {
        return f.cell();
    }

    static inline Cell_index cell(const Triangulation& tri, Vertex_index v)
    {
        return v->face();
    }

    static Facet_index mirror_facet(const Triangulation& tri, Facet_index f)
    {
        Cell_index c = f.cell();
        Cell_index n = c->neighbor(f.index_of_covertex());
        return facet(tri, n, n->index(c));
    }

    static inline int mirror_index(const Triangulation& tri, Facet_index f)
    {
        return mirror_index(f.cell(), f.index_of_covertex());
    }

    static inline int mirror_index(const Triangulation& tri, Cell_index c, int i)
    {
        return c->neighbor(i)->index(c);
    }

    static inline Cell_index neighbor(const Triangulation& tri, Cell_index c, int i)
    {
        return c->neighbor(i);
    }

    static inline Facet_index facet(const Triangulation& tri, Cell_index c, int i)
    {
        return Facet_index(c, i);
    }

    static inline bool is_valid(const Triangulation& tri, bool verbose = false, int level = 0)
    {
        return tri.is_valid(verbose, level);
    }

    static inline bool less_coordinate(const Point& p, const Point& q, int i) {
        return p[i] < q[i];
    }

    static inline std::ostream& write(std::ostream& out, const Triangulation& tri) { return out << tri; }
    static inline std::istream& read(std::istream& in, Triangulation& tri) { return in >> tri; }

    /// Bbox type
    typedef CGAL::Bbox_2 Bbox;

    static inline Bbox bbox(const Point& p) {
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

    typedef CGAL::Random_points_in_disc_2<Point>                     Random_points_in_ball;

    struct Random_points_in_box : CGAL::Random_points_in_square_2<Point>
    {
        Random_points_in_box(int d, double g) : CGAL::Random_points_in_square_2<Point>(g)
        {
            CGAL_assertion(d==2);
        }
        Random_points_in_box(double g) : CGAL::Random_points_in_square_2<Point>(g) {}
    };


};

}
}

#endif // CGAL_DDT_TRIANGULATION_TRAITS_2_H
