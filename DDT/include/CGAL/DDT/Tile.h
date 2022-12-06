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

#ifndef CGAL_DDT_TILE_H
#define CGAL_DDT_TILE_H

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
/// The Tile stores a local Delaunay triangulation.
/// The main id of a simplex is defined by the selector
template<class T, template <class> class Selector = Median_selector>
class Tile
{
public:
    typedef T                                        Traits;
    typedef typename Traits::Id                      Id;
    typedef typename Traits::Info                    Info;
    typedef typename Traits::Point                   Point;
    typedef typename Traits::Delaunay_triangulation  Delaunay_triangulation;
    typedef typename Traits::Vertex_handle           Vertex_handle;
    typedef typename Traits::Vertex_iterator         Vertex_iterator;
    typedef typename Traits::Vertex_const_handle     Vertex_const_handle;
    typedef typename Traits::Vertex_const_iterator   Vertex_const_iterator;
    typedef typename Traits::Cell_handle             Cell_handle;
    typedef typename Traits::Cell_const_handle       Cell_const_handle;
    typedef typename Traits::Cell_const_iterator     Cell_const_iterator;
    typedef typename Traits::Facet_const_iterator    Facet_const_iterator;
    typedef typename Traits::Facet_const_iterator    Facet_const_handle;
    typedef std::pair<Id,Vertex_const_handle>        Vertex_const_handle_and_id;
    typedef std::pair<Id,Point>                      Point_id;
    typedef std::vector<Point_id>                    Points;
    typedef std::map<Id, Points>                     Points_map;


    /// constructor
    Tile(Id id, Traits t)
        : traits(t),
          id_(id),
          dt_(traits.triangulation()),
          number_of_main_finite_vertices_(0),
          number_of_main_finite_facets_(0),
          number_of_main_finite_cells_(0),
          number_of_main_facets_(0),
          number_of_main_cells_(0),
          number_of_extreme_points_received(0),
          in_use(false)
    {}

    inline Delaunay_triangulation& triangulation() { return dt_; }
    inline const Delaunay_triangulation& triangulation() const { return dt_; }

    inline Id id() const { return id_; }
    inline void set_id(Id i) { id_ = i; }

    inline int maximal_dimension() const { return traits.maximal_dimension(dt_); }
    inline int current_dimension() const { return traits.current_dimension(dt_); }

    inline Cell_const_iterator cells_begin() const { return traits.cells_begin(dt_); }
    inline Cell_const_iterator cells_end  () const { return traits.cells_end  (dt_); }

    inline Vertex_const_iterator vertices_begin() const { return traits.vertices_begin(dt_); }
    inline Vertex_const_iterator vertices_end  () const { return traits.vertices_end  (dt_); }

    inline Vertex_iterator vertices_begin() { return traits.vertices_begin(dt_); }
    inline Vertex_iterator vertices_end  () { return traits.vertices_end  (dt_); }

    inline Facet_const_iterator  facets_begin()  const { return traits.facets_begin(dt_); }
    inline Facet_const_iterator  facets_end  ()  const { return traits.facets_end  (dt_); }

    inline size_t number_of_vertices() const { return traits.number_of_vertices(dt_); }
    inline size_t number_of_cells   () const { return traits.number_of_cells   (dt_); }

    inline size_t number_of_main_facets  () const { return number_of_main_facets_;   }
    inline size_t number_of_main_cells   () const { return number_of_main_cells_;    }
    inline size_t number_of_main_finite_vertices() const { return number_of_main_finite_vertices_; }
    inline size_t number_of_main_finite_facets  () const { return number_of_main_finite_facets_;   }
    inline size_t number_of_main_finite_cells   () const { return number_of_main_finite_cells_;    }

    inline Info& info(Vertex_const_handle v) const { assert(!vertex_is_infinite(v)); return traits.info(v); }
    inline Id vertex_id(Vertex_const_handle v) const { assert(!vertex_is_infinite(v)); return traits.id(v); }

    Id cell_id(Cell_const_handle c) const
    {
        selector.clear();
        int D = current_dimension();
        for(int i=0; i<=D; ++i) {
            Vertex_const_handle v = vertex(c, i);
            if(!vertex_is_infinite(v)) selector.insert(vertex_id(v));
        }
        return selector.select();
    }

    Id facet_id(const Facet_const_iterator& f) const
    {
        selector.clear();
        int cid = index_of_covertex(f);
        Cell_const_handle c = cell(f);
        int D = current_dimension();
        for(int i=0; i<=D; ++i) {
            if (i == cid) continue;
            Vertex_const_handle v = vertex(c, i);
            if (!vertex_is_infinite(v)) selector.insert(vertex_id(v));
        }
        return selector.select();
    }

    inline void clear() { traits.clear(dt_); }
    inline std::pair<Vertex_handle, bool> insert(const Point& p, Id id, Vertex_handle v = Vertex_handle()) { return traits.insert(dt_, p, id, v); }
    inline void remove(Vertex_handle v) { traits.remove(dt_, v); }

    inline void spatial_sort(std::vector<std::size_t>& indices, const std::vector<Point>& points) const { traits.spatial_sort(dt_, indices, points); }

    /// \name Infinity tests
    /// @{
    inline bool vertex_is_infinite(Vertex_const_handle v) const { return traits.vertex_is_infinite(dt_, v); }
    inline bool facet_is_infinite (Facet_const_handle  f) const { return traits.facet_is_infinite (dt_, f); }
    inline bool cell_is_infinite  (Cell_const_handle   c) const { return traits.cell_is_infinite  (dt_, c); }
    /// @}

    /// \name Validity tests
    /// @{
    /// A simplex of a tile triangulation is valid if it is a local representative of the corresponding simplex in the overall triangulation

    /// A vertex is valid if it is finite
    inline bool vertex_is_valid(Vertex_const_handle v) const { return !vertex_is_infinite(v); }
    /// A facet is valid if at least one of the following vertices is finite and local : its covertex, its mirror vertex and its incident vertices
    inline bool facet_is_valid(Facet_const_handle f) const { return !cell_is_foreign(cell(f)) || !vertex_is_foreign(mirror_vertex(f)); }
    /// A cell is valid if at least one of its incident vertices are finite and local
    inline bool cell_is_valid(Cell_const_handle c) const { return !cell_is_foreign(c); }
    /// @}

    /// \name Vertex functions
    /// @{
    template<typename OutputIterator>
    inline OutputIterator adjacent_vertices(Vertex_const_handle v, OutputIterator out) const { return traits.adjacent_vertices(dt_, v, out); }
    template<typename OutputIterator>
    inline OutputIterator incident_cells(Vertex_const_handle v, OutputIterator out) const { return traits.incident_cells(dt_, v, out); }
    inline Vertex_handle infinite_vertex() const { return traits.infinite_vertex(dt_); }
    inline const Point& point(Vertex_const_handle v) const { return traits.point(dt_, v); }
    inline double coord(const Point& p, int i) const { return traits.coord(dt_, p, i); }
    /// @}

    /// \name Facet functions
    /// @{
    inline int index_of_covertex(Facet_const_handle f) const { return traits.index_of_covertex(dt_, f); }
    inline Vertex_const_handle covertex(Facet_const_handle f) const { return traits.covertex(dt_, f); }
    inline Vertex_const_handle mirror_vertex(Facet_const_handle f) const { return traits.mirror_vertex(dt_, f); }
    inline Cell_const_handle cell(Facet_const_handle f) const { return traits.cell(dt_, f); }
    inline Cell_const_handle cell(Vertex_const_handle v) const { return traits.cell(dt_, v); }
    inline Facet_const_handle mirror_facet(Facet_const_handle f) const { return traits.mirror_facet(dt_, f); }
    inline int mirror_index(Facet_const_handle f) const { return traits.mirror_index(dt_, f); }
    /// @}

    /// \name Cell functions
    /// @{
    inline Vertex_const_handle vertex(Cell_const_handle c, int i) const { return traits.vertex(dt_, c, i); }
    inline Facet_const_iterator facet(Cell_const_handle c, int i) const { return traits.facet(dt_, c, i); }
    inline int mirror_index(Cell_const_handle c, int i) const { return traits.mirror_index(dt_, c, i); }
    inline Cell_const_handle neighbor(Cell_const_handle c, int i) const { return traits.neighbor(dt_, c, i); }
    /// @}

    /// \name CGAL IO
    /// @{
    /// @todo : remove unused function ?
    ///
    void write_cgal(std::ostream & ofile) const { traits.write_cgal(ofile,dt_); }
    void read_cgal(std::istream & ifile) { traits.read_cgal(ifile,dt_); }
    /// @}

    /// \name Tile locality tests
    /// @{
    /// A finite vertex is local if its tile id matches the id of the tile triangulation (tile.vertex_id(vertex) == tile.id()), otherwise, it is foreign
    /// Simplices may be local, mixed or foreign if respectively all, some or none of their finite incident vertices are local.

    /// checks if a finite vertex is local : tile.vertex_id(vertex) == tile.id()
    /// precondition : the vertex is finite.
    inline bool vertex_is_local(Vertex_const_handle v) const { assert(!vertex_is_infinite(v)); return vertex_id(v) == id(); }

    /// checks if a finite vertex is foreign : tile.vertex_id(vertex) != tile.id()
    /// precondition : the vertex is finite.
    inline bool vertex_is_foreign(Vertex_const_handle v) const { return !vertex_is_local(v); }

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
            if ( vertex_is_infinite(v) ) continue;
            if ( vertex_is_foreign(v) ) return false;
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
            if ( vertex_is_infinite(v) ) continue;
            if ( vertex_is_local(v) ) return false;
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
            if ( vertex_is_infinite(v) ) continue;
            if ( vertex_is_foreign(v) ) return false;
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
            if ( vertex_is_infinite(v) ) continue;
            if ( vertex_is_local(v) ) return false;
        }

        return true;
    }
    /// @}


    /// \name Main tests
    /// @{
    /// checks if a vertex is finite and local : tile.vertex_id(vertex) == tile.id()
    template<typename V> inline bool vertex_is_main(V v) const { return !vertex_is_infinite(v) && vertex_id(v) == id(); }
    /// checks if a facet is main : tile.facet_id(facet) == tile.id()
    template<typename F> inline bool facet_is_main(F f) const { return facet_id(f) == id(); }
    /// checks if a cell is main : tile.cell_id(cell) == tile.id()
    template<typename C> inline bool cell_is_main(C c) const { return cell_id(c) == id(); }
    /// @}

    /// remove a finite vertex if it is foreign and if all its adjacent vertices are foreign
    /// returns whether simplification occured
    bool simplify(Vertex_handle v)
    {
        assert(!vertex_is_infinite(v));
        if (!vertex_is_foreign(v)) return false;
        std::vector<Vertex_const_handle> adj;
        adjacent_vertices(v, std::back_inserter(adj));
        for (Vertex_const_handle a : adj)
            if(!vertex_is_infinite(a) && vertex_is_local(a))
                return false;
        remove(v);
        return true;
    }

    /// Collect at most 2*D vertices which points define the bounding box of the local tile vertices
    void get_axis_extreme_points(std::vector<Vertex_const_handle>& out) const
    {
        std::vector<Vertex_const_handle> v;
        int D = traits.dimension();
        v.reserve(2*D);
        auto vit = vertices_begin();
        // first local point
        for(; vit != vertices_end(); ++vit)
        {
            if (!vertex_is_infinite(vit) && vertex_is_local(vit))
            {
                for(int i=0; i<2*D; ++i) v[i] = vit;
                break;
            }
        }
        if(vit == vertices_end()) return; // no local points
        // other local points
        for(; vit != vertices_end(); ++vit)
        {
            if (!vertex_is_infinite(vit) && vertex_is_local(vit))
            {
                const Point& p = point(vit);
                for(int i=0; i<D; ++i)
                {
                    if(p[i] < point(v[i  ])[i]) v[i  ] = vit;
                    if(p[i] > point(v[i+D])[i]) v[i+D] = vit;
                }
            }
        }
        // remove duplicates (O(D^2) implementation, should we bother ?)
        for(int i=0; i<2*D; ++i)
        {
            int j = 0;
            for(; j<i; ++j) if(v[j]==v[i]) break;
            if(i==j) out.push_back(v[i]);
        }
    }

    /// Collect (vertex,id) pairs listing finite vertices that are possibly newly adjacent to vertices of a foreign tile (id),
    /// after the insertion of the inserted vertices, as required by the star splaying algorithm.
    void get_finite_neighbors(const std::set<Vertex_const_handle>& inserted, std::map<Id, std::set<Vertex_const_handle>>& out) const
    {
        for(auto v : inserted) {
            if(vertex_is_infinite(v)) continue;
            Id idv = vertex_id(v);
            std::vector<Vertex_const_handle> vadj;
            adjacent_vertices(v, std::back_inserter(vadj));
            for (auto w : vadj) {
                if(vertex_is_infinite(w)) continue;
                Id idw = vertex_id(w);
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
    /// reports either the set of all inserted points by default.
    /// If foreign_only is true, then only foreign inserted points are reported.
    /// Foreign vertices of the tile triangulation are automatically simplified if the insertion makes it possible.
    /// @returns the number of inserted points (disregarding the reinsertions of already inserted points
    /// and the foreign point simplifications)
    template <class PointIdContainer>
    int insert(const PointIdContainer& received, std::set<Vertex_const_handle>& inserted, bool foreign_only=false)
    {
        // retrieve the input points and ids in separate vectors
        // compute the axis-extreme points on the way
        std::vector<Point> points;
        std::vector<Id> ids;
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
        Vertex_handle v;
        int local_inserted_size = 0;
        for (size_t index : indices) {
          std::pair<Vertex_handle,bool> p = insert(points[index], ids[index], v);
          if (!p.second) {
              v = p.first;
          } else if (!simplify(p.first)) {
              v = p.first;
              if (!foreign_only || vertex_is_foreign(v))
                inserted.insert(v);
              else
                ++local_inserted_size;
          }
        }

        // simplify neighbors: collect vertices adjacent to newly inserted foreign points
        std::set<Vertex_handle> adj;
        for (Vertex_const_handle v : inserted)
            if(foreign_only || vertex_is_foreign(v))
                adjacent_vertices(v, std::inserter(adj, adj.begin()));

        for (Vertex_handle v : adj)
            if(!vertex_is_infinite(v) && simplify(v))
                inserted.erase(v);

        return local_inserted_size + inserted.size();
    }

    void get_adjacency_graph_edges(std::set<Id>& out_edges) const
    {
        for(auto cit = cells_begin(); cit != cells_end(); ++cit)
            if(cell_is_mixed(cit))
                for(int i=0; i<=current_dimension(); ++i)
                {
                    Vertex_const_handle v = vertex(cit,i);
                    if(!vertex_is_infinite(v) && vertex_is_foreign(v))
                        out_edges.insert(vertex_id(v));
                }
    }

    bool are_vertices_equal(Vertex_const_handle v, const Tile& t, Vertex_const_handle tv) const
    {
        return traits.are_vertices_equal(dt_, v, t.dt_, tv);
    }

    bool are_facets_equal(Facet_const_handle f, const Tile& t, Facet_const_handle tf) const
    {
        return traits.are_facets_equal(dt_, f, t.dt_, tf);
    }

    bool are_cells_equal(Cell_const_handle c, const Tile& t, Cell_const_handle tc) const
    {
        return traits.are_cells_equal(dt_, c, t.dt_, tc);
    }


    Vertex_const_handle locate_vertex(const Point& p, Vertex_handle hint = Vertex_handle()) const
    {
        return traits.locate_vertex(dt_, p, hint);
    }

    Vertex_const_handle relocate_vertex(const Tile& t, Vertex_const_handle v, Vertex_handle hint = Vertex_handle()) const
    {
        if(t.vertex_is_infinite(v)) return infinite_vertex();
        return locate_vertex(t.point(v), hint);
    }

    Facet_const_handle relocate_facet(const Tile& t, Facet_const_handle f) const
    {
        assert(t.facet_is_valid(f));
        Cell_const_handle c = t.cell(f);
        if(t.cell_is_foreign(c)) return mirror_facet(relocate_facet(t, t.mirror_facet(f)));
        assert(!t.cell_is_foreign(c));
        Cell_const_handle d = relocate_cell(t, c);
        Vertex_const_handle v = t.vertex(c, t.index_of_covertex(f));
        for(int i=0; i<=current_dimension(); ++i)
        {
            if(traits.are_vertices_equal(t.dt_, v, dt_, vertex(d, i)))
                return facet(d, i);
        }
        return facets_end();
    }

    Cell_const_handle relocate_cell(const Tile& t, Cell_const_handle c) const
    {
        Vertex_const_handle v = relocate_vertex(t, t.vertex(c, 0));
        if (v == Vertex_const_handle()) return cells_end();
        std::vector<Cell_const_handle> cells;
        incident_cells(v, std::back_inserter(cells));
        for(Cell_const_handle ic: cells)
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

        for(auto vit = vertices_begin(); vit != vertices_end(); ++vit)
        {
            if(vertex_is_main(vit))
                ++number_of_main_finite_vertices_;
        }
        for(auto fit = facets_begin(); fit != facets_end(); ++fit )
        {
            if(facet_is_main(fit))
            {
                ++number_of_main_facets_;
                if(!facet_is_infinite(fit))
                    ++number_of_main_finite_facets_;
            }
        }

        for(auto cit = cells_begin(); cit != cells_end(); ++cit)
        {
            if(cell_is_main(cit))
            {
                ++number_of_main_cells_;
                if(!cell_is_infinite(cit))
                    ++number_of_main_finite_cells_;
            }
        }
    }


    inline bool is_valid(bool verbose = false, int level = 0) const
    {
      return traits.is_valid(dt_, verbose, level);
    }

    const Traits& geom_traits() const { return traits; }

    bool in_use;
    size_t number_of_extreme_points_received;

private:
    Traits traits;
    Id id_;
    Delaunay_triangulation dt_;
    mutable Selector<Id> selector;

    size_t number_of_main_finite_vertices_;
    size_t number_of_main_finite_facets_;
    size_t number_of_main_finite_cells_;
    size_t number_of_main_facets_;
    size_t number_of_main_cells_;
};


template<class T>
std::istream& operator>> (std::istream& is,Tile<T> & tt)
{

  is >> tt.triangulation();
  return is;
}

template<class T>
std::ostream& operator<< (std::ostream& os,const Tile<T> & tt)
{
    os << tt.triangulation();
    return os;
}

}
}

#endif // CGAL_DDT_TILE_H
