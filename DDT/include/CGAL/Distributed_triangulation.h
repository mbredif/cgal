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

#include <CGAL/DDT/iterator/Vertex_const_iterator.h>
#include <CGAL/DDT/iterator/Facet_const_iterator.h>
#include <CGAL/DDT/iterator/Cell_const_iterator.h>
#include <CGAL/DDT/insert.h>
#include <CGAL/DDT/Messaging.h>
#include <CGAL/DDT/Messaging_container.h>
#include <CGAL/DDT/Tile_container.h>

namespace CGAL {

/// \ingroup PkgDDTRef
/// \tparam template params to instantiate a `CGAL::DDT::Tile_container` that manages the storage of the triangulation tiles.
/// The Distributed_triangulation class wraps a TileContainer to expose a triangulation interface.
template<typename Triangulation_,
         typename TileIndexProperty_,
         typename Serializer_ = CGAL::DDT::No_serializer<Triangulation_, TileIndexProperty_> >
class Distributed_triangulation
{
public:
    typedef Triangulation_                                    Triangulation;
    typedef TileIndexProperty_                                TileIndexProperty;
    typedef Serializer_                                       Serializer;
    typedef CGAL::DDT::Tile_container<Triangulation_, TileIndexProperty_, Serializer_> TileContainer;

private:
    typedef typename TileContainer::Tile                Tile;
    typedef typename TileContainer::Tile_index          Tile_index;
    typedef typename TileContainer::iterator            Tile_iterator;
    typedef typename TileContainer::const_iterator      Tile_const_iterator;
    typedef typename TileContainer::Tile_triangulation  Tile_triangulation;

    typedef CGAL::DDT::Triangulation_traits<Triangulation>    Traits;
    typedef typename Traits::Vertex_index               Tile_vertex_index;
    typedef typename Traits::Cell_index                 Tile_cell_index;
    typedef typename Traits::Facet_index                Tile_facet_index;
    typedef typename Traits::Point                      Point;

    typedef CGAL::DDT::Messaging_container<CGAL::DDT::Messaging<Tile_index, Point>> Messaging_container;

public:
/// \name Types
/// @{

#ifndef DOXYGEN_RUNNING
    typedef CGAL::DDT::Vertex_const_iterator<TileContainer> Vertex_const_iterator;
    typedef CGAL::DDT::Facet_const_iterator <TileContainer> Facet_const_iterator;
    typedef CGAL::DDT::Cell_const_iterator  <TileContainer> Cell_const_iterator;
#else
    /// A const iterator to the vertices of a distributed Delaunay triangulation
    typedef unspecified_type Vertex_const_iterator;
    /// A const iterator to the facets of a distributed Delaunay triangulation
    typedef unspecified_type Facet_const_iterator;
    /// A const iterator to the cells of a distributed Delaunay triangulation
    typedef unspecified_type Cell_const_iterator;
#endif
/// @}

    /// contructor
    Distributed_triangulation(int dimension = Traits::D, std::size_t number_of_triangulations_mem_max = 0, const Serializer& serializer = Serializer())
    : tiles(dimension, number_of_triangulations_mem_max, serializer) {}

    /// returns the dimension of the triangulation
    inline int maximal_dimension() const { return tiles.maximal_dimension(); }
    /// returns the number of finite cells in the triangulation, including cells incident to the vertex at infinity.
    inline std::size_t number_of_finite_cells   () const { return tiles.number_of_finite_cells();    }
    /// returns the number of finite vertices in the triangulation, including the vertex at infinity.
    inline std::size_t number_of_finite_vertices() const { return tiles.number_of_finite_vertices(); }
    /// returns the number of facets in the triangulation, including facets incident to the vertex at infinity.
    inline std::size_t number_of_finite_facets  () const { return tiles.number_of_finite_facets();   }
    /// returns the number of finite cells in the triangulation.
    inline std::size_t number_of_cells   () const { return tiles.number_of_cells();    }
    /// returns the number of finite vertices in the triangulation.
    inline std::size_t number_of_vertices() const { return tiles.number_of_vertices(); }
    /// returns the number of finite facets in the triangulation.
    inline std::size_t number_of_facets  () const { return tiles.number_of_facets();   }



    /// \name Iterators
    /// @{

    /// returns a const iterator at the start of the range of finite vertices.
    Vertex_const_iterator vertices_begin() const { return Vertex_const_iterator(&tiles, tiles.cbegin()); }
    /// returns a const iterator past the end of the range of finite vertices.
    Vertex_const_iterator vertices_end  () const { return Vertex_const_iterator(&tiles, tiles.cend()); }

    /// returns a const iterator at the start of the range of finite cells.
    Cell_const_iterator cells_begin() const { return Cell_const_iterator(&tiles, tiles.cbegin()); }
    /// returns a const iterator past the end of the range of finite cells.
    Cell_const_iterator cells_end  () const { return Cell_const_iterator(&tiles, tiles.cend()); }

    /// returns a const iterator at the start of the range of finite facets.
    Facet_const_iterator facets_begin() const { return Facet_const_iterator(&tiles, tiles.cbegin()); }
    /// returns a const iterator past the end of the range of finite facets.
    Facet_const_iterator facets_end  () const { return Facet_const_iterator(&tiles, tiles.cend()); }

    /// @}

    /// \name Global Identifiers
    /// @{

    /// returns a global id of the vertex iterator using its distance to vertices_begin. (implementation is linear in the returned id)
    int vertex_id(Vertex_const_iterator v) const
    {
        if (is_infinite(v)) return -1;
        return std::distance(vertices_begin(), main(v));
    }

    /// returns a global id of the cell iterator using its distance to cells_begin. (implementation is linear in the returned id)
    int cell_id(Cell_const_iterator c) const
    {
        return std::distance(cells_begin(), main(c));
    }
    /// @}

    /// checks the validity of the Distributed_triangulation
    bool is_valid(bool verbose = false, int level = 0) const
    {
        if (!tiles.is_valid(verbose, level)) return false;
        for(const Tile& tile : tiles)
        {
            const typename Tile::Tile_triangulation& dt = tile.triangulation();
            for(Tile_vertex_index v = dt.vertices_begin(); v != dt.vertices_end(); ++v)
            {
                assert(dt.vertex_is_infinite(v) || (dt.vertex_is_local(v) + dt.vertex_is_foreign(v) == 1));
                if(dt.vertex_is_infinite(v)) continue;
                Tile_index tid = dt.vertex_id(v);
                if(tid == dt.id()) continue;
                Tile_const_iterator t = tiles.find(tid);
                const Tile_triangulation& dt2 = t->triangulation();
                if(dt2.relocate_vertex(dt, v) == dt2.vertices_end())
                {
                    assert(! "relocate_vertex failed" );
                    return false;
                }
            }
            for(Tile_facet_index f = dt.facets_begin(); f != dt.facets_end(); ++f)
            {
                assert(dt.facet_is_local(f) + dt.facet_is_mixed(f) + dt.facet_is_foreign(f) == 1);
                if(!dt.facet_is_mixed(f)) continue;
                std::set<Tile_index> tids;
                for(int d = 0; d <= dt.current_dimension(); ++d)
                {
                    if(d==dt.index_of_covertex(f)) continue;
                    Tile_cell_index c = dt.cell(f);
                    Tile_vertex_index v = dt.vertex(c, d);
                    if(dt.vertex_is_infinite(v)) continue;
                    Tile_index tid = dt.vertex_id(v);
                    if(tid == dt.id()) continue;
                    tids.insert(tid);
                }
                for(Tile_index tid : tids)
                {
                    Tile_const_iterator t = tiles.find(tid);
                    const Tile_triangulation& dt2 = t->triangulation();
                    if(dt2.relocate_facet(dt, f) == dt2.facets_end())
                    {
                      assert(! "relocate_facet failed" );
                      return false;
                    }
                }

            }
            for(Tile_cell_index c = dt.cells_begin(); c != dt.cells_end(); ++c)
            {
                assert(dt.cell_is_local(c) + dt.cell_is_mixed(c) + dt.cell_is_foreign(c) == 1);
                if(!dt.cell_is_mixed(c)) continue;
                std::set<Tile_index> tids;
                for(int d = 0; d <= dt.current_dimension(); ++d)
                {
                    Tile_vertex_index v = dt.vertex(c, d);
                    if(dt.vertex_is_infinite(v)) continue;
                    Tile_index tid = dt.vertex_id(v);
                    if(tid == dt.id()) continue;
                    tids.insert(tid);
                }
                for(Tile_index tid : tids)
                {
                    Tile_const_iterator t = tiles.find(tid);
                    const Tile_triangulation& dt2 = t->triangulation();
                    if(dt2.relocate_cell(dt, c) == dt2.cells_end())
                    {
                      assert(! "relocate_facet failed" );
                        return false;
                    }
                }
            }
        }
        return true;
    }

    /// \name Iterator tests
    /// @{

    bool is_local(const Vertex_const_iterator& v) const { return v.triangulation().vertex_is_local(*v); }
    bool is_local(const Facet_const_iterator&  f) const { return f.triangulation().facet_is_local(*f); }
    bool is_local(const Cell_const_iterator&   c) const { return c.triangulation().cell_is_local(*c); }

    bool is_valid(const Vertex_const_iterator& v) const { return v.triangulation().vertex_is_valid(*v); } // + tile toujours chargée ?
    bool is_valid(const Facet_const_iterator&  f) const { return f.triangulation().facet_is_valid(*f); } // + tile toujours chargée ?
    bool is_valid(const Cell_const_iterator&   c) const { return c.triangulation().cell_is_valid(*c); } // + tile toujours chargée ?

    // vertices are never mixed
    bool is_mixed(const Facet_const_iterator& f) const { return f.triangulation().facet_is_mixed(*f); }
    bool is_mixed(const Cell_const_iterator&  c) const { return c.triangulation().cell_is_mixed(*c); }

    bool is_foreign(const Vertex_const_iterator& v) const { return v.triangulation().vertex_is_foreign(*v); }
    bool is_foreign(const Facet_const_iterator&  f) const { return f.triangulation().facet_is_foreign(*f); }
    bool is_foreign(const Cell_const_iterator&   c) const { return c.triangulation().cell_is_foreign(*c); }

    bool is_main(const Vertex_const_iterator& v) const { return v.triangulation().vertex_is_main(*v); }
    bool is_main(const Facet_const_iterator&  f) const { return f.triangulation().facet_is_main(*f); }
    bool is_main(const Cell_const_iterator&   c) const { return c.triangulation().cell_is_main(*c); }

    bool is_infinite(const Vertex_const_iterator& v) const { return v.triangulation().vertex_is_infinite(*v); }
    bool is_infinite(const Facet_const_iterator&  f) const { return f.triangulation().facet_is_infinite(*f); }
    bool is_infinite(const Cell_const_iterator&   c) const { return c.triangulation().cell_is_infinite(*c); }
    /// @}

    /// \name Tile identifiers from iterators
    /// @{
    Tile_index id(const Vertex_const_iterator&v) const { return v.triangulation().vertex_id(*v); }
    Tile_index id(const Facet_const_iterator& f) const { return f.triangulation().facet_id(*f); }
    Tile_index id(const Cell_const_iterator&  c) const { return c.triangulation().cell_id(*c); }

    Tile_index tile_id(const Vertex_const_iterator& v) const { return v.tile()->id(); }
    Tile_index tile_id(const Facet_const_iterator&  f) const { return f.tile()->id(); }
    Tile_index tile_id(const Cell_const_iterator&   c) const { return c.tile()->id(); }
    /// @}

    /// \name Iterator relocation
    /// @{
    /// `relocate` functions return an alternative iterator that represents the same simplex,
    /// but that lives in the tile with the provided Tile_index.
    /// If the simplex is not represented there, the end iterator is returned

    /// returns a vertex iterator equivalent to v in tile id. They represent the same vertex of the global triangulation.
    Vertex_const_iterator relocate(const Vertex_const_iterator& v, Tile_index id) const
    {
        assert(is_valid(v));
        if (id == tile_id(v)) return v; // v is already in tile id
        Tile_const_iterator tile = tiles.find(id);
        if (tile == tiles.end()) return vertices_end();
        Tile_vertex_index vertex = tile->triangulation().relocate_vertex(v.triangulation(), *v);
        if (vertex==tile->triangulation().vertices_end()) return vertices_end();
        return Vertex_const_iterator(&tiles, tile, vertex);
    }

    /// returns a facet iterator equivalent to f in tile id. They represent the same facet of the global triangulation.
    Facet_const_iterator relocate(const Facet_const_iterator& f, Tile_index id) const
    {
        assert(is_valid(f));
        if (id == tile_id(f)) return f; // f is already in tile id
        Tile_const_iterator tile = tiles.find(id);
        if (tile == tiles.end()) return facets_end();
        Tile_facet_index facet = tile->triangulation().relocate_facet(f.triangulation(), *f);
        if (facet==tile->triangulation().facets_end()) return facets_end();
        return Facet_const_iterator(&tiles, tile, facet);
    }

    /// returns a cell iterator equivalent to c in tile id. They represent the same cell of the global triangulation.
    Cell_const_iterator relocate(const Cell_const_iterator& c, Tile_index id) const
    {
        assert(is_valid(c));
        if (id == tile_id(c)) return c; // c is already in tile id
        Tile_const_iterator tile = tiles.find(id);
        if (tile == tiles.end()) return cells_end();
        Tile_cell_index cell = tile->triangulation().relocate_cell(c.triangulation(), *c);
        if (cell==tile->triangulation().cells_end()) return cells_end();
        return Cell_const_iterator(&tiles, tile, cell);
    }

    /// returns the main representative of a vertex iterator
    inline Vertex_const_iterator main(const Vertex_const_iterator& v) const { return relocate(v, id(v)); }
    /// returns the main representative of a facet iterator
    inline Facet_const_iterator main(const Facet_const_iterator& f) const { return relocate(f, id(f)); }
    /// returns the main representative of a cell iterator
    inline Cell_const_iterator main(const Cell_const_iterator& c) const { return relocate(c, id(c)); }

    /// @}


    /// \name Iterator operations
    /// @{

    /// returns a representative iterator for the infinite vertex
    /// precondition : at least one tile is loaded.
    inline Vertex_const_iterator infinite_vertex() const
    {
        assert(!tiles.empty());
        Tile_const_iterator tile = tiles.cbegin();
        return Vertex_const_iterator(&tiles, tile, tile->triangulation().infinite_vertex());
    }

    /// returns the ith vertex of cell c.
    /// Indexing by i is consistent over all representatives of cell c, as its main representative is looked up.
    Vertex_const_iterator vertex (const Cell_const_iterator& c, const int i) const
    {
        assert(is_valid(c));
        return local_vertex(main(c), i); // i is defined wrt the main representative
    }

    /// returns the point embedding of the vertex.
    /// This can be done locally without considering the main tile of the vertex, as point coordinates
    /// are replicated in all tiles.
    const Point& point(Vertex_const_iterator v) const
    {
        assert(is_valid(v));
        return v.triangulation().point(*v);
    }

    /// returns the mirror facet. This operation is performed locally: the resulting facet belongs
    /// to the same tile as the input facet.
    /// Precondition: the facet f is valid (ie: at least one of the facet points, the covertex and the mirror vertex is local)
    Facet_const_iterator mirror_facet(const Facet_const_iterator& f) const
    {
        assert(is_valid(f));
        Tile_const_iterator tile = f.tile();
        assert(tile->triangulation().facet_is_valid(*f));
        return Facet_const_iterator(&tiles, tile, tile->triangulation().mirror_facet(*f));
    }

    /// returns the mirror index of facet f, such that neighbor(cell(mirror_facet(f)), mirror_index)==cell(f)
    /// The index of covertex of the main version of the mirror facet of f is considered, as indices may not be consistent across
    /// the representatives of mirror facet in other tiles.
    inline int mirror_index(const Facet_const_iterator& f) const
    {
        assert(is_valid(f));
        return index_of_covertex(mirror_facet(f));
    }

    /// returns the full cell that is incident to the input facet f and that joins the covertex with the vertices of f
    /// The operation is local iff the local cell of f is not foreign.
    Cell_const_iterator cell(const Facet_const_iterator& f) const
    {
        assert(is_valid(f));
        Tile_const_iterator tile = f.tile();
        Tile_cell_index c = tile->triangulation().cell(*f);
        if(tile->triangulation().cell_is_foreign(c)) return local_cell(main(f)); // any non foreign representative could do
        return Cell_const_iterator(&tiles, tile, c);
    }

    /// returns one of the full cells that is incident to the input vertex v. The operation is local
    Cell_const_iterator cell(const Vertex_const_iterator& v) const
    {
        Tile_const_iterator tile = v.tile();
        const Tile_triangulation& triangulation = tile->triangulation();
        Tile_vertex_index tv = *v;
        Tile_cell_index tc = triangulation.cell(tv);
        if(!triangulation.cell_is_foreign(tc))
            return Cell_const_iterator(&tiles, tile, tc);

        std::vector<Tile_cell_index> cells;
        triangulation.incident_cells(tv, std::back_inserter(cells));
        for(Tile_cell_index c: cells)
            if(!triangulation.cell_is_foreign(c))
                return Cell_const_iterator(&tiles, tile, c);
        assert(false); // all incident cells are foreign, v should have been simplified !
        return cells_end();
    }

    /// returns whether vertex v is incident to cell c. The operation is local in the tile of c
    bool has_vertex(const Cell_const_iterator& c, const Vertex_const_iterator& v) const
    {
        Tile_const_iterator ctile = c.tile();
        Tile_const_iterator vtile = v.tile();
        Tile_cell_index tc = *c;
        Tile_vertex_index tv = *v;
        const Tile_triangulation& ctriangulation = ctile->triangulation();
        if (ctile == vtile)
            for(int d = 0; d <= ctriangulation.current_dimension(); ++d)
                if(ctriangulation.vertex(tc, d) == tv)
                    return true;
        for(int d = 0; d <= ctriangulation.current_dimension(); ++d)
            if(ctriangulation.are_vertices_equal(ctriangulation.vertex(tc, d), vtile->triangulation(), tv))
                return true;
        return false;
    }

    /// returns the index of the covertex of a facet f
    /// The operation is local iff the local cell of f is main
    inline int index_of_covertex(const Facet_const_iterator& f) const
    {
        assert(is_valid(f));
        Tile_const_iterator tile = f.tile();
        const Tile_triangulation& triangulation = tile->triangulation();
        Tile_cell_index c = triangulation.cell(*f);
        if(triangulation.cell_is_main(c)) return local_index_of_covertex(f);
        return local_index_of_covertex(relocate(f, triangulation.cell_id(c)));
    }

    /// returns the covertex of a facet f
    /// The operation is local iff the local cell of f is not foreign
    Vertex_const_iterator covertex(const Facet_const_iterator& f) const
    {
        assert(is_valid(f));
        Tile_const_iterator tile = f.tile();
        const Tile_triangulation& triangulation = tile->triangulation();
        Tile_cell_index c = triangulation.cell(*f);
        if(triangulation.cell_is_foreign(c)) return local_covertex(main(f)); // any non foreign representative could do
        return Vertex_const_iterator(&tiles, tile, triangulation.covertex(*f));
    }

    /// returns the mirror_vertex of a facet f, as the covertex of its mirror facet.
    /// The operation is local iff the local cell of the mirror facet of f is not foreign
    Vertex_const_iterator mirror_vertex(const Facet_const_iterator& f) const
    {
        return covertex(mirror_facet(f));
    }


    /// returns the facet defined by a cell and the index of its covertex.
    /// The operation is local iff the given cell is main, to ensure consistency across representatives of the cell.
    inline Facet_const_iterator facet(const Cell_const_iterator& c, int i) const
    {
        assert(is_valid(c));
        return local_facet(main(c),i); // i is defined wrt the main representative
    }

    /// returns the neighboring cell, opposite to the ith vertex.
    /// The operation may require to change tile twice : once if the given cell c is not main, and once if the mirror_facet of facet(main(c),i) is foreign.
    inline Cell_const_iterator neighbor(const Cell_const_iterator& c, int i) const
    {
        assert(is_valid(c));
        return cell(mirror_facet(facet(c, i)));
    }

    /// returns the mirror index of a cell, such that neighbor(neighbor(cell,i), mirror_index(cell,i))==cell .
    /// The operation is local if may require to change tile twice : once if the given cell c is not main, and once if the cell of mirror_facet of facet(main(c),i) is not main.
    inline int mirror_index(const Cell_const_iterator& c, int i) const
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
    Vertex_const_iterator local_vertex (const Cell_const_iterator& c, const int i) const
    {
        assert(is_valid(c));
        Tile_const_iterator tile = c.tile();
        return Vertex_const_iterator(&tiles, tile, tile->triangulation().vertex(*c, i));
    }

    /// returns the index of the covertex of f in its local cell
    /// Advanced use: Access is local, thus more more effective, but the returned index relates to
    /// the local cell incident to f. The vertex ordering of the local cell may not correspond to the one of
    /// its main representative.
    /// Precondition: the local cell of f is not foreign.
    inline int local_index_of_covertex(const Facet_const_iterator& f) const
    {
        assert(is_valid(f));
        Tile_const_iterator tile = f.tile();
        return tile->triangulation().index_of_covertex(*f);
    }

    /// constructs a facet locally given a cell and a local index i
    /// Advanced use: Access is local, thus more more effective, but the returned facet is defined using the local index i,
    /// which may indexing of the main representative of the cell c.
    Facet_const_iterator local_facet(const Cell_const_iterator& c, int i) const
    {
        assert(is_valid(c));
        Tile_const_iterator tile = c.tile();
        return Facet_const_iterator(&tiles, tile, tile->triangulation().facet(*c, i));
    }

    /// returns the index of the mirror vertex of f locally
    /// Advanced use: Access is local, thus more more effective, but the returned index relates to the local indexing of the local cell of the mirror of f
    /// Precondition : the local cell of the mirror of f should not be foreign
    inline int local_mirror_index(const Facet_const_iterator& f) const
    {
        assert(is_valid(f));
        Tile_const_iterator tile = f.tile();
        const Tile_triangulation& triangulation = tile->triangulation();
        Tile_cell_index c = triangulation.cell(*f);
        assert(!triangulation.cell_is_foreign(c));
        return tile->mirror_index(c,triangulation.index_of_covertex(*f));
    }

    /// returns the full cell that is adjacent to the input facet f and that joins the covertex with the vertices of f
    /// Advanced use: Access is local, thus more more effective, but assumes that the local cell of f is not foreign.
    Cell_const_iterator local_cell(const Facet_const_iterator& f) const
    {
        assert(is_valid(f));
        Tile_const_iterator tile = f.tile();
        Tile_cell_index c = tile->triangulation().cell(*f);
        assert(!tile->triangulation().cell_is_foreign(c));
        return Cell_const_iterator(&tiles, tile, c);
    }

    /// @returns the covertex of the input facet f
    /// Advanced use: Access is local, thus more more effective, but assumes that the local cell of f is not foreign.
    Vertex_const_iterator local_covertex(const Facet_const_iterator& f) const
    {
        assert(is_valid(f));
        Tile_const_iterator tile = f.tile();
        Tile_cell_index c = tile->triangulation().cell(*f);
        assert(!tile->triangulation().cell_is_foreign(c));
        return Vertex_const_iterator(&tiles, tile, tile->triangulation().covertex(*f));
    }
    /// @}


    /// \ingroup PkgDDTInsert
    /// Triangulate the points into the tile container, so that each of its tile triangulations contain a local view of the overall
    /// triangulation of all inserted points.
    /// The scheduler provides the distribution environment (single thread, multithread, MPI...)
    /// @returns the number of newly inserted vertices
    template<typename Scheduler, typename Messaging>
    std::size_t insert(Scheduler& sch, CGAL::DDT::Messaging_container<Messaging>& messagings){
        std::size_t n = tiles.number_of_finite_vertices();
        CGAL::DDT::impl::insert_and_send_all_axis_extreme_points(tiles, messagings, sch);
        CGAL::DDT::impl::splay_stars(tiles, messagings, sch);
        tiles.finalize(); /// @todo : return 0 for unloaded tiles
        return tiles.number_of_finite_vertices() - n;
    }


    /// \ingroup PkgDDTInsert
    /// Inserts the given point in the tile given by the given id, in the Delaunay triangulation stored in the tile container.
    /// The scheduler provides the distribution environment (single thread, multithread, MPI...)
    /// @returns 1 if a new vertex has been inserted, 0 if it was already in the inserted.
    /// @todo returns a descritor to the inserted vertex and a bool ?
    template<typename Scheduler, typename Point, typename Tile_index>
    typename std::size_t insert(Scheduler& sch, const Point& point, Tile_index id){
        Messaging_container messaging;
        messaging[id].send_point(id,id,point);
        return insert(sch, messaging);
    }

    /// \ingroup PkgDDTInsert
    /// inserts the points of the provided point+id range in the tiles given by the given ids, in the Delaunay triangulation stored in the tile container.
    /// The scheduler provides the distribution environment (single thread, multithread, MPI...)
    /// @returns the number of newly inserted vertices
    template<typename Scheduler, typename PointIndexRange>
    std::size_t insert(Scheduler& sch, const PointIndexRange& range) {
        Messaging_container messaging;
        for (auto& p : range)
            messaging[p.first].send_point(p.first,p.first,p.second);
        return insert(sch, messaging);
    }

    /// \ingroup PkgDDTInsert
    /// inserts the points of the provided point range in the tiles given by the partitioning function, in the Delaunay triangulation stored in the tile container.
    /// The scheduler provides the distribution environment (single thread, multithread, MPI...)
    /// @returns the number of newly inserted vertices
    template<typename Scheduler, typename PointRange, typename Partitioner>
    std::size_t insert(Scheduler& sch, const PointRange& points, Partitioner& part) {
        Messaging_container messaging;
        for(const auto& p : points)  {
            typename Partitioner::Tile_index id = part(p);
            messaging[id].send_point(id,id,p);
        }
        return insert(sch, messaging);
    }

    /// \ingroup PkgDDTInsert
    /// inserts the points of the provided point range in the tiles given by the partitioning function, in the Delaunay triangulation stored in the tile container.
    /// The scheduler provides the distribution environment (single thread, multithread, MPI...)
    /// @returns the number of newly inserted vertices
    template<typename Scheduler, typename Iterator, typename Partitioner>
    std::size_t insert(Scheduler& sch, Iterator it, int count, Partitioner& part) {
    #if __cplusplus >= 202002L
        // using c++20 and #include <ranges>
        return insert(sch, std::views::counted(it, count), part);
    #else
        Messaging_container messaging;
        for(; count; --count, ++it) {
            auto p(*it);
            Tile_index id = part(p);
            messaging[id].send_point(id,id,p);
        }
        return insert(sch, messaging);
    #endif
    }

    TileContainer tiles; /// underlying tile container
};

}

#endif // CGAL_DISTRIBUTED_TRIANGULATION_H
