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

#include <CGAL/DDT/Bbox.h>

#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <assert.h>

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTClasses
/// \tparam T is a model of the TriangulationTraits concept
/// The Tile stores a local Delaunay triangulation
template<class T>
class Tile
{
public:
    typedef T                                        Traits;
    typedef typename Traits::Id                      Id;
    typedef typename Traits::Flag                    Flag;
    typedef typename Traits::Point                   Point;
    typedef typename Traits::Delaunay_triangulation  DT;
    typedef typename Traits::Vertex_handle           Vertex_handle;
    typedef typename Traits::Vertex_iterator         Vertex_iterator;
    typedef typename Traits::Vertex_const_handle     Vertex_const_handle;
    typedef typename Traits::Vertex_const_iterator   Vertex_const_iterator;
    typedef typename Traits::Cell_handle             Cell_handle;
    typedef typename Traits::Cell_const_handle       Cell_const_handle;
    typedef typename Traits::Cell_const_iterator     Cell_const_iterator;
    typedef typename Traits::Facet_const_iterator    Facet_const_iterator;
    typedef typename Traits::Facet_const_iterator    Facet_const_handle;
    typedef std::pair<Cell_const_handle,Id>          Cell_const_handle_and_id;
    typedef std::pair<Vertex_const_handle,Id>        Vertex_const_handle_and_id;
    typedef std::pair<Point,Id>                      Point_id;
    enum { D = Traits::D };

    /// constructor
    Tile(Id id, int dimension = D)
        : id_(id),
          dt_(traits.triangulation(dimension)),
          number_of_main_finite_vertices_(0),
          number_of_main_finite_facets_(0),
          number_of_main_finite_cells_(0),
          number_of_main_facets_(0),
          number_of_main_cells_(0)
    {}

    inline DT& triangulation() { return dt_; }
    inline const DT& triangulation() const { return dt_; }

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

    inline Id    id  (Vertex_const_handle v) const { assert(!vertex_is_infinite(v)); return traits.id  (v); }
    inline Flag& flag(Vertex_const_handle v) const { assert(!vertex_is_infinite(v)); return traits.flag(v); }

    Id minimum_id(Cell_const_handle c) const
    {
        bool valid = false;
        Id mid = 0;
        int D = current_dimension();
        for(int i=0; i<=D; ++i)
        {
            Vertex_const_handle v = vertex(c, i);
            if(vertex_is_infinite(v)) continue;
            Id vid = id(v);
            if (!valid || vid < mid) { mid = vid; valid = true; }
        }
        assert(valid);
        return mid;
    }

    Id minimum_id(const Facet_const_iterator& f) const
    {
        int cid = index_of_covertex(f);
        Cell_const_handle c = cell(f);
        bool valid = false;
        Id mid = 0;
        int D = current_dimension();
        for(int i=0; i<=D; ++i)
        {
            if (i == cid) continue;
            Vertex_const_handle v = vertex(c, i);
            if (vertex_is_infinite(v)) continue;
            Id vid = id(v);
            if (!valid || vid < mid) { mid = vid; valid = true; }
        }
        assert(valid);
        return mid;
    }

    inline void clear() { traits.clear(dt_); }
    template<class It> inline void insert(It begin, It end) { traits.insert(dt_, begin, end); }
    template<class It> inline void remove(It begin, It end) { traits.remove(dt_, begin, end); }

    inline Vertex_handle infinite_vertex() const { return traits.infinite_vertex(dt_); }
    inline const Point& point(Vertex_const_handle v) const { return traits.point(dt_, v); }
    inline double coord(const Point& p, int i) const { return traits.coord(dt_, p, i); }

    inline bool vertex_is_infinite(Vertex_const_handle v) const { return traits.vertex_is_infinite(dt_, v); }
    inline bool facet_is_infinite (Facet_const_handle  f) const { return traits.facet_is_infinite (dt_, f); }
    inline bool cell_is_infinite  (Cell_const_handle   c) const { return traits.cell_is_infinite  (dt_, c); }

    inline bool vertex_is_valid(Vertex_const_handle v) const { return !vertex_is_infinite(v); }
    inline bool facet_is_valid(Facet_const_handle f) const { return !cell_is_foreign(cell(f)) || !vertex_is_foreign(mirror_vertex(f)); }
    inline bool cell_is_valid(Cell_const_handle c) const { return !cell_is_foreign(c); }

    /// Facet functions
    inline int index_of_covertex(Facet_const_handle f) const { return traits.index_of_covertex(dt_, f); }
    inline Vertex_const_handle covertex(Facet_const_handle f) const { return traits.covertex(dt_, f); }
    inline Vertex_const_handle mirror_vertex(Facet_const_handle f) const { return traits.mirror_vertex(dt_, f); }
    inline Cell_const_handle cell(Facet_const_handle f) const { return traits.cell(dt_, f); }
    inline Facet_const_handle mirror_facet(Facet_const_handle f) const { return traits.mirror_facet(dt_, f); }
    inline int mirror_index(Facet_const_handle f) const { return traits.mirror_index(dt_, f); }

    /// Cell functions
    inline Vertex_const_handle vertex(Cell_const_handle c, int i) const { return traits.vertex(dt_, c, i); }
    inline Facet_const_iterator facet(Cell_const_handle c, int i) const { return traits.facet(dt_, c, i); }
    inline int mirror_index(Cell_const_handle c, int i) const { return traits.mirror_index(dt_, c, i); }
    inline Cell_const_handle neighbor(Cell_const_handle c, int i) const { return traits.neighbor(dt_, c, i); }

    // read/write (TODO: move away?)
    void write_cgal(std::ostream & ofile) const { traits.write_cgal(ofile,dt_); }
    void read_cgal(std::istream & ifile) { traits.read_cgal(ifile,dt_); }

    // Local => all vertex ids are equal to the tile id
    // Mixed  => some vertex ids are equal to the tile id
    // Foreign => no vertex ids are equal to the tile id
    inline bool vertex_is_local(Vertex_const_handle v) const { assert(!vertex_is_infinite(v)); return id(v) == id(); }
    inline bool vertex_is_local_in_tile(Vertex_const_handle v, int tid) const { assert(!vertex_is_infinite(v)); return id(v) == tid; }
    inline bool vertex_is_foreign(Vertex_const_handle v) const { return !vertex_is_local(v); }

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


    template<typename C>
    bool cell_is_foreign_in_tile(C c, int tid) const
    {
        for(int i=0; i<=current_dimension(); ++i)
        {
            auto v = vertex(c,i);
            if ( vertex_is_infinite(v) ) continue;
            if ( vertex_is_local_in_tile(v,tid) ) return false;
        }

        return true;
    }



    // Main
    template<typename V>
    bool vertex_is_main(V v) const
    {
        /// @todo define somehow the main infinite vertex
        return !vertex_is_infinite(v) && vertex_is_local(v) ;
    }

    template<typename F>
    bool facet_is_main(F f) const
    {
        int icv = index_of_covertex(f);
        auto c = cell(f);
        bool foreign = true;
        for(int i=0; i<=current_dimension(); ++i)
        {
            if (i == icv) continue;
            auto v = vertex(c,i);
            if (vertex_is_infinite(v)) continue;
            Id vid = id(v);
            if ( vid < id() )
                return false;
            else if (vid == id())
                foreign = false;
        }
        return !foreign;
    }

    template<typename C>
    bool cell_is_main(C c) const
    {
        bool foreign = true;
        for(int i=0; i<=current_dimension(); ++i)
        {
            auto v = vertex(c,i);
            if (vertex_is_infinite(v)) continue;
            Id vid = id(v);
            if ( vid < id() )
                return false;
            else if (vid == id())
                foreign = false;
        }
        return !foreign;
    }

    /// remove vertices that are adjacent to foreign cells only
    /// returns the number of removed vertices
    int simplify()
    {
        // initialize flags to 1
        for(auto vit = vertices_begin(); vit != vertices_end(); ++vit)
            if(!vertex_is_infinite(vit))
                flag(vit) = 1;

        // set flags of vertices incident to non-foreign cells to 0
        for(auto cit = cells_begin(); cit != cells_end(); ++cit)
        {
            if(cell_is_foreign(cit)) continue;
            for(int i=0; i<=current_dimension(); ++i)
            {
                Vertex_const_handle v = vertex(cit, i);
                if(!vertex_is_infinite(v))
                {
                    flag(v) = 0;
                }
            }
        }

        // gather vertices that are to be removed
        std::vector<Vertex_handle> todo;
        for(auto vit = vertices_begin(); vit != vertices_end(); ++vit)
            if(!vertex_is_infinite(vit) && flag(vit))
                todo.push_back(vit);
        // remove these vertices
        remove(todo.begin(), todo.end());
        return todo.size();
    }

    void get_axis_extreme_points(std::vector<Vertex_const_handle>& out) const
    {
        Vertex_const_handle v[2*D];
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

    void get_local_neighbors(std::vector<Vertex_const_handle_and_id>& out) const
    {
        std::map<Id, std::set<Vertex_const_handle>> outbox;
        for(Cell_const_iterator cit = cells_begin(); cit != cells_end(); ++cit)
            for(int i=0; i<=current_dimension(); ++i)
            {
                Vertex_const_handle v = vertex(cit, i);
                if(vertex_is_infinite(v)) break;
                Id idv = id(v);
                if(idv != id())
                    for(int j=0; j<=current_dimension(); ++j)
                        if(vertex_is_local(vertex(cit, j))) // implies i!=j
                            outbox[idv].insert(vertex(cit, j));
            }
        for(auto&& pair : outbox)
            for(auto vh : pair.second)
                out.push_back(std::make_pair(vh, pair.first));
    }

    void get_finite_neighbors(std::vector<Vertex_const_handle_and_id>& out) const
    {
        /// @todo: change api to consider only newly inserted vertices in splay_star (convert )
        std::map<Id, std::set<Vertex_const_handle>> outbox;
        for(auto cit = cells_begin(); cit != cells_end(); ++cit)
            for(int i=0; i<=current_dimension(); ++i)
            {
                Vertex_const_handle v = vertex(cit, i);
                if(vertex_is_infinite(v)) break;
                Id idv = id(v);
                if(idv != id())
                {
                    for(int j=0; j<=current_dimension(); ++j)
                    {
                        Vertex_const_handle w = vertex(cit, j);
                        if(!vertex_is_infinite(w) && id(w) != idv) // implies i!=j
                            outbox[idv].insert(w);
                    }
                }
            }
        for(auto&& pair : outbox)
        {
            Id idv = pair.first;
            for(auto w : pair.second)
            {
                out.push_back(std::make_pair(w, idv));
            }
        }
    }

    /// @todo : expose spatial_sort in traits
    /// @todo : expose insert(point)->vertex in traits
    /// @todo : return container of new foreign vertices
    template <class PointIdContainer>
    int insert(const PointIdContainer& received, bool do_simplify = true)
    {
        if(received.empty()) return 0;
        std::vector<Point_id> points;
        points.reserve(received.size());
        // spatial_sort
        for(auto& v : received)
        {
            Id vid = v.second;
            points.emplace_back(v.first,vid);
            bbox_[vid] += v.first;
            // insert point
        }
        insert(points.begin(), points.end());
        int s = 0;
        if(do_simplify)
            s = simplify();
        return points.size() - s;
    }

    void get_mixed_cells(std::vector<Cell_const_handle_and_id>& out) const
    {
        for(auto cit = cells_begin(); cit != cells_end(); ++cit)
            if(cell_is_mixed(cit))
                for(int i=0; i<=current_dimension(); ++i)
                    if(!vertex_is_infinite(vertex(cit,i)))
                        out.emplace_back(cit, id(vertex(cit,i)));
    }

    void get_adjacency_graph_edges(std::set<Id>& out_edges) const
    {
        for(auto cit = cells_begin(); cit != cells_end(); ++cit)
            if(cell_is_mixed(cit))
                for(int i=0; i<=current_dimension(); ++i)
                {
                    Vertex_const_handle v = vertex(cit,i);
                    if(!vertex_is_infinite(v) && id(v) != id())
                        out_edges.insert(id(v));
                }
    }
/*
    void get_merge_graph_edges(std::set<Id>& out_edges, std::vector<Cell_const_handle>& finalized) const
    {
        for(auto cit = cells_begin(); cit != cells_end(); ++cit)
        {
            if(cell_is_foreign(cit)) continue;
            bool active = false;
            for(auto pair : bbox_)
                if((active || out_edges.find(pair.first)==out_edges.end()) && cell_is_active(pair.second, cit) )
                {
                    active = true;
                    out_edges.insert(pair.first);
                }
            if(!active)
                finalized.push_back(cit);
        }
    }


    bool cell_is_finalized(Cell_const_handle c) const
    {
        if(cell_is_foreign(c)) return false;
        /// @todo acceleration data structure !!!
        for(auto pair : bbox_)
            if(pair.first != id() && cell_is_active(pair.second, c)) return false;
        return true;
    }
*/

    const std::map<Id, Bbox<D,double>>& bbox() const
    {
        return bbox_;
    }


    std::map<Id, Bbox<D,double>>& bbox()
    {
        return bbox_;
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


    Vertex_const_handle locate_vertex(const Point& p) const
    {
        return traits.locate_vertex(dt_, p);
    }

    Vertex_const_handle relocate_vertex(const Tile& t, Vertex_const_handle v) const
    {
        if(t.vertex_is_infinite(v)) return infinite_vertex();
        return locate_vertex(t.point(v));
    }

    /*
    Vertex_const_handle relocate_vertex(const Tile& t, Vertex_const_handle tv) const
    {
        for(auto v = vertices_begin(); v != vertices_end(); ++v )
            if(are_vertices_equal(v, t, tv))
                return v;
        return vertices_end();
    }
    */

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

    Cell_const_handle relocate_cell(const Tile& t, Cell_const_handle tc) const
    {
        assert(!t.cell_is_foreign(tc));
        /// @todo locate the first vertex point of c in the other dt
        for(auto c = cells_begin(); c != cells_end(); ++c )
            if(are_cells_equal(c, t, tc))
                return c;
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


    bool is_valid() const
    {

      return dt_.is_valid(true,5);
    }

private:
    Traits traits;
    Id id_;
    DT dt_;

    size_t number_of_main_finite_vertices_;
    size_t number_of_main_finite_facets_;
    size_t number_of_main_finite_cells_;
    size_t number_of_main_facets_;
    size_t number_of_main_cells_;

    std::map<Id, Bbox<D, double>> bbox_;
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
