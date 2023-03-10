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

#ifndef CGAL_DDT_TILE_TRIANGULATION_H
#define CGAL_DDT_TILE_TRIANGULATION_H

#include <CGAL/DDT/selector/Median_selector.h>

#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <assert.h>

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTClasses
/// \tparam T is a model of the TriangulationTraits concept
/// \tparam Selector is a template for a model of the Selector concept (defaults to Median_selector)
/// The Tile_triangulation stores a local Delaunay triangulation.
/// The main id of a simplex is defined by the selector
template<class T, template <class> class Selector = Median_selector>
class Tile_triangulation
{
public:
    typedef T                                        Traits;
    typedef typename Traits::Tile_index              Tile_index;
    typedef typename Traits::Info                    Info;
    typedef typename Traits::Point                   Point;
    typedef typename Traits::Bbox                    Bbox;
    typedef typename Traits::Delaunay_triangulation  Delaunay_triangulation;
    typedef typename Traits::Vertex_index            Vertex_index;
    typedef typename Traits::Facet_index             Facet_index;
    typedef typename Traits::Cell_index              Cell_index;

    /// constructor
    Tile_triangulation(Tile_index id, Traits t)
        : traits(t),
          id_(id),
          dt_(traits.triangulation()),
          number_of_main_finite_vertices_(0),
          number_of_main_finite_facets_(0),
          number_of_main_finite_cells_(0),
          number_of_main_facets_(0),
          number_of_main_cells_(0)
    {}

    inline Delaunay_triangulation& triangulation() { return dt_; }
    inline const Delaunay_triangulation& triangulation() const { return dt_; }

    inline Tile_index id() const { return id_; }

    inline int maximal_dimension() const { return traits.maximal_dimension(dt_); }
    inline int current_dimension() const { return traits.current_dimension(dt_); }

    inline Cell_index cells_begin() const { return traits.cells_begin(dt_); }
    inline Cell_index cells_end  () const { return traits.cells_end  (dt_); }

    inline Vertex_index vertices_begin() const { return traits.vertices_begin(dt_); }
    inline Vertex_index vertices_end  () const { return traits.vertices_end  (dt_); }

    inline Facet_index  facets_begin()  const { return traits.facets_begin(dt_); }
    inline Facet_index  facets_end  ()  const { return traits.facets_end  (dt_); }

    inline std::size_t number_of_vertices() const { return traits.number_of_vertices(dt_); }
    inline std::size_t number_of_cells   () const { return traits.number_of_cells   (dt_); }

    inline std::size_t number_of_main_facets  () const { return number_of_main_facets_;   }
    inline std::size_t number_of_main_cells   () const { return number_of_main_cells_;    }
    inline std::size_t number_of_main_finite_vertices() const { return number_of_main_finite_vertices_; }
    inline std::size_t number_of_main_finite_facets  () const { return number_of_main_finite_facets_;   }
    inline std::size_t number_of_main_finite_cells   () const { return number_of_main_finite_cells_;    }

    inline Info& info(Vertex_index v) const { assert(!vertex_is_infinite(v)); return traits.info(v); }
    inline Tile_index vertex_id(Vertex_index v) const { assert(!vertex_is_infinite(v)); return traits.id(v); }

    Tile_index cell_id(Cell_index c) const
    {
        selector.clear();
        int D = current_dimension();
        for(int i=0; i<=D; ++i) {
            Vertex_index v = vertex(c, i);
            if(!vertex_is_infinite(v)) selector.insert(vertex_id(v));
        }
        return selector.select();
    }

    Tile_index facet_id(const Facet_index& f) const
    {
        selector.clear();
        int cid = index_of_covertex(f);
        Cell_index c = cell(f);
        int D = current_dimension();
        for(int i=0; i<=D; ++i) {
            if (i == cid) continue;
            Vertex_index v = vertex(c, i);
            if (!vertex_is_infinite(v)) selector.insert(vertex_id(v));
        }
        return selector.select();
    }

    inline void clear() { traits.clear(dt_); }
    inline std::pair<Vertex_index, bool> insert(const Point& p, Tile_index id, Vertex_index v = Vertex_index()) { return traits.insert(dt_, p, id, v); }
    inline void remove(Vertex_index v) { traits.remove(dt_, v); }

    inline void spatial_sort(std::vector<std::size_t>& indices, const std::vector<Point>& points) const { traits.spatial_sort(dt_, indices, points); }

    /// \name Infinity tests
    /// @{
    inline bool vertex_is_infinite(Vertex_index v) const { return traits.vertex_is_infinite(dt_, v); }
    inline bool facet_is_infinite (Facet_index  f) const { return traits.facet_is_infinite (dt_, f); }
    inline bool cell_is_infinite  (Cell_index   c) const { return traits.cell_is_infinite  (dt_, c); }
    /// @}

    /// \name Validity tests
    /// @{
    /// A simplex of a tile triangulation is valid if it is a local representative of the corresponding simplex in the overall triangulation

    /// A vertex is valid if it is finite
    inline bool vertex_is_valid(Vertex_index v) const { return !vertex_is_infinite(v); }
    /// A facet is valid if at least one of the following vertices is finite and local : its covertex, its mirror vertex and its incident vertices
    inline bool facet_is_valid(Facet_index f) const { return !cell_is_foreign(cell(f)) || !vertex_is_foreign(mirror_vertex(f)); }
    /// A cell is valid if at least one of its incident vertices are finite and local
    inline bool cell_is_valid(Cell_index c) const { return !cell_is_foreign(c); }
    /// @}

    /// \name Vertex functions
    /// @{
    template<typename OutputIterator>
    inline OutputIterator adjacent_vertices(Vertex_index v, OutputIterator out) const { return traits.adjacent_vertices(dt_, v, out); }
    template<typename OutputIterator>
    inline OutputIterator incident_cells(Vertex_index v, OutputIterator out) const { return traits.incident_cells(dt_, v, out); }
    inline Vertex_index infinite_vertex() const { return traits.infinite_vertex(dt_); }
    inline const Point& point(Vertex_index v) const { return traits.point(dt_, v); }
    inline Bbox bbox(Vertex_index v) const { return traits.bbox(point(v)); }
    inline double approximate_cartesian_coordinate(Vertex_index v, int i) const { return traits.approximate_cartesian_coordinate(point(v), i); }
    /// @}

    /// \name Facet functions
    /// @{
    inline int index_of_covertex(Facet_index f) const { return traits.index_of_covertex(dt_, f); }
    inline Vertex_index covertex(Facet_index f) const { return traits.covertex(dt_, f); }
    inline Vertex_index mirror_vertex(Facet_index f) const { return traits.mirror_vertex(dt_, f); }
    inline Cell_index cell(Facet_index f) const { return traits.cell(dt_, f); }
    inline Cell_index cell(Vertex_index v) const { return traits.cell(dt_, v); }
    inline Facet_index mirror_facet(Facet_index f) const { return traits.mirror_facet(dt_, f); }
    inline int mirror_index(Facet_index f) const { return traits.mirror_index(dt_, f); }
    /// @}

    /// \name Cell functions
    /// @{
    inline Vertex_index vertex(Cell_index c, int i) const { return traits.vertex(dt_, c, i); }
    inline Facet_index facet(Cell_index c, int i) const { return traits.facet(dt_, c, i); }
    inline int mirror_index(Cell_index c, int i) const { return traits.mirror_index(dt_, c, i); }
    inline Cell_index neighbor(Cell_index c, int i) const { return traits.neighbor(dt_, c, i); }
    /// @}

    /// \name Tile_triangulation locality tests
    /// @{
    /// A finite vertex is local if its tile id matches the id of the tile triangulation (tile.vertex_id(vertex) == tile.id()), otherwise, it is foreign
    /// Simplices may be local, mixed or foreign if respectively all, some or none of their finite incident vertices are local.

    /// checks if a finite vertex is local : tile.vertex_id(vertex) == tile.id()
    /// precondition : the vertex is finite.
    inline bool vertex_is_local(Vertex_index v) const { assert(!vertex_is_infinite(v)); return vertex_id(v) == id(); }

    /// checks if a finite vertex is foreign : tile.vertex_id(vertex) != tile.id()
    /// precondition : the vertex is finite.
    inline bool vertex_is_foreign(Vertex_index v) const { return !vertex_is_local(v); }

    /// checks if a facet is local : all its finite vertices are local
    template<typename F>
    bool facet_is_local(F f) const
    {
        int icv = index_of_covertex(f);
        auto c = cell(f);
        for(int i=0; i<=current_dimension(); ++i)
        {
            if (i == icv) continue;
            auto v = vertex(c,i);
            if ( !vertex_is_infinite(v) && vertex_is_foreign(v) ) return false;
        }
        return true;
    }

    /// checks if a facet is mixed : some of its finite vertices are local and some are foreign
    template<typename F>
    bool facet_is_mixed(F f) const
    {
        int icv = index_of_covertex(f);
        auto c = cell(f);
        bool local_found = false;
        bool foreign_found = false;
        for(int i=0; i <= current_dimension(); ++i)
        {
            if ( i == icv ) continue;
            auto v = vertex(c,i);
            if ( vertex_is_infinite(v) ) continue;
            if ( vertex_is_local(v) )
            {
                if (foreign_found) return true;
                local_found = true;
            }
            else
            {
                if (local_found) return true;
                foreign_found = true;
            }
        }
        return false;
    }

    /// checks if a facet is foreign : all its finite vertices are foreign
    template<typename F>
    bool facet_is_foreign(F f) const
    {
        int icv = index_of_covertex(f);
        auto c = cell(f);
        for(int i=0; i<=current_dimension(); ++i)
        {
            if ( i == icv ) continue;
            auto v = vertex(c,i);
            if ( !vertex_is_infinite(v) && vertex_is_local(v) ) return false;
        }
        return true;
    }

    /// checks if a cell is local : all its finite vertices are local
    template<typename C>
    bool cell_is_local(C c) const
    {
        for(int i=0; i<=current_dimension(); ++i)
        {
            auto v = vertex(c,i);
            if ( !vertex_is_infinite(v) && vertex_is_foreign(v) ) return false;
        }
        return true;
    }

    /// checks if a cell is mixed : some of its finite vertices are local and some are foreign
    template<typename C>
    bool cell_is_mixed(C c) const
    {
        bool local_found = false;
        bool foreign_found = false;
        for(int i=0; i <= current_dimension(); ++i)
        {
            auto v = vertex(c,i);
            if ( vertex_is_infinite(v) ) continue;
            if ( vertex_is_local(v) )
            {
                if (foreign_found) return true;
                local_found = true;
            }
            else
            {
                if (local_found) return true;
                foreign_found = true;
            }
        }
        return false;
    }

    /// checks if a cell is foreign : all its finite vertices are foreign
    template<typename C>
    bool cell_is_foreign(C c) const
    {
        for(int i=0; i<=current_dimension(); ++i)
        {
            auto v = vertex(c,i);
            if ( !vertex_is_infinite(v) && vertex_is_local(v) ) return false;
        }

        return true;
    }

    /// checks if the star of a vertex is mixed : all its finite vertices are local
    bool star_is_local(Vertex_index v) const {
        if (!vertex_is_infinite(v) && vertex_is_foreign(v)) return false;
        std::vector<Vertex_index> adj;
        adjacent_vertices(v, std::back_inserter(adj));
        for (Vertex_index a : adj)
            if(!vertex_is_infinite(a) && vertex_is_foreign(a))
                return false;
        return true;
    }

    /// checks if the star of a vertex is mixed : some of its finite vertices are local and some are foreign
    bool star_is_mixed(Vertex_index v) const {
        bool local_found = false;
        bool foreign_found = false;
        if ( vertex_is_infinite(v) ) {
            if ( vertex_is_local(v) )
                local_found = true;
            else
                foreign_found = true;
        }
        std::vector<Vertex_index> adj;
        adjacent_vertices(v, std::back_inserter(adj));
        for (Vertex_index a : adj) {
            if ( vertex_is_infinite(a) ) continue;
            if ( vertex_is_local(a) )
            {
                if (foreign_found) return true;
                local_found = true;
            }
            else
            {
                if (local_found) return true;
                foreign_found = true;
            }
        }
        return true;
    }

    /// checks if the star of a vertex is foreign : all its finite vertices are foreign
    bool star_is_foreign(Vertex_index v) const {
        if (!vertex_is_infinite(v) && vertex_is_local(v)) return false;
        std::vector<Vertex_index> adj;
        adjacent_vertices(v, std::back_inserter(adj));
        for (Vertex_index a : adj)
            if(!vertex_is_infinite(a) && vertex_is_local(a))
                return false;
        return true;
    }
    /// @}


    /// \name Main tests
    /// @{
    /// checks if a vertex is finite and local : tile.vertex_id(vertex) == tile.id()
    inline bool vertex_is_main(Vertex_index v) const { return !vertex_is_infinite(v) && vertex_id(v) == id(); }
    /// checks if a facet is main : tile.facet_id(facet) == tile.id()
    inline bool facet_is_main(Facet_index f) const { return facet_id(f) == id(); }
    /// checks if a cell is main : tile.cell_id(cell) == tile.id()
    inline bool cell_is_main(Cell_index c) const { return cell_id(c) == id(); }
    /// @}

    /// remove a finite vertex if it is foreign and if all its adjacent vertices are foreign
    /// returns whether simplification occured
    bool simplify(Vertex_index v)
    {
        assert(!vertex_is_infinite(v));
        if (!vertex_is_foreign(v)) return false;
        std::vector<Vertex_index> adj;
        adjacent_vertices(v, std::back_inserter(adj));
        for (Vertex_index a : adj)
            if(!vertex_is_infinite(a) && vertex_is_local(a))
                return false;
        remove(v);
        return true;
    }

    /// Collect at most 2*D vertices which points define the bounding box of the local tile vertices
    void get_axis_extreme_points(std::vector<Vertex_index>& out) const
    {
        std::vector<Vertex_index> vertices;
        int D = traits.dimension();
        vertices.reserve(2*D);
        Vertex_index v = vertices_begin();
        // first local point
        for(; v != vertices_end(); ++v)
        {
            if (!vertex_is_infinite(v) && vertex_is_local(v))
            {
                for(int i=0; i<2*D; ++i) vertices[i] = v;
                break;
            }
        }
        if(v == vertices_end()) return; // no local points
        // other local points
        for(; v != vertices_end(); ++v)
        {
            if (!vertex_is_infinite(v) && vertex_is_local(v))
            {
                const Point& p = point(v);
                for(int i=0; i<D; ++i)
                {
                    if(traits.less_coordinate(p, point(vertices[i  ]), i)) vertices[i  ] = v;
                    if(traits.less_coordinate(p, point(vertices[i+D]), i)) vertices[i+D] = v;
                }
            }
        }
        // remove duplicates (O(D^2) implementation, should we bother ?)
        for(int i=0; i<2*D; ++i)
        {
            int j = 0;
            for(; j<i; ++j) if(vertices[j]==vertices[i]) break;
            if(i==j) out.push_back(vertices[i]);
        }
    }

    /// Collect (vertex,id) pairs listing finite vertices that are possibly newly adjacent to vertices of a foreign tile (id),
    /// after the insertion of the inserted vertices, as required by the star splaying algorithm.
    void get_finite_neighbors(const std::set<Vertex_index>& inserted, std::map<Tile_index, std::set<Vertex_index>>& out) const
    {
        for(auto v : inserted) {
            if(vertex_is_infinite(v)) continue;
            Tile_index idv = vertex_id(v);
            std::vector<Vertex_index> vadj;
            adjacent_vertices(v, std::back_inserter(vadj));
            for (auto w : vadj) {
                if(vertex_is_infinite(w)) continue;
                Tile_index idw = vertex_id(w);
                if(idw != idv)
                {
                    // w is not new in the dt, insert it only if it is foreign and
                    /// @todo if it was not already adjacent to a vertex in tile idv
                    if (idv!=id()) out[idv].insert(w);
                    // v is new so it has no previous neighbors in the dt
                    if (idw!=id()) out[idw].insert(v);
                }
            }
        }
    }

    /// Insert a range of received points with tile ids.
    /// reports the set of all inserted points by default.
    /// If report_vertices_with_mixed_stars_only is true, then only the new vertices with mixed stars are reported.
    /// foreign vertices of the tile triangulation are automatically simplified if their star is foreign as well.
    /// @returns the number of inserted points (not counting the number of simplified points and the insertion of already inserted points)
    template <class PointTile_indexContainer>
    int insert(const PointTile_indexContainer& received, std::set<Vertex_index>& inserted, bool report_vertices_with_mixed_stars_only=false)
    {
        // retrieve the input points and ids in separate vectors
        // compute the axis-extreme points on the way
        std::vector<Point> points;
        std::vector<Tile_index> ids;
        std::vector<std::size_t> indices;
        std::size_t index=0;
        points.reserve(received.size());
        for(auto& r : received)
        {
            points.push_back(r.second);
            ids.push_back(r.first);
            indices.push_back(index++);
        }

        // sort spatially the points
        spatial_sort(indices, points);

        // insert the point with infos in the sorted order
        // check immediately for simplification
        Vertex_index v;
        int local_inserted_size = 0;
        for (std::size_t index : indices) {
          std::pair<Vertex_index,bool> p = insert(points[index], ids[index], v);
          if (!p.second) {
              // update the hint, but do not process as the point was already inserted
              v = p.first;
          } else if (!simplify(p.first)) {
              // update the hint and mark the unsimplified vertex for processing
              v = p.first;
              if (!(report_vertices_with_mixed_stars_only && star_is_local(v)))
                inserted.insert(v);
              else
                ++local_inserted_size;
          }
        }

        // simplify neighbors: collect vertices adjacent to newly inserted foreign points
        std::set<Vertex_index> adj;
        for (Vertex_index v : inserted)
            if(vertex_is_foreign(v))
                adjacent_vertices(v, std::inserter(adj, adj.begin()));

        for (Vertex_index v : adj)
            if(!vertex_is_infinite(v) && simplify(v))
                inserted.erase(v);

        return local_inserted_size + inserted.size();
    }

    void get_adjacency_graph_edges(std::set<Tile_index>& out_edges) const
    {
        for(auto cit = cells_begin(); cit != cells_end(); ++cit)
            if(cell_is_mixed(cit))
                for(int i=0; i<=current_dimension(); ++i)
                {
                    Vertex_index v = vertex(cit,i);
                    if(!vertex_is_infinite(v) && vertex_is_foreign(v))
                        out_edges.insert(vertex_id(v));
                }
    }

    bool are_vertices_equal(Vertex_index v, const Tile_triangulation& t, Vertex_index tv) const
    {
        return traits.are_vertices_equal(dt_, v, t.dt_, tv);
    }

    bool are_facets_equal(Facet_index f, const Tile_triangulation& t, Facet_index tf) const
    {
        return traits.are_facets_equal(dt_, f, t.dt_, tf);
    }

    bool are_cells_equal(Cell_index c, const Tile_triangulation& t, Cell_index tc) const
    {
        return traits.are_cells_equal(dt_, c, t.dt_, tc);
    }


    Vertex_index locate_vertex(const Point& p, Vertex_index hint = Vertex_index()) const
    {
        return traits.locate_vertex(dt_, p, hint);
    }

    Vertex_index relocate_vertex(const Tile_triangulation& t, Vertex_index v, Vertex_index hint = Vertex_index()) const
    {
        if(t.vertex_is_infinite(v)) return infinite_vertex();
        return locate_vertex(t.point(v), hint);
    }

    Facet_index relocate_facet(const Tile_triangulation& t, Facet_index f) const
    {
        assert(t.facet_is_valid(f));
        Cell_index c = t.cell(f);
        if(t.cell_is_foreign(c)) return mirror_facet(relocate_facet(t, t.mirror_facet(f)));
        assert(!t.cell_is_foreign(c));
        Cell_index d = relocate_cell(t, c);
        if (d == cells_end()) return facets_end();
        Vertex_index v = t.vertex(c, t.index_of_covertex(f));
        for(int i=0; i<=current_dimension(); ++i)
        {
            if(traits.are_vertices_equal(t.dt_, v, dt_, vertex(d, i)))
                return facet(d, i);
        }
        return facets_end();
    }

    Cell_index relocate_cell(const Tile_triangulation& t, Cell_index c) const
    {
        Vertex_index v = relocate_vertex(t, t.vertex(c, 0));
        if (v == vertices_end()) return cells_end();
        std::vector<Cell_index> cells;
        incident_cells(v, std::back_inserter(cells));
        for(Cell_index ic: cells)
            if(are_cells_equal(ic, t, c))
                return ic;
        return cells_end();
    }

    void finalize()
    {

        number_of_main_finite_vertices_ = 0;
        number_of_main_finite_facets_ = 0;
        number_of_main_finite_cells_ = 0;
        number_of_main_facets_ = 0;
        number_of_main_cells_ = 0;

        for(Vertex_index v = vertices_begin(); v != vertices_end(); ++v)
        {
            if(vertex_is_main(v))
                ++number_of_main_finite_vertices_;
        }
        for(Facet_index f = facets_begin(); f != facets_end(); ++f )
        {
            if(facet_is_main(f))
            {
                ++number_of_main_facets_;
                if(!facet_is_infinite(f))
                    ++number_of_main_finite_facets_;
            }
        }

        for(Cell_index c = cells_begin(); c != cells_end(); ++c)
        {
            if(cell_is_main(c))
            {
                ++number_of_main_cells_;
                if(!cell_is_infinite(c))
                    ++number_of_main_finite_cells_;
            }
        }
    }


    inline bool is_valid(bool verbose = false, int level = 0) const
    {
      return traits.is_valid(dt_, verbose, level);
    }

    const Traits& geom_traits() const { return traits; }

private:
    Traits traits;
    Tile_index id_;
    Delaunay_triangulation dt_;
    mutable Selector<Tile_index> selector;

    std::size_t number_of_main_finite_vertices_;
    std::size_t number_of_main_finite_facets_;
    std::size_t number_of_main_finite_cells_;
    std::size_t number_of_main_facets_;
    std::size_t number_of_main_cells_;
};

template<class T, template <class> class Selector>
std::ostream& operator<<(std::ostream& out, const Tile_triangulation<T, Selector>& tt)
{
    return tt.geom_traits().write(out, tt.triangulation());
}

template<class T, template <class> class Selector>
std::istream& operator>>(std::istream& in, Tile_triangulation<T, Selector>& tt)
{
    return tt.geom_traits().read(in, tt.triangulation());
}

}
}

#endif // CGAL_DDT_TILE_TRIANGULATION_H
