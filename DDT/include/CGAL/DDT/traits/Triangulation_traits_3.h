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

#ifndef CGAL_DDT_TRIANGULATION_TRAITS_3_H
#define CGAL_DDT_TRIANGULATION_TRAITS_3_H

#include <CGAL/point_generators_3.h>
#include <CGAL/Spatial_sort_traits_adapter_3.h>
#include <CGAL/spatial_sort.h>
#include <CGAL/Bbox_3.h>
#include <CGAL/DDT/traits/Facet_index.h>
#include <CGAL/DDT/traits/Vertex_info_property_map.h>

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTTraitsClasses
/// 3-dimensional triangulation traits.
/// \cgalModels TriangulationTraits
template<typename T, typename TileIndexPmap = Vertex_info_property_map<T>>
struct Triangulation_traits_3
{
    static constexpr int D = 3;
    typedef T                                                        Triangulation;
    typedef typename Triangulation::Geom_traits                      Geom_traits;
    typedef typename Geom_traits::Point_3                            Point;
    typedef TileIndexPmap                                            Tile_index_pmap;
    typedef typename Tile_index_pmap::value_type                     Tile_index;

    typedef typename Triangulation::Triangulation_data_structure     TDS;
    typedef typename TDS::Vertex_iterator                            Vertex_index;
    typedef typename TDS::Cell_iterator                              Cell_index;
    typedef CGAL::DDT::Facet_index<3, Cell_index>                    Facet_index;

    typedef CGAL::Random_points_in_sphere_3<Point>                   Random_points_in_ball;

    Tile_index_pmap tile_index_pmap;

    Triangulation_traits_3(int d = 0) { assert(d==0 || d==D); }
    inline constexpr int dimension() const { return D; }

    /// Bbox type
    typedef CGAL::Bbox_3 Bbox;

    inline Bbox bbox(const Point& p) const {
        return CGAL::Bbox_3(p.x(), p.y(), p.z(), p.x(), p.y(), p.z());
    }

    static inline Bbox bbox(unsigned int d, double range) {
      CGAL_assertion(d==3);
      return Bbox_3(-range, -range, -range, range, range, range);
    }

    static inline Bbox bbox(unsigned int d) {
      CGAL_assertion(d==3);
      return CGAL::Bbox_3();
    }

    struct Random_points_in_box : CGAL::Random_points_in_cube_3<Point>
    {
        Random_points_in_box(int d, double g) : CGAL::Random_points_in_cube_3<Point>(g)
        {
            CGAL_assertion(d==D);
        }
        Random_points_in_box(double g) : CGAL::Random_points_in_cube_3<Point>(g) {}
    };

    inline Triangulation triangulation() const
    {
        return Triangulation();
    }

    inline Tile_index vertex_id(const Triangulation& tri, Vertex_index v) const
    {
        return get(tile_index_pmap, v);
    }
    inline int current_dimension(const Triangulation& tri) const
    {
        return tri.dimension();
    }
    inline int maximal_dimension(const Triangulation& tri) const
    {
        return D;
    }
    inline std::size_t number_of_cells(const Triangulation& tri) const
    {
        return tri.number_of_cells();
    }
    inline std::size_t number_of_vertices(const Triangulation& tri) const
    {
        return tri.number_of_vertices();
    }
    inline Vertex_index vertex(const Triangulation& tri, Cell_index c, int i) const
    {
        return c->vertex(i);
    }
    inline Vertex_index vertices_begin(const Triangulation& tri) const
    {
        return tri.all_vertices_begin();
    }
    inline Vertex_index vertices_end(const Triangulation& tri) const
    {
        return tri.all_vertices_end();
    }
    inline Facet_index facets_begin(const Triangulation& tri) const
    {
        return facet(tri, cells_begin(tri), 0);
    }
    inline Facet_index facets_end(const Triangulation& tri) const
    {
        return facet(tri, cells_end(tri), 0);
    }
    inline Cell_index cells_begin(const Triangulation& tri) const
    {
        return tri.all_cells_begin();
    }
    inline Cell_index cells_end(const Triangulation& tri) const
    {
        return tri.all_cells_end();
    }

    inline Vertex_index infinite_vertex(const Triangulation& tri) const
    {
        return tri.infinite_vertex();
    }

    inline void clear(Triangulation& tri) const
    {
        return tri.clear();
    }

    void spatial_sort(const Triangulation& tri, std::vector<std::size_t>& indices, const std::vector<Point>& points) const
    {
        typedef typename Pointer_property_map<Point>::const_type Pmap;
        typedef Spatial_sort_traits_adapter_3<Geom_traits,Pmap> Search_traits;
        CGAL::spatial_sort(indices.begin(), indices.end(), Search_traits(make_property_map(points), tri.geom_traits()));
    }

    template<typename OutputIterator>
    inline OutputIterator incident_cells(const Triangulation& tri, Vertex_index v, OutputIterator out) const
    {
        return tri.incident_cells(v, out);
    }

    template<typename OutputIterator>
    inline OutputIterator adjacent_vertices(const Triangulation& tri, Vertex_index v, OutputIterator out) const
    {
        return tri.adjacent_vertices(v, out);
    }

    Vertex_index locate_vertex(const Triangulation& tri, const Point& p, Vertex_index hint = Vertex_index()) const
    {
        typename Triangulation::Locate_type  lt;
        int li, lj;
        Cell_index c = tri.locate(p, lt, li, lj, hint);
        return (lt==Triangulation::VERTEX) ? vertex(tri, c, li) : vertices_end(tri);
    }

    std::pair<Vertex_index, bool> insert(Triangulation& tri, const Point& p, Tile_index id, Vertex_index hint = Vertex_index()) const
    {
        typename Triangulation::Locate_type lt;
        int li, lj;
        Cell_index c = tri.locate(p, lt, li, lj, hint);
        if(lt == Triangulation::VERTEX) {
            Vertex_index v = c->vertex(li);
            assert(id == vertex_id(tri, v));
            return std::make_pair(v, false);
        }
        Vertex_index v = tri.insert(p, lt, c, li, lj);
        put(tile_index_pmap, v, id);
        return std::make_pair(v, true);
    }

    inline void remove(Triangulation& tri, Vertex_index v) const
    {
        tri.remove(v);
    }

    inline bool vertex_is_infinite(const Triangulation& tri, Vertex_index v) const
    {
        return tri.is_infinite(v);
    }

    inline bool facet_is_infinite(const Triangulation& tri, Facet_index f) const
    {
        for(int i = 0; i<=D; ++i)
            if(i!=f.index_of_covertex() && tri.is_infinite(f.cell()->vertex(i)))
                return true;
        return false;
    }

    inline bool cell_is_infinite(const Triangulation& tri, Cell_index c) const
    {
        for(int i = 0; i<=D; ++i)
            if(tri.is_infinite(c->vertex(i)))
                return true;
        return false;
    }

    inline const Point& point(const Triangulation& tri, Vertex_index v) const
    {
        return v->point();
    }

    static inline double approximate_cartesian_coordinate(const Point& p, int i)
    {
        return CGAL::to_double(p[i]);
    }

    bool are_vertices_equal(const Triangulation& t1, Vertex_index v1, const Triangulation& t2, Vertex_index v2) const
    {
        bool inf1 = vertex_is_infinite(t1, v1);
        bool inf2 = vertex_is_infinite(t2, v2);
        return (inf1 || inf2) ? (inf1 == inf2) : v1->point() == v2->point();
    }

    bool are_facets_equal(const Triangulation& t1, Facet_index f1, const Triangulation& t2, Facet_index f2) const
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

    bool are_cells_equal(const Triangulation& t1, Cell_index c1, const Triangulation& t2, Cell_index c2) const
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

    inline int index_of_covertex(const Triangulation& tri, Facet_index f) const
    {
        return f.index_of_covertex();
    }

    inline Vertex_index covertex(const Triangulation& tri, Facet_index f) const
    {
        return vertex(tri, f.cell(), f.index_of_covertex());
    }

    inline Vertex_index mirror_vertex(const Triangulation& tri, Facet_index f) const
    {
        Cell_index c = f.cell();
        Cell_index n = c->neighbor(f.index_of_covertex());
        return vertex(tri, n, mirror_index(tri, c, f.index_of_covertex()));
    }

    inline Cell_index cell(const Triangulation& tri, Facet_index f) const
    {
        return f.cell();
    }

    inline Cell_index cell(const Triangulation& tri, Vertex_index v) const
    {
        return v->cell();
    }

    Facet_index mirror_facet(const Triangulation& tri, Facet_index f) const
    {
        Cell_index c = f.cell();
        Cell_index n = c->neighbor(f.index_of_covertex());
        return facet(tri, n, tri.mirror_index(c, f.index_of_covertex()));
    }

    inline int mirror_index(const Triangulation& tri, Facet_index f) const
    {
        return mirror_index(f.cell(), f.index_of_covertex());
    }

    inline int mirror_index(const Triangulation& tri, Cell_index c, int i) const
    {
        return tri.mirror_index(c, i);
    }

    inline Cell_index neighbor(const Triangulation& tri, Cell_index c, int i) const
    {
        return c->neighbor(i);
    }

    inline Facet_index facet(const Triangulation& tri, Cell_index c, int i) const
    {
        return Facet_index(c, cells_end(tri), i);
    }

    inline bool is_valid(const Triangulation& tri, bool verbose = false, int level = 0) const
    {
        return tri.is_valid(verbose, level);
    }

    inline bool less_coordinate(const Point& p, const Point& q, int i) const {
        return p[i] < q[i];
    }

    inline std::ostream& write(std::ostream& out, const Triangulation& tri) const { return out << tri; }
    inline std::istream& read(std::istream& in, Triangulation& tri) const { return in >> tri; }
};

}
}

#endif // CGAL_DDT_TRIANGULATION_TRAITS_3_H
