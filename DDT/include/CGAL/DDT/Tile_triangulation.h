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
#include <CGAL/assertions.h>

namespace CGAL {
namespace DDT {

struct Statistics {
    std::size_t number_of_finite_vertices;
    std::size_t number_of_finite_facets;
    std::size_t number_of_finite_cells;
    std::size_t number_of_facets;
    std::size_t number_of_cells;
    bool valid;

    Statistics() :
        number_of_finite_vertices(0),
        number_of_finite_facets  (0),
        number_of_finite_cells   (0),
        number_of_facets         (0),
        number_of_cells          (0),
        valid                    (true)
    {}

    Statistics operator+(const Statistics& stats) const {
        CGAL_assertion(valid && stats.valid);
        Statistics res;
        res.number_of_finite_vertices = number_of_finite_vertices + stats.number_of_finite_vertices;
        res.number_of_finite_facets   = number_of_finite_facets   + stats.number_of_finite_facets;
        res.number_of_finite_cells    = number_of_finite_cells    + stats.number_of_finite_cells;
        res.number_of_facets          = number_of_facets          + stats.number_of_facets;
        res.number_of_cells           = number_of_cells           + stats.number_of_cells;
        return res;
    }
};

std::ostream& operator<<(std::ostream& out, const Statistics& stats)
{
    return out << "{ \"finite_vertices\": " << stats.number_of_finite_vertices
        << ", \"finite_facets\": " << stats.number_of_finite_facets
        << ", \"finite_cells\": " << stats.number_of_finite_cells
        << ", \"facets\": " << stats.number_of_facets
        << ", \"cells\": " << stats.number_of_cells
        << ", \"valid\": " << stats.valid
        << " }";
}

std::istream& operator>>(std::istream& in, Statistics& stats)
{
    std::string s;
    return in
        >> s /* { */ >> s /* \"finite_vertices\": */ >> stats.number_of_finite_vertices
        >> s /* , */ >> s /* \"finite_facets\":   */ >> stats.number_of_finite_facets
        >> s /* , */ >> s /* \"finite_cells\":    */ >> stats.number_of_finite_cells
        >> s /* , */ >> s /* \"facets\":          */ >> stats.number_of_facets
        >> s /* , */ >> s /* \"cells\":           */ >> stats.number_of_cells
        >> s /* , */ >> s /* \"valid\":           */ >> stats.valid
        >> s /* } */;
}

std::string to_string(const Statistics& stats) {
    std::ostringstream oss;
    oss << stats;
    return oss.str();
}

// \tparam T is a model of the Triangulation concept
// \tparam Selector is a template for a model of the Selector concept (defaults to Median_selector)
// The Tile_triangulation stores a local Delaunay triangulation.
// The main id of a simplex is defined by the selector
template<
    class T,
    class TileIndexProperty,
    template <class> class Selector = Median_selector>
class Tile_triangulation
{
public:
    typedef T           Triangulation;
    typedef Triangulation_traits<T>                                  Traits;
    typedef TileIndexProperty                                        Tile_index_property;
    typedef typename Tile_index_property::value_type                 Tile_index;
    typedef typename Traits::Point                   Point;
    typedef typename Traits::Point_const_reference   Point_const_reference;
    typedef typename Traits::Vertex_index            Vertex_index;
    typedef typename Traits::Facet_index             Facet_index;
    typedef typename Traits::Cell_index              Cell_index;

    /// constructor
    Tile_triangulation(Tile_index id, int dimension, Tile_index_property index_map)
        : id_(id),
          tri_(Traits::triangulation(dimension)),
          tile_indices(index_map),
          statistics_()
    {}

    inline Triangulation& triangulation() { return tri_; }
    inline const Triangulation& triangulation() const { return tri_; }

    inline Tile_index id() const { return id_; }
    inline Tile_index& id() { return id_; }

    inline int maximal_dimension() const { return Traits::maximal_dimension(tri_); }
    inline int current_dimension() const { return Traits::current_dimension(tri_); }

    inline Cell_index cells_begin() const { return Traits::cells_begin(tri_); }
    inline Cell_index cells_end  () const { return Traits::cells_end  (tri_); }

    inline Vertex_index vertices_begin() const { return Traits::vertices_begin(tri_); }
    inline Vertex_index vertices_end  () const { return Traits::vertices_end  (tri_); }

    inline Facet_index  facets_begin()  const { return Traits::facets_begin(tri_); }
    inline Facet_index  facets_end  ()  const { return Traits::facets_end  (tri_); }

    inline std::size_t number_of_vertices() const { return Traits::number_of_vertices(tri_); }
    inline std::size_t number_of_cells   () const { return Traits::number_of_cells   (tri_); }

    inline std::size_t number_of_main_facets  () const { return statistics_.number_of_facets;   }
    inline std::size_t number_of_main_cells   () const { return statistics_.number_of_cells;    }
    inline std::size_t number_of_main_finite_vertices() const { return statistics_.number_of_finite_vertices; }
    inline std::size_t number_of_main_finite_facets  () const { return statistics_.number_of_finite_facets;   }
    inline std::size_t number_of_main_finite_cells   () const { return statistics_.number_of_finite_cells;    }

    inline Tile_index vertex_id(Vertex_index v) const {
        CGAL_assertion(!vertex_is_infinite(v));
        return get(tile_indices, std::make_pair(std::ref(tri_), v));
    }

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
        Cell_index c = cell_of_facet(f);
        int D = current_dimension();
        for(int i=0; i<=D; ++i) {
            if (i == cid) continue;
            Vertex_index v = vertex(c, i);
            if (!vertex_is_infinite(v)) selector.insert(vertex_id(v));
        }
        return selector.select();
    }

    inline void clear() { if (!statistics_.valid) finalize(); Traits::clear(tri_); }
    inline std::pair<Vertex_index, bool> insert(Point_const_reference p, Tile_index id, Vertex_index v = Vertex_index()) {
        statistics_.valid = false;
        auto inserted = Traits::insert(tri_, p, v);
        if(inserted.second) {
            if constexpr (std::is_convertible_v<typename Tile_index_property::category, boost::writable_property_map_tag>)
                put(tile_indices, std::make_pair(std::ref(tri_), inserted.first), id);
            else
                CGAL_assertion(get(tile_indices, std::make_pair(std::ref(tri_), inserted.first)) == id);
        }
        return inserted;
    }

    inline void remove(Vertex_index v) {
        statistics_.valid = false;
        Traits::remove(tri_, v);
    }

    inline void spatial_sort(std::vector<std::size_t>& indices, const std::vector<Point>& points) const { Traits::spatial_sort(tri_, indices, points); }

    /// \name Infinity tests
    /// @{
    inline bool vertex_is_infinite(Vertex_index v) const { return Traits::vertex_is_infinite(tri_, v); }
    inline bool facet_is_infinite (Facet_index  f) const { return Traits::facet_is_infinite (tri_, f); }
    inline bool cell_is_infinite  (Cell_index   c) const { return Traits::cell_is_infinite  (tri_, c); }
    /// @}

    /// \name Validity tests
    /// @{
    /// A simplex of a tile triangulation is valid if it is a local representative of the corresponding simplex in the overall triangulation

    /// A vertex is valid if it is finite
    inline bool vertex_is_valid(Vertex_index v) const { return !vertex_is_infinite(v); }
    /// A facet is valid if at least one of the following vertices is finite and local : its covertex, its mirror vertex and its incident vertices
    inline bool facet_is_valid(Facet_index f) const { return !cell_is_foreign(cell_of_facet(f)) || !vertex_is_foreign(mirror_vertex(f)); }
    /// A cell is valid if at least one of its incident vertices are finite and local
    inline bool cell_is_valid(Cell_index c) const { return !cell_is_foreign(c); }
    /// @}

    /// \name Vertex functions
    /// @{
    template<typename OutputIterator>
    inline OutputIterator adjacent_vertices(Vertex_index v, OutputIterator out) const { return Traits::adjacent_vertices(tri_, v, out); }
    template<typename OutputIterator>
    inline OutputIterator incident_cells(Vertex_index v, OutputIterator out) const { return Traits::incident_cells(tri_, v, out); }
    inline Vertex_index infinite_vertex() const { return Traits::infinite_vertex(tri_); }
    inline Point_const_reference point(Vertex_index v) const { return Traits::point(tri_, v); }
    /// @}

    /// \name Facet functions
    /// @{
    inline int index_of_covertex(Facet_index f) const { return Traits::index_of_covertex(tri_, f); }
    inline Vertex_index covertex(Facet_index f) const { return Traits::covertex(tri_, f); }
    inline Vertex_index mirror_vertex(Facet_index f) const { return Traits::mirror_vertex(tri_, f); }
    inline Cell_index cell_of_facet(Facet_index f) const { return Traits::cell_of_facet(tri_, f); }
    inline Cell_index cell_of_vertex(Vertex_index v) const { return Traits::cell_of_vertex(tri_, v); }
    inline Facet_index mirror_facet(Facet_index f) const { return Traits::mirror_facet(tri_, f); }
    inline int mirror_index(Facet_index f) const { return Traits::mirror_index(tri_, f); }
    /// @}

    /// \name Cell functions
    /// @{
    inline Vertex_index vertex(Cell_index c, int i) const { return Traits::vertex(tri_, c, i); }
    inline Facet_index facet(Cell_index c, int i) const { return Traits::facet(tri_, c, i); }
    inline int mirror_index(Cell_index c, int i) const { return Traits::mirror_index(tri_, c, i); }
    inline Cell_index neighbor(Cell_index c, int i) const { return Traits::neighbor(tri_, c, i); }
    /// @}

    /// \name Tile_triangulation locality tests
    /// @{
    /// A finite vertex is local if its tile id matches the id of the tile triangulation (tile.vertex_id(vertex) == tile.id()), otherwise, it is foreign
    /// Simplices may be local, mixed or foreign if respectively all, some or none of their finite incident vertices are local.

    /// checks if a finite vertex is local : tile.vertex_id(vertex) == tile.id()
    /// precondition : the vertex is finite.
    inline bool vertex_is_local(Vertex_index v) const { CGAL_assertion(!vertex_is_infinite(v)); return vertex_id(v) == id(); }

    /// checks if a finite vertex is foreign : tile.vertex_id(vertex) != tile.id()
    /// precondition : the vertex is finite.
    inline bool vertex_is_foreign(Vertex_index v) const { return !vertex_is_local(v); }

    /// checks if a facet is local : all its finite vertices are local
    template<typename F>
    bool facet_is_local(F f) const
    {
        int icv = index_of_covertex(f);
        auto c = cell_of_facet(f);
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
        auto c = cell_of_facet(f);
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
        auto c = cell_of_facet(f);
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

    /// removes a finite vertex if it is foreign and if all its adjacent vertices are foreign
    /// returns whether simplification occured
    bool simplify(Vertex_index v)
    {
        CGAL_assertion(!vertex_is_infinite(v));
        if (!vertex_is_foreign(v)) return false;
        std::vector<Vertex_index> adj;
        adjacent_vertices(v, std::back_inserter(adj));
        for (Vertex_index a : adj)
            if(!vertex_is_infinite(a) && vertex_is_local(a))
                return false;
        remove(v);
        return true;
    }

    /// collects at most 2*D vertices which points define the bounding box of the local tile vertices
    template <class VertexContainer>
    void get_axis_extreme_points(VertexContainer& out) const
    {
        std::vector<Vertex_index> vertices;
        int D = maximal_dimension();
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
                Point_const_reference p = point(v);
                for(int i=0; i<D; ++i)
                {
                    if(less_coordinate(p, point(vertices[i  ]), i)) vertices[i  ] = v;
                    if(less_coordinate(point(vertices[i+D]), p, i)) vertices[i+D] = v;
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

    /// collects (vertex,id) pairs listing finite vertices that are possibly newly adjacent to vertices of a foreign tile (id),
    /// after the insertion of the inserted vertices, as required by the star splaying algorithm.
    template <class VertexContainer, typename VertexIndexSet>
    void get_finite_neighbors(const VertexContainer& inserted, std::map<Tile_index, VertexIndexSet>& out) const
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

    /// inserts a range of received points with tile ids.
    /// reports the set of all inserted points by default.
    /// If report_vertices_with_mixed_stars_only is true, then only the new vertices with mixed stars are reported.
    /// foreign vertices of the tile triangulation are automatically simplified if their star is foreign as well.
    /// @returns the number of inserted points (not counting the number of simplified points and the insertion of already inserted points)
    template <typename PointSet>
    int insert(const PointSet& input, std::set<Vertex_index>& inserted, bool report_vertices_with_mixed_stars_only=false)
    {
        statistics_.valid = false;
        // retrieve the input points and ids in separate vectors
        // compute the axis-extreme points on the way
        std::vector<Point> points;
        std::vector<Tile_index> ids;
        std::vector<std::size_t> indices;
        std::size_t index=0;
        points.reserve(input.size());
        ids.reserve(input.size());
        indices.reserve(input.size());

        for(auto it = input.begin(); it != input.end(); ++it, ++index)
        {
            Point p = input.point(it);
            Tile_index i = input.point_id(it);
            points.push_back(p);
            ids.push_back(i);
            indices.push_back(index);
        }

        // sort spatially the points
        spatial_sort(indices, points);

        // insert the points in the sorted order
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

    bool are_vertices_equal(Vertex_index v, const Tile_triangulation& t, Vertex_index tv) const
    {
        return Traits::are_vertices_equal(tri_, v, t.tri_, tv);
    }

    bool are_facets_equal(Facet_index f, const Tile_triangulation& t, Facet_index tf) const
    {
        return Traits::are_facets_equal(tri_, f, t.tri_, tf);
    }

    bool are_cells_equal(Cell_index c, const Tile_triangulation& t, Cell_index tc) const
    {
        return Traits::are_cells_equal(tri_, c, t.tri_, tc);
    }


    Vertex_index locate_vertex(Point_const_reference p, Vertex_index hint = Vertex_index()) const
    {
        return Traits::locate_vertex(tri_, p, hint);
    }

    Vertex_index relocate_vertex(const Tile_triangulation& t, Vertex_index v, Vertex_index hint = Vertex_index()) const
    {
        if(t.vertex_is_infinite(v)) return infinite_vertex();
        return locate_vertex(t.point(v), hint);
    }

    Facet_index relocate_facet(const Tile_triangulation& t, Facet_index f) const
    {
        CGAL_assertion(t.facet_is_valid(f));
        Cell_index c = t.cell_of_facet(f);
        if(t.cell_is_foreign(c)) {
            Facet_index g = t.mirror_facet(f);
            Facet_index h = relocate_facet(t, g);
            CGAL_assertion(h != facets_end());
            return mirror_facet(h);
        }
        int icv = t.index_of_covertex(f);
        int iv  = !icv;
        // v is a relocated vertex on the facet (!=covertex)
        Vertex_index v = relocate_vertex(t, t.vertex(c, iv));
        CGAL_assertion(v!=vertices_end());
        if (v == vertices_end()) return facets_end();
        std::vector<Cell_index> cells;
        incident_cells(v, std::back_inserter(cells));
        int D = maximal_dimension();
        for(Cell_index c2: cells)
        {
            for(int icv2 =0; icv2<=D; ++icv2)
            {
                Facet_index f2 = t.facet(c2, icv2);
                if (are_facets_equal(f2, t, f))
                    return f2;
            }
        }
        CGAL_assertion(false);
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

    void finalize() const
    {
        if (statistics_.valid) return;
        statistics_.number_of_finite_vertices = 0;
        statistics_.number_of_finite_facets = 0;
        statistics_.number_of_finite_cells = 0;
        statistics_.number_of_facets = 0;
        statistics_.number_of_cells = 0;
        statistics_.valid = true;

        for(Vertex_index v = vertices_begin(); v != vertices_end(); ++v)
        {
            if(vertex_is_main(v))
                ++statistics_.number_of_finite_vertices;
        }

        int D = current_dimension();
        for(Cell_index c = cells_begin(); c != cells_end(); ++c)
        {
            int finite= 1;
            int lower = 0;
            int equal = 0;
            for(int i=0; i<=D; ++i) {
                Vertex_index v = vertex(c, i);
                if(vertex_is_infinite(v)) {
                    finite = 0;
                } else {
                    Tile_index vid = vertex_id(v);
                    if (vid < id()) ++lower;
                    else if (vid == id()) ++equal;
                }
            }
#if CGAL_DDT_DEBUG_SELECTOR
            int cells = 0;
            int finite_cells = 0;
            int facets = 0;
            int finite_facets = 0;

            selector.cell_statistics(lower, equal, D, finite,
                cells,
                finite_cells,
                facets,
                finite_facets
            );
            int cells2 = cell_is_main(c);
            int finite_cells2 = cell_is_main(c) && !cell_is_infinite(c);
            int facets2 = 0;
            int finite_facets2 = 0;
            for(int i=0; i<=D; ++i) {
                Facet_index f = facet(c, i);
                if (facet_is_main(f)) {
                    ++facets2;
                    if(!facet_is_infinite(f))
                        ++finite_facets2;
                }
            }
            CGAL_assertion(cells == cells2);
            CGAL_assertion(finite_cells == finite_cells2);
            CGAL_assertion(facets == facets2);
            CGAL_assertion(finite_facets == finite_facets2);

            statistics_.number_of_cells += cells ;
            statistics_.number_of_finite_cells +=finite_cells;
            statistics_.number_of_facets += facets;
            statistics_.number_of_finite_facets += finite_facets;
#else
            selector.cell_statistics(lower, equal, D, finite,
                statistics_.number_of_cells,
                statistics_.number_of_finite_cells,
                statistics_.number_of_facets,
                statistics_.number_of_finite_facets
            );
#endif
        }
    }


    inline bool is_valid(bool verbose = false, int level = 0) const
    {
      return Traits::is_valid(tri_, verbose, level);
    }

    Statistics& statistics() { if (!statistics_.valid) finalize(); return statistics_; }
    const Statistics& statistics() const { if (!statistics_.valid) finalize(); return statistics_; }

private:
    Tile_index id_;
    Triangulation tri_;
    Tile_index_property tile_indices;

    mutable Selector<Tile_index> selector;
    mutable Statistics statistics_;
};

template<class T, class Pmap, template <class> class Selector>
std::ostream& operator<<(std::ostream& out, const Tile_triangulation<T, Pmap, Selector>& t)
{
    return CGAL::DDT::Triangulation_traits<T>::write(out, t.triangulation());
}

template<class T, class Pmap, template <class> class Selector>
std::istream& operator>>(std::istream& in, Tile_triangulation<T, Pmap, Selector>& t)
{
    return CGAL::DDT::Triangulation_traits<T>::read(in, t.triangulation());
}

template<class T, class Pmap, template <class> class Selector>
std::ostream& write_summary(std::ostream& out, const Tile_triangulation<T, Pmap, Selector>& t)
{
    return out << t.statistics();
}

}
}

#endif // CGAL_DDT_TILE_TRIANGULATION_H
