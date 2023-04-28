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

#ifndef CGAL_DDT_TRIANGULATION_TRAITS_D_H
#define CGAL_DDT_TRIANGULATION_TRAITS_D_H

#include <CGAL/point_generators_d.h>
#include <CGAL/Spatial_sort_traits_adapter_d.h>
#include <CGAL/DDT/traits/Bbox.h>
#include <CGAL/DDT/traits/Facet_index.h>
#include <CGAL/DDT/traits/Vertex_data_property_map.h>

namespace CGAL {
namespace DDT {

namespace Impl {

template<unsigned int N, typename T, typename TileIndexPmap, typename Derived>
struct Triangulation_traits_d
{
    typedef T                                                      Triangulation;
    typedef typename Triangulation::Geom_traits                    Geom_traits;
    typedef typename Geom_traits::Point_d                          Point;
    typedef TileIndexPmap                                          Tile_index_pmap;
    typedef typename Tile_index_pmap::value_type                   Tile_index;
    typedef typename Triangulation::Triangulation_ds               TDS;
    typedef CGAL::DDT::Bbox<N, double, Point>                      Bbox;

    typedef typename TDS::Vertex_const_iterator                    Vertex_index;
    typedef typename TDS::Full_cell_const_iterator                 Cell_index;
    typedef CGAL::DDT::Facet_index<N, Cell_index>                  Facet_index;
    typedef CGAL::Random_points_in_cube_d<Point>                   Random_points_in_box;

    Tile_index_pmap tile_index_pmap;

private:
    typedef typename TDS::Vertex_iterator                          Vertex_iterator;
    typedef typename TDS::Full_cell_iterator                       Cell_iterator;

    Vertex_iterator remove_const_workaround(Vertex_index v) const {
        return Vertex_iterator(const_cast<typename Vertex_iterator::pointer>(v.operator->()));
    }

public:
    inline Tile_index vertex_id(const Triangulation& tri, Vertex_index v) const
    {
        return get(tile_index_pmap, remove_const_workaround(v));
    }
    inline int current_dimension(const Triangulation& tri) const
    {
        return tri.current_dimension();
    }
    inline int maximal_dimension(const Triangulation& tri) const
    {
        return tri.maximal_dimension();
    }
    inline std::size_t number_of_cells(const Triangulation& tri) const
    {
        return tri.number_of_full_cells();
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
        return tri.vertices_begin();
    }
    inline Vertex_index vertices_end(const Triangulation& tri) const
    {
        return tri.vertices_end();
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
        return tri.full_cells_begin();
    }
    inline Cell_index cells_end(const Triangulation& tri) const
    {
        return tri.full_cells_end();
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
        typedef Spatial_sort_traits_adapter_d<Geom_traits,Pmap> Search_traits;
        CGAL::spatial_sort(indices.begin(), indices.end(), Search_traits(make_property_map(points), tri.geom_traits()));
    }

    template<typename OutputIterator>
    inline OutputIterator incident_cells(const Triangulation& tri, Vertex_index v, OutputIterator out) const
    {
        return tri.incident_full_cells(v, out);
    }

    template<typename OutputIterator>
    OutputIterator adjacent_vertices(const Triangulation& tri, Vertex_index v, OutputIterator out) const
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

    Vertex_index locate_vertex(const Triangulation& tri, const Point& p, Vertex_index hint = Vertex_index()) const
    {
        if (hint == Vertex_index()) hint = tri.infinite_vertex();
        typename Triangulation::Locate_type  lt;
        typename Triangulation::Face f(tri.maximal_dimension());
        typename Triangulation::Facet ft;
        tri.locate(p, lt, f, ft, remove_const_workaround(hint));
        return (lt==Triangulation::ON_VERTEX) ? f.vertex(0) : vertices_end(tri);
    }

    std::pair<Vertex_index, bool> insert(Triangulation& tri, const Point& p, Tile_index id, Vertex_index hint = Vertex_index()) const
    {
        typename Triangulation::Locate_type lt;
        typename Triangulation::Face f(tri.maximal_dimension());
        typename Triangulation::Facet ft;
        if (hint == Vertex_index()) hint = tri.infinite_vertex();
        Cell_iterator c = tri.locate(p, lt, f, ft, remove_const_workaround(hint));
        if(lt == Triangulation::ON_VERTEX) {
            Vertex_iterator v = c->vertex(f.index(0));
            v->set_point(p);
            assert(id == vertex_id(tri, v));
            return std::make_pair(Vertex_index(v), false);
        }
        Vertex_iterator v = tri.insert(p, lt, f, ft, c);
        put(tile_index_pmap, v, id);
        return std::make_pair(Vertex_index(v), true);
    }

    inline void remove(Triangulation& tri, Vertex_index v) const
    {
        tri.remove(remove_const_workaround(v));
    }

    inline bool vertex_is_infinite(const Triangulation& tri, Vertex_index v) const
    {
        return tri.is_infinite(v);
    }

    inline bool facet_is_infinite(const Triangulation& tri, Facet_index f) const
    {
        for(int i = 0; i<=tri.current_dimension(); ++i)
            if(i!=f.index_of_covertex() && tri.is_infinite(vertex(tri, f.cell(), i)))
                return true;
        return false;
    }

    inline bool cell_is_infinite(const Triangulation& tri, Cell_index c) const
    {
        for(int i = 0; i<=tri.current_dimension(); ++i)
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

    bool are_cells_equal(const Triangulation& t1, Cell_index c1, const Triangulation& t2, Cell_index c2) const
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
        return vertex(tri, n, c->mirror_index(f.index_of_covertex()));
    }

    inline Cell_index cell(const Triangulation& tri, Facet_index f) const
    {
        return f.cell();
    }

    inline Cell_index cell(const Triangulation& tri, Vertex_index v) const
    {
        return v->full_cell();
    }

    Facet_index mirror_facet(const Triangulation& tri, Facet_index f) const
    {
        Cell_index c = f.cell();
        Cell_index n = c->neighbor(f.index_of_covertex());
        return facet(tri, n, c->mirror_index(f.index_of_covertex()));
    }

    inline int mirror_index(const Triangulation& tri, Facet_index f) const
    {
        return mirror_index(f.cell(), f.index_of_covertex());
    }

    inline int mirror_index(const Triangulation& tri, Cell_index c, int i) const
    {
        return c->mirror_index(i);
    }

    inline Cell_index neighbor(const Triangulation& tri, Cell_index c, int i) const
    {
        return c->neighbor(i);
    }

    inline Facet_index facet(const Triangulation& tri, Cell_index c, int i) const
    {
        return static_cast<const Derived*>(this)->facet(tri, c, i);
    }

    inline bool is_valid(const Triangulation& tri, bool verbose = false, int level = 0) const
    {
        return tri.is_valid(verbose, level);
    }

    inline int dimension() const {
        return static_cast<const Derived*>(this)->dimension();
    }

    Bbox bbox(const Point& p) const {
        int d = dimension();
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

    static inline Bbox bbox(unsigned int d, double range) {
      return Bbox(d, range);
    }

    static inline Bbox bbox(unsigned int d) {
      return Bbox(d);
    }

    inline bool less_coordinate(const Point& p, const Point& q, int i) const {
        return p[i] < q[i];
    }

    inline std::ostream& write(std::ostream& out, const Triangulation& tri) const { return out << tri; }
    inline std::istream& read(std::istream& in, Triangulation& tri) const { return in >> tri; }
    inline Triangulation triangulation() const
    {
        return Triangulation(dimension());
    }
};

}

/// \ingroup PkgDDTTraitsClasses
/// D dimensional triangulation traits, where D is specified dynamically at runtime.
/// \cgalModels TriangulationTraits
template<typename T, typename TileIndexPmap = Vertex_data_property_map<T>, unsigned int N = 0>
class Triangulation_traits_d : public Impl::Triangulation_traits_d<0,T,TileIndexPmap,Triangulation_traits_d<T,TileIndexPmap>>
{
    int dim_;
    typedef Impl::Triangulation_traits_d<0,T,TileIndexPmap,Triangulation_traits_d<T,TileIndexPmap>> Base;

public:
    typedef typename Base::Cell_index Cell_index;
    typedef typename Base::Facet_index Facet_index;
    typedef typename Base::Triangulation Triangulation;
    static constexpr int D = 0;
    Triangulation_traits_d(int d) : dim_(d)
    {
        assert(d >= 2);
    }
    inline int dimension() const {
        return dim_;
    }
    inline Facet_index facet(const Triangulation& tri, Cell_index c, int i) const
    {
        return Facet_index(dimension(), c, i);
    }
};

/// \ingroup PkgDDTTraitsClasses
/// D dimensional triangulation traits, where D is specified statically at compile-time.
/// \cgalModels TriangulationTraits
template<typename T, typename TileIndexPmap = Vertex_data_property_map<T>>
class Triangulation_traits : public Impl::Triangulation_traits_d<T::Maximal_dimension::value,T,TileIndexPmap,Triangulation_traits<T,TileIndexPmap>>
{
    typedef Impl::Triangulation_traits_d<T::Maximal_dimension::value,T,TileIndexPmap,Triangulation_traits<T,TileIndexPmap>> Base;

public:
    typedef typename Base::Cell_index Cell_index;
    typedef typename Base::Facet_index Facet_index;
    typedef typename Base::Triangulation Triangulation;
    static constexpr int D = T::Maximal_dimension::value;
    Triangulation_traits(int d = 0)
    {
        assert(d==0 || d==D);
    }
    inline constexpr int dimension() const { return D; }
    inline Facet_index facet(const Triangulation& tri, Cell_index c, int i) const
    {
        return Facet_index(c, i);
    }
};

}
}

#endif // CGAL_DDT_TRIANGULATION_TRAITS_D_H
