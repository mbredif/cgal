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

#ifndef CGAL_DDT_DELAUNAY_TRIANGULATION_D_H
#define CGAL_DDT_DELAUNAY_TRIANGULATION_D_H

#include <CGAL/Delaunay_triangulation.h>
#include <CGAL/spatial_sort.h>
#include <CGAL/DDT/kernel/Kernel_traits_d.h>
#include <CGAL/DDT/triangulation/Facet_index.h>
#include <CGAL/DDT/triangulation/Triangulation_traits.h>
#include <CGAL/DDT/point_set/Point_set_traits.h>
#include <CGAL/Spatial_sort_traits_adapter_d.h>

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTTriangulationClasses
/// Triangulation_traits specialization for the CGAL::Delaunay_triangulation class.
/// \cgalModels{Triangulation}
/// \tparam CGAL::Delaunay_triangulation The adapted triangulation
template<typename GT, typename TDS>
struct Triangulation_traits<CGAL::Delaunay_triangulation<GT, TDS>> : public Kernel_traits<typename GT::Point_d>
{
    using Triangulation = CGAL::Delaunay_triangulation<GT, TDS>;

    typedef typename Triangulation::Vertex_const_iterator          Vertex_index;
    typedef typename Triangulation::Full_cell_const_iterator       Cell_index;
    typedef typename GT::Point_d                                   Point;
    typedef typename Kernel_traits<Point>::Point_const_reference   Point_const_reference;

    static constexpr int D = Impl::Dim_value<typename Triangulation::Maximal_dimension>::value;
    typedef CGAL::DDT::Facet_index<D, Cell_index>                  Facet_index;

private:
    typedef typename Triangulation::Vertex_iterator                Vertex_iterator;
    typedef typename Triangulation::Full_cell_iterator             Cell_iterator;

    static Vertex_iterator remove_const_workaround(Vertex_index v) {
        return Vertex_iterator(const_cast<typename Vertex_iterator::pointer>(v.operator->()));
    }

public:
    static inline Triangulation triangulation(int dimension)
    {
        return Triangulation(dimension);
    }

    static inline int current_dimension(const Triangulation& tri)
    {
        return tri.current_dimension();
    }

    static inline int maximal_dimension(const Triangulation& tri)
    {
        return tri.maximal_dimension();
    }

    static inline std::size_t number_of_cells(const Triangulation& tri)
    {
        return tri.number_of_full_cells();
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
        return tri.vertices_begin();
    }
    static inline Vertex_index vertices_end(const Triangulation& tri)
    {
        return tri.vertices_end();
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
        return tri.full_cells_begin();
    }
    static inline Cell_index cells_end(const Triangulation& tri)
    {
        return tri.full_cells_end();
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
        typedef Spatial_sort_traits_adapter_d<GT,Pmap> Search_traits;
        CGAL::spatial_sort(indices.begin(), indices.end(), Search_traits(make_property_map(points), tri.geom_traits()));
    }

    template<typename OutputIterator>
    static OutputIterator incident_cells(const Triangulation& tri, Vertex_index v, OutputIterator out)
    {
        return tri.incident_full_cells(v, out);
    }

    template<typename OutputIterator>
    static OutputIterator adjacent_vertices(const Triangulation& tri, Vertex_index v, OutputIterator out)
    {
        std::vector<Cell_index> cells;
        incident_cells(tri, v, std::back_inserter(cells));

        std::set<Vertex_index> vertices;
        for(Cell_index c : cells)
        {
            for( int i = 0; i <= tri.current_dimension(); ++i ) {
                Vertex_index w = c->vertex(i);
                if (w != v && vertices.insert(w).second)
                    *out++ = w;
            }
        }
        return out;
    }

    static Vertex_index locate_vertex(const Triangulation& tri, Point_const_reference p, Vertex_index hint = Vertex_index())
    {
        if (hint == Vertex_index()) hint = tri.infinite_vertex();
        typename Triangulation::Locate_type  lt;
        typename Triangulation::Face f(tri.maximal_dimension());
        typename Triangulation::Facet ft;
        tri.locate(p, lt, f, ft, remove_const_workaround(hint));
        return (lt==Triangulation::ON_VERTEX) ? f.vertex(0) : vertices_end(tri);
    }

    template<typename TileIndex>
    static std::pair<Vertex_iterator, bool> insert(Triangulation& tri, Point_const_reference p, TileIndex /*i*/, Vertex_index hint = Vertex_index())
    {
        typename Triangulation::Locate_type lt;
        typename Triangulation::Face f(tri.maximal_dimension());
        typename Triangulation::Facet ft;
        if (hint == Vertex_index()) hint = tri.infinite_vertex();
        Cell_iterator c = tri.locate(p, lt, f, ft, remove_const_workaround(hint));
        if(lt == Triangulation::ON_VERTEX) {
            Vertex_iterator v = c->vertex(f.index(0));
            v->set_point(p);
            return std::make_pair(v, false);
        }
        Vertex_iterator v = tri.insert(p, lt, f, ft, c);
        return std::make_pair(v, true);
    }

    static inline void remove(Triangulation& tri, Vertex_index v)
    {
        tri.remove(remove_const_workaround(v));
    }

    static inline bool vertex_is_infinite(const Triangulation& tri, Vertex_index v)
    {
        return tri.is_infinite(v);
    }

    static inline bool facet_is_infinite(const Triangulation& tri, Facet_index f)
    {
        for(int i = 0; i<=tri.current_dimension(); ++i)
            if(i!=f.index_of_covertex() && tri.is_infinite(vertex(tri, f.cell(), i)))
                return true;
        return false;
    }

    static inline bool cell_is_infinite(const Triangulation& tri, Cell_index c)
    {
        for(int i = 0; i<=tri.current_dimension(); ++i)
            if(tri.is_infinite(c->vertex(i)))
                return true;
        return false;
    }

    static inline Point_const_reference point(const Triangulation& tri, Vertex_index v)
    {
        return v->point();
    }

    static bool are_vertices_equal(const Triangulation& t1, Vertex_index v1, const Triangulation& t2, Vertex_index v2)
    {
        bool inf1 = vertex_is_infinite(t1, v1);
        bool inf2 = vertex_is_infinite(t2, v2);
        return (inf1 || inf2) ? (inf1 == inf2) : v1->point() == v2->point();
    }

    static bool are_facets_equal(const Triangulation& t1, Facet_index f1, const Triangulation& t2, Facet_index f2)
    {
        assert(t1.current_dimension() == t2.current_dimension());
        int D = t1.current_dimension();
        Cell_index c1 = f1.cell();
        Cell_index c2 = f2.cell();
        int icv1 = f1.index_of_covertex();
        int icv2 = f2.index_of_covertex();
        std::vector<int> perm(D+1);
        for(int i1 = 0; i1 <= D; ++i1 )
        {
            if(i1 == icv1) {
                perm[icv1] = icv2;
                continue;
            }
            auto v1 = c1->vertex(i1);
            bool found = false;
            for(int i2 = 0; i2 <= D; ++i2 )
            {
                if(i2 == icv2) continue;
                auto v2 = c2->vertex(i2);
                if(are_vertices_equal(t1, v1, t2, v2))
                {
                    found = true;
                    perm[i1] = i2;
                    break;
                }
            }
            if(!found)
                return false;
        }

        bool orientation = true;
        for(int i = 0; i <= D; ++i)
            for(int j = i+1; j <= D; ++j)
                if (perm[i] > perm[j]) orientation = !orientation;
        return orientation;
    }

    static bool are_cells_equal(const Triangulation& t1, Cell_index c1, const Triangulation& t2, Cell_index c2)
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
        return vertex(tri, n, c->mirror_index(f.index_of_covertex()));
    }

    static inline Cell_index cell_of_facet(const Triangulation& tri, Facet_index f)
    {
        return f.cell();
    }

    static inline Cell_index cell_of_vertex(const Triangulation& tri, Vertex_index v)
    {
        return v->full_cell();
    }

    static Facet_index mirror_facet(const Triangulation& tri, Facet_index f)
    {
        Cell_index c = f.cell();
        Cell_index n = c->neighbor(f.index_of_covertex());
        return facet(tri, n, c->mirror_index(f.index_of_covertex()));
    }

    static inline int mirror_index(const Triangulation& tri, Facet_index f)
    {
        return mirror_index(f.cell(), f.index_of_covertex());
    }

    static inline int mirror_index(const Triangulation& tri, Cell_index c, int i)
    {
        return c->mirror_index(i);
    }

    static inline Cell_index neighbor(const Triangulation& tri, Cell_index c, int i)
    {
        return c->neighbor(i);
    }

    static inline Facet_index facet(const Triangulation& tri, Cell_index c, int i)
    {
        return Facet_index(c, i, tri.maximal_dimension());
    }

    static inline bool is_valid(const Triangulation& tri, bool verbose = false, int level = 0)
    {
        return tri.is_valid(verbose, level);
    }

    static inline std::ostream& write(std::ostream& out, const Triangulation& tri) { return out << tri; }
    static inline std::istream& read(std::istream& in, Triangulation& tri) { return in >> tri; }
};

/// \ingroup PkgDDTPointSetClasses
/// Point_set_traits specialization for the CGAL::Delaunay_triangulation class.
/// \cgalModels{PointSet}
/// \tparam CGAL::Delaunay_triangulation The adapted triangulation.
template<typename GT, typename TDS_>
struct Point_set_traits<CGAL::Delaunay_triangulation<GT, TDS_>> : public Triangulation_traits<CGAL::Delaunay_triangulation<GT, TDS_>>
{
    typedef Triangulation_traits<CGAL::Delaunay_triangulation<GT, TDS_>> Traits;
    typedef typename Traits::Vertex_index const_iterator;
    typedef typename Traits::Vertex_index iterator;
};

}
}

#endif // CGAL_DDT_DELAUNAY_TRIANGULATION_D_H
