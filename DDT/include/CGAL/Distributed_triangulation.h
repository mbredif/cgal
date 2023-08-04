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

#ifndef CGAL_DISTRIBUTED_TRIANGULATION_H
#define CGAL_DISTRIBUTED_TRIANGULATION_H

#include <CGAL/DDT/iterator/Vertex_iterator.h>
#include <CGAL/DDT/iterator/Facet_iterator.h>
#include <CGAL/DDT/iterator/Cell_iterator.h>
#include <CGAL/DDT/insert.h>
#include <CGAL/DDT/Tile_triangulation.h>
#include <CGAL/DDT/Tile_container.h>
#include <CGAL/Distributed_point_set.h>

namespace CGAL {

/// \ingroup PkgDDTRef
/// \tparam template params to instantiate a `CGAL::DDT::Tile_container` that manages the storage of the triangulation tiles.
/// The Distributed_triangulation class wraps a Container to expose a triangulation interface.
template<typename Triangulation_,
         typename TileIndexProperty_,
         typename Serializer_ = CGAL::DDT::No_serializer >
class Distributed_triangulation
{
public:
    typedef Triangulation_                                                  Triangulation;
    typedef TileIndexProperty_                                              TileIndexProperty;
    typedef Serializer_                                                     Serializer;

    typedef typename TileIndexProperty::value_type                          Tile_index;
    typedef CGAL::DDT::Tile_triangulation<Triangulation, TileIndexProperty> Tile_triangulation;
    typedef std::unordered_map<Tile_index, Tile_triangulation>              AssociativeContainer;
    // typedef std::map<Tile_index, Tile_triangulation>                      AssociativeContainer; // alternative
    typedef CGAL::DDT::Tile_container<AssociativeContainer, Serializer_>    Container;
    // typedef AssociativeContainer                                            Container; // alternative, disables serialization

private:
    typedef typename Container::iterator            Tile_iterator;
    typedef typename Container::const_iterator      Tile_const_iterator;

    typedef CGAL::DDT::Statistics Statistics;
    typedef CGAL::DDT::Triangulation_traits<Triangulation>    Traits;
    typedef typename Traits::Vertex_index               Tile_vertex_index;
    typedef typename Traits::Cell_index                 Tile_cell_index;
    typedef typename Traits::Facet_index                Tile_facet_index;
    typedef typename Traits::Point                      Point;

public:
/// \name Types
/// @{

#ifndef DOXYGEN_RUNNING
    typedef CGAL::DDT::Vertex_iterator<Container> Vertex_iterator;
    typedef CGAL::DDT::Facet_iterator <Container> Facet_iterator;
    typedef CGAL::DDT::Cell_iterator  <Container> Cell_iterator;
#else
    /// A const iterator to the vertices of a distributed Delaunay triangulation
    typedef unspecified_type Vertex_iterator;
    /// A const iterator to the facets of a distributed Delaunay triangulation
    typedef unspecified_type Facet_iterator;
    /// A const iterator to the cells of a distributed Delaunay triangulation
    typedef unspecified_type Cell_iterator;
#endif
/// @}

    /// contructor
    template<typename... Args>
    Distributed_triangulation(int dim, Args&&... args)
    :   maximal_dimension_(dim),
        tiles(std::forward<Args>(args)...),
        statistics_()
    {}

    /// returns the dimension of the triangulation
    inline int maximal_dimension() const { return maximal_dimension_; }
    /// returns the dimension of the triangulation
    inline int& maximal_dimension() { return maximal_dimension_; }
    /// returns the number of finite cells in the triangulation, including cells incident to the vertex at infinity.
    inline std::size_t number_of_finite_cells   () const { return statistics_.number_of_finite_cells;    }
    /// returns the number of finite vertices in the triangulation, including the vertex at infinity.
    inline std::size_t number_of_finite_vertices() const { return statistics_.number_of_finite_vertices; }
    /// returns the number of facets in the triangulation, including facets incident to the vertex at infinity.
    inline std::size_t number_of_finite_facets  () const { return statistics_.number_of_finite_facets;   }
    /// returns the number of finite cells in the triangulation.
    inline std::size_t number_of_cells   () const { return statistics_.number_of_cells;    }
    /// returns the number of finite vertices in the triangulation.
    inline std::size_t number_of_vertices() const { return statistics_.number_of_finite_vertices + 1; }
    /// returns the number of finite facets in the triangulation.
    inline std::size_t number_of_facets  () const { return statistics_.number_of_facets;   }

    /// \name Iterators
    /// @{

    /// returns a const iterator at the start of the range of finite vertices.
    Vertex_iterator vertices_begin() const { return Vertex_iterator(&tiles, tiles.cbegin()); }
    /// returns a const iterator past the end of the range of finite vertices.
    Vertex_iterator vertices_end  () const { return Vertex_iterator(&tiles, tiles.cend()); }

    /// returns a const iterator at the start of the range of finite cells.
    Cell_iterator cells_begin() const { return Cell_iterator(&tiles, tiles.cbegin()); }
    /// returns a const iterator past the end of the range of finite cells.
    Cell_iterator cells_end  () const { return Cell_iterator(&tiles, tiles.cend()); }

    /// returns a const iterator at the start of the range of finite facets.
    Facet_iterator facets_begin() const { return Facet_iterator(&tiles, tiles.cbegin()); }
    /// returns a const iterator past the end of the range of finite facets.
    Facet_iterator facets_end  () const { return Facet_iterator(&tiles, tiles.cend()); }

    /// @}

    /// \name Global Identifiers
    /// @{

    /// returns a global id of the vertex iterator using its distance to vertices_begin. (implementation is linear in the returned id)
    int vertex_id(Vertex_iterator v) const
    {
        if (is_infinite(v)) return -1;
        return std::distance(vertices_begin(), main(v));
    }

    /// returns a global id of the cell iterator using its distance to cells_begin. (implementation is linear in the returned id)
    int cell_id(Cell_iterator c) const
    {
        return std::distance(cells_begin(), main(c));
    }
    /// @}

    /// checks the validity of the Distributed_triangulation
    bool is_valid(bool verbose = false, int level = 0) const
    {
        for(const auto& [id, tile] : tiles)
        {
            const Tile_triangulation& tri = tile;
            for(Tile_vertex_index v = tri.vertices_begin(); v != tri.vertices_end(); ++v)
            {
                assert(tri.vertex_is_infinite(v) || (tri.vertex_is_local(v) + tri.vertex_is_foreign(v) == 1));
                if(tri.vertex_is_infinite(v)) continue;
                Tile_index tid = tri.vertex_id(v);
                if(tid == tri.id()) continue;
                Tile_const_iterator t = tiles.find(tid);
                const Tile_triangulation& tri2 = t->second;
                if(tri2.relocate_vertex(tri, v) == tri2.vertices_end())
                {
                    assert(! "relocate_vertex failed" );
                    return false;
                }
            }
            for(Tile_facet_index f = tri.facets_begin(); f != tri.facets_end(); ++f)
            {
                assert(tri.facet_is_local(f) + tri.facet_is_mixed(f) + tri.facet_is_foreign(f) == 1);
                if(!tri.facet_is_mixed(f)) continue;
                std::set<Tile_index> tids;
                for(int d = 0; d <= tri.current_dimension(); ++d)
                {
                    if(d==tri.index_of_covertex(f)) continue;
                    Tile_cell_index c = tri.cell(f);
                    Tile_vertex_index v = tri.vertex(c, d);
                    if(tri.vertex_is_infinite(v)) continue;
                    Tile_index tid = tri.vertex_id(v);
                    if(tid == tri.id()) continue;
                    tids.insert(tid);
                }
                for(Tile_index tid : tids)
                {
                    Tile_const_iterator t = tiles.find(tid);
                    const Tile_triangulation& tri2 = t->second;
                    if(tri2.relocate_facet(tri, f) == tri2.facets_end())
                    {
                      assert(! "relocate_facet failed" );
                      return false;
                    }
                }

            }
            for(Tile_cell_index c = tri.cells_begin(); c != tri.cells_end(); ++c)
            {
                assert(tri.cell_is_local(c) + tri.cell_is_mixed(c) + tri.cell_is_foreign(c) == 1);
                if(!tri.cell_is_mixed(c)) continue;
                std::set<Tile_index> tids;
                for(int d = 0; d <= tri.current_dimension(); ++d)
                {
                    Tile_vertex_index v = tri.vertex(c, d);
                    if(tri.vertex_is_infinite(v)) continue;
                    Tile_index tid = tri.vertex_id(v);
                    if(tid == tri.id()) continue;
                    tids.insert(tid);
                }
                for(Tile_index tid : tids)
                {
                    Tile_const_iterator t = tiles.find(tid);
                    const Tile_triangulation& tri2 = t->second;
                    if(tri2.relocate_cell(tri, c) == tri2.cells_end())
                    {
                      assert(! "relocate_facet failed" );
                        return false;
                    }
                }
            }

            if(!tri.is_valid(verbose, level))
            {
                std::cerr << "Tile " << std::to_string(id) << " is invalid" << std::endl;
                //assert(! "CGAL tile not valid" );
                return false;
            }
        }
        return true;
    }

    /// \name Iterator tests
    /// @{

    bool is_local(const Vertex_iterator& v) const { return v.triangulation().vertex_is_local(*v); }
    bool is_local(const Facet_iterator&  f) const { return f.triangulation().facet_is_local(*f); }
    bool is_local(const Cell_iterator&   c) const { return c.triangulation().cell_is_local(*c); }

    bool is_valid(const Vertex_iterator& v) const { return v.triangulation().vertex_is_valid(*v); } // + tile toujours chargée ?
    bool is_valid(const Facet_iterator&  f) const { return f.triangulation().facet_is_valid(*f); } // + tile toujours chargée ?
    bool is_valid(const Cell_iterator&   c) const { return c.triangulation().cell_is_valid(*c); } // + tile toujours chargée ?

    // vertices are never mixed
    bool is_mixed(const Facet_iterator& f) const { return f.triangulation().facet_is_mixed(*f); }
    bool is_mixed(const Cell_iterator&  c) const { return c.triangulation().cell_is_mixed(*c); }

    bool is_foreign(const Vertex_iterator& v) const { return v.triangulation().vertex_is_foreign(*v); }
    bool is_foreign(const Facet_iterator&  f) const { return f.triangulation().facet_is_foreign(*f); }
    bool is_foreign(const Cell_iterator&   c) const { return c.triangulation().cell_is_foreign(*c); }

    bool is_main(const Vertex_iterator& v) const { return v.triangulation().vertex_is_main(*v); }
    bool is_main(const Facet_iterator&  f) const { return f.triangulation().facet_is_main(*f); }
    bool is_main(const Cell_iterator&   c) const { return c.triangulation().cell_is_main(*c); }

    bool is_infinite(const Vertex_iterator& v) const { return v.triangulation().vertex_is_infinite(*v); }
    bool is_infinite(const Facet_iterator&  f) const { return f.triangulation().facet_is_infinite(*f); }
    bool is_infinite(const Cell_iterator&   c) const { return c.triangulation().cell_is_infinite(*c); }
    /// @}

    /// \name Tile identifiers from iterators
    /// @{
    Tile_index id(const Vertex_iterator&v) const { return v.triangulation().vertex_id(*v); }
    Tile_index id(const Facet_iterator& f) const { return f.triangulation().facet_id(*f); }
    Tile_index id(const Cell_iterator&  c) const { return c.triangulation().cell_id(*c); }

    Tile_index tile_id(const Vertex_iterator& v) const { return v.id(); }
    Tile_index tile_id(const Facet_iterator&  f) const { return f.id(); }
    Tile_index tile_id(const Cell_iterator&   c) const { return c.id(); }
    /// @}

    /// \name Iterator relocation
    /// @{
    /// `relocate` functions return an alternative iterator that represents the same simplex,
    /// but that lives in the tile with the provided Tile_index.
    /// If the simplex is not represented there, the end iterator is returned

    /// returns a vertex iterator equivalent to v in tile id. They represent the same vertex of the global triangulation.
    Vertex_iterator relocate(const Vertex_iterator& v, Tile_index id) const
    {
        assert(is_valid(v));
        if (id == tile_id(v)) return v; // v is already in tile id
        Tile_const_iterator tile = tiles.find(id);
        if (tile == tiles.end()) return vertices_end();
        const Tile_triangulation& tri = tile->second;
        Tile_vertex_index vertex = tri.relocate_vertex(v.triangulation(), *v);
        if (vertex==tri.vertices_end()) return vertices_end();
        return Vertex_iterator(&tiles, tile, vertex);
    }

    /// returns a facet iterator equivalent to f in tile id. They represent the same facet of the global triangulation.
    Facet_iterator relocate(const Facet_iterator& f, Tile_index id) const
    {
        assert(is_valid(f));
        if (id == tile_id(f)) return f; // f is already in tile id
        Tile_const_iterator tile = tiles.find(id);
        if (tile == tiles.end()) return facets_end();
        const Tile_triangulation& tri = tile->second;
        Tile_facet_index facet = tri.relocate_facet(f.triangulation(), *f);
        if (facet==tri.facets_end()) return facets_end();
        return Facet_iterator(&tiles, tile, facet);
    }

    /// returns a cell iterator equivalent to c in tile id. They represent the same cell of the global triangulation.
    Cell_iterator relocate(const Cell_iterator& c, Tile_index id) const
    {
        assert(is_valid(c));
        if (id == tile_id(c)) return c; // c is already in tile id
        Tile_const_iterator tile = tiles.find(id);
        if (tile == tiles.end()) return cells_end();
        const Tile_triangulation& tri = tile->second;
        Tile_cell_index cell = tri.relocate_cell(c.triangulation(), *c);
        if (cell==tri.cells_end()) return cells_end();
        return Cell_iterator(&tiles, tile, cell);
    }

    /// returns the main representative of a vertex iterator
    inline Vertex_iterator main(const Vertex_iterator& v) const { return is_infinite(v) ? v : relocate(v, id(v)); }
    /// returns the main representative of a facet iterator
    inline Facet_iterator main(const Facet_iterator& f) const { return relocate(f, id(f)); }
    /// returns the main representative of a cell iterator
    inline Cell_iterator main(const Cell_iterator& c) const { return relocate(c, id(c)); }

    /// @}


    /// \name Iterator operations
    /// @{

    /// returns a representative iterator for the infinite vertex
    /// precondition : at least one tile is loaded.
    inline Vertex_iterator infinite_vertex() const
    {
        assert(!tiles.empty());
        Tile_const_iterator tile = tiles.cbegin();
        const Tile_triangulation& tri = tile->second;
        return Vertex_iterator(&tiles, tile, tri.infinite_vertex());
    }

    /// returns the ith vertex of cell c.
    /// Indexing by i is consistent over all representatives of cell c, as its main representative is looked up.
    Vertex_iterator vertex (const Cell_iterator& c, const int i) const
    {
        assert(is_valid(c));
        return local_vertex(main(c), i); // i is defined wrt the main representative
    }

    /// returns the point embedding of the vertex.
    /// This can be done locally without considering the main tile of the vertex, as point coordinates
    /// are replicated in all tiles.
    const Point& point(Vertex_iterator v) const
    {
        assert(is_valid(v));
        return v.triangulation().point(*v);
    }

    /// returns the mirror facet. This operation is performed locally: the resulting facet belongs
    /// to the same tile as the input facet.
    /// Precondition: the facet f is valid (ie: at least one vertex is local among the covertex and the mirror vertex and the facet points)
    Facet_iterator mirror_facet(const Facet_iterator& f) const
    {
        assert(is_valid(f));
        return Facet_iterator(&tiles, f.tile(), f.triangulation().mirror_facet(*f));
    }

    /// returns the mirror index of facet f, such that neighbor(cell(mirror_facet(f)), mirror_index)==cell(f)
    /// The index of covertex of the main version of the mirror facet of f is considered, as indices may not be consistent across
    /// the representatives of mirror facet in other tiles.
    inline int mirror_index(const Facet_iterator& f) const
    {
        assert(is_valid(f));
        return index_of_covertex(mirror_facet(f));
    }

    /// returns the full cell that is incident to the input facet f and that joins the covertex with the vertices of f
    /// The operation is local iff the local cell of f is not foreign.
    Cell_iterator cell(const Facet_iterator& f) const
    {
        assert(is_valid(f));
        const Tile_triangulation& tri = f.triangulation();
        Tile_cell_index c = tri.cell(*f);
        if(tri.cell_is_foreign(c)) return local_cell(main(f)); // any non foreign representative could do
        return Cell_iterator(&tiles, f.tile(), c);
    }

    /// returns one of the full cells that is incident to the input vertex v. The operation is local
    Cell_iterator cell(const Vertex_iterator& v) const
    {
        const Tile_triangulation& triangulation = v.triangulation();
        Tile_vertex_index tv = *v;
        Tile_cell_index tc = triangulation.cell(tv);
        if(!triangulation.cell_is_foreign(tc))
            return Cell_iterator(&tiles, v.tile(), tc);

        std::vector<Tile_cell_index> cells;
        triangulation.incident_cells(tv, std::back_inserter(cells));
        for(Tile_cell_index c: cells)
            if(!triangulation.cell_is_foreign(c))
                return Cell_iterator(&tiles, v.tile(), c);
        assert(false); // all incident cells are foreign, v should have been simplified !
        return cells_end();
    }

    /// returns whether vertex v is incident to cell c. The operation is local in the tile of c
    bool has_vertex(const Cell_iterator& c, const Vertex_iterator& v) const
    {
        Tile_cell_index tc = *c;
        Tile_vertex_index tv = *v;
        const Tile_triangulation& ctri = c.triangulation();
        if (c.tile() == v.tile())
            for(int d = 0; d <= ctri.current_dimension(); ++d)
                if(ctri.vertex(tc, d) == tv)
                    return true;
        const Tile_triangulation& vtri = v.triangulation();
        for(int d = 0; d <= ctri.current_dimension(); ++d)
            if(ctri.are_vertices_equal(ctri.vertex(tc, d), vtri, tv))
                return true;
        return false;
    }

    /// returns the index of the covertex of a facet f
    /// The operation is local iff the local cell of f is main
    inline int index_of_covertex(const Facet_iterator& f) const
    {
        assert(is_valid(f));
        const Tile_triangulation& tri = f.triangulation();
        if (tri.cell_is_main(tri.cell(*f)))
            return local_index_of_covertex(f);
        return local_index_of_covertex(relocate(f, id(cell(f))));
    }

    /// returns the covertex of a facet f
    /// The operation is local iff the local cell of f is not foreign
    Vertex_iterator covertex(const Facet_iterator& f) const
    {
        assert(is_valid(f));
        const Tile_triangulation& tri = f.triangulation();
        Tile_cell_index c = tri.cell(*f);
        if(tri.cell_is_foreign(c)) return local_covertex(main(f)); // any non foreign representative could do
        return Vertex_iterator(&tiles, f.tile(), tri.covertex(*f));
    }

    /// returns the mirror_vertex of a facet f, as the covertex of its mirror facet.
    /// The operation is local iff the local cell of the mirror facet of f is not foreign
    Vertex_iterator mirror_vertex(const Facet_iterator& f) const
    {
        return covertex(mirror_facet(f));
    }


    /// returns the facet defined by a cell and the index of its covertex.
    /// The operation is local iff the given cell is main, to ensure consistency across representatives of the cell.
    inline Facet_iterator facet(const Cell_iterator& c, int i) const
    {
        assert(is_valid(c));
        return local_facet(main(c),i); // i is defined wrt the main representative
    }

    /// returns the neighboring cell, opposite to the ith vertex.
    /// The operation may require to change tile twice : once if the given cell c is not main, and once if the mirror_facet of facet(main(c),i) is foreign.
    inline Cell_iterator neighbor(const Cell_iterator& c, int i) const
    {
        assert(is_valid(c));
        return cell(mirror_facet(facet(c, i)));
    }

    /// returns the mirror index of a cell, such that neighbor(neighbor(cell,i), mirror_index(cell,i))==cell .
    /// The operation is local if may require to change tile twice : once if the given cell c is not main, and once if the cell of mirror_facet of facet(main(c),i) is not main.
    inline int mirror_index(const Cell_iterator& c, int i) const
    {
        assert(is_valid(c));
        return mirror_index(facet(c,i));
    }
    /// @}

    /// \name Iterator Local operations
    /// @{
    /// \cgalAdvancedBegin
    /// The local_ functions are useful for advanced uses where the function calls can be done locally on the tile triangulation, without changing tile.
    /// This is more efficient if the operation is garanteed to be local.
    /// Functions that have an input or output vertex index yield different results if performed locally,
    /// as the vertex ordering in cells is not garanteed to be consistent across tiles. In some cases however, like
    /// iterating over the vertices of a cell, a globally consistent indexing of the cell vertices may not be required
    /// and the local indexing of the local functions may be used instead for better performance.
    /// \cgalAdvancedEnd

    /// returns the ith vertex of the cell c in its local tile.
    /// Advanced use: Access is local, thus more more effective, but the vertex index i corresponds
    /// to the index in the local tile representative of cell c, which may not be consistent with
    /// the vertex ordering in its main representative.
    Vertex_iterator local_vertex (const Cell_iterator& c, const int i) const
    {
        assert(is_valid(c));
        return Vertex_iterator(&tiles, c.tile(), c.triangulation().vertex(*c, i));
    }

    /// returns the index of the covertex of f in its local cell
    /// Advanced use: Access is local, thus more more effective, but the returned index relates to
    /// the local cell incident to f. The vertex ordering of the local cell may not correspond to the one of
    /// its main representative.
    /// Precondition: the local cell of f is not foreign.
    inline int local_index_of_covertex(const Facet_iterator& f) const
    {
        assert(is_valid(f));
        return f.triangulation().index_of_covertex(*f);
    }

    /// constructs a facet locally given a cell and a local index i
    /// Advanced use: Access is local, thus more more effective, but the returned facet is defined using the local index i,
    /// which may indexing of the main representative of the cell c.
    Facet_iterator local_facet(const Cell_iterator& c, int i) const
    {
        assert(is_valid(c));
        return Facet_iterator(&tiles, c.tile(), c.triangulation().facet(*c, i));
    }

    /// returns the index of the mirror vertex of f locally
    /// Advanced use: Access is local, thus more more effective, but the returned index relates to the local indexing of the local cell of the mirror of f
    /// Precondition : the local cell of the mirror of f should not be foreign
    inline int local_mirror_index(const Facet_iterator& f) const
    {
        assert(is_valid(f));
        const Tile_triangulation& tri= f.triangulation();
        Tile_cell_index c = tri.cell(*f);
        assert(!tri.cell_is_foreign(c));
        return tri.mirror_index(c,tri.index_of_covertex(*f));
    }

    /// returns the full cell that is adjacent to the input facet f and that joins the covertex with the vertices of f
    /// Advanced use: Access is local, thus more more effective, but assumes that the local cell of f is not foreign.
    Cell_iterator local_cell(const Facet_iterator& f) const
    {
        assert(is_valid(f));
        const Tile_triangulation& tri= f.triangulation();
        Tile_cell_index c = tri.cell(*f);
        assert(!tri.cell_is_foreign(c));
        return Cell_iterator(&tiles, f.tile(), c);
    }

    /// @returns the covertex of the input facet f
    /// Advanced use: Access is local, thus more more effective, but assumes that the local cell of f is not foreign.
    Vertex_iterator local_covertex(const Facet_iterator& f) const
    {
        assert(is_valid(f));
        const Tile_triangulation& tri = f.triangulation();
        Tile_cell_index c = tri.cell(*f);
        assert(!tri.cell_is_foreign(c));
        return Vertex_iterator(&tiles, f.tile(), tri.covertex(*f));
    }
    /// @}


    /// \ingroup PkgDDTInsert
    /// Triangulate the points into the tile container, so that each of its tile triangulations contain a local view of the overall
    /// triangulation of all inserted points.
    /// The scheduler provides the distribution environment (single thread, multithread, MPI...)
    /// @returns the number of newly inserted vertices
    template<typename Scheduler, typename Point, typename TileIndex, typename TilePoints>
    std::size_t insert(Scheduler& sch, CGAL::Distributed_point_set<Point, TileIndex, TilePoints>& point_sets){
        std::size_t n = number_of_finite_vertices();
        std::cout << std::endl << "---insert_and_send_all_axis_extreme_points---" << std::endl;
        CGAL::DDT::impl::insert_and_send_all_axis_extreme_points(tiles, point_sets, sch, maximal_dimension());
        std::cout << std::endl << "---splay_stars---" << std::endl;
        CGAL::DDT::impl::splay_stars(tiles, point_sets, sch, maximal_dimension());
        std::cout << std::endl << "---finalize---" << std::endl;
        finalize(sch);
        std::cout << std::endl << "---inserted---" << std::endl;
        return number_of_finite_vertices() - n;
    }


    /// \ingroup PkgDDTInsert
    /// Inserts the given point in the tile given by the given id, in the Delaunay triangulation stored in the tile container.
    /// The scheduler provides the distribution environment (single thread, multithread, MPI...)
    /// @returns 1 if a new vertex has been inserted, 0 if it was already in the inserted.
    /// @todo returns a descritor to the inserted vertex and a bool ?
    template<typename Scheduler, typename Point, typename Tile_index>
    typename std::size_t insert(Scheduler& sch, const Point& point, Tile_index id){
        Distributed_point_set<Point, Tile_index> point_set;
        point_set[id].send_point(id,id,point);
        return insert(sch, point_set);
    }

    void get_adjacency_graph(std::unordered_multimap<Tile_index,Tile_index>& edges) const
    {
        for(const auto& [id, tile] : tiles)
        {
            std::set<Tile_index> out_edges;
            tile.get_adjacency_graph_edges(out_edges);
            Tile_index source = id;
            for(Tile_index target : out_edges)
                edges.emplace(source, target);
        }
    }

    bool is_adjacency_graph_symmetric() const
    {
        std::unordered_multimap<Tile_index,Tile_index> edges;
        std::unordered_multimap<Tile_index,Tile_index> reversed;
        get_adjacency_graph(edges);
        for(auto& edge : edges)
            reversed.emplace(edge.second, edge.first);
        return edges == reversed;
    }

    template <typename Scheduler>
    void finalize(Scheduler& sch)
    {
        Statistics stats;
        statistics_ = sch.transform_reduce(tiles, stats,
            [](Tile_index id, const Tile_triangulation& tri) { return tri.statistics();});
    }

    template <typename Scheduler, typename Serializer>
    bool write(Scheduler& sch,  const Serializer& serializer) {
        if (!serializer.write_begin(*this)) return false;
        if (!sch.transform_reduce(tiles, true,
            [&serializer](Tile_index id, const Tile_triangulation& tri) { return serializer.write(tri);},
            std::logical_and<>()))
            return false;
        return serializer.write_end(*this);
    }

    template <typename Scheduler, typename Serializer>
    bool read(Scheduler& sch,  const Serializer& serializer) {
        if (!serializer.read_begin(*this)) return false;
        if (!sch.transform_reduce(tiles, true,
            [&serializer](Tile_index id, Tile_triangulation& tri) { return serializer.read(tri);},
            std::logical_and<>()))
            return false;
        return serializer.read_end(*this);
    }

    const Statistics& statistics() const { return statistics_; }
    Statistics& statistics() { return statistics_; }

    Container tiles;

    private:
    Statistics statistics_;
    int maximal_dimension_;
};

}

#endif // CGAL_DISTRIBUTED_TRIANGULATION_H
