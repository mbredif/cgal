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
#include <CGAL/DDT/point_set/Pair_container_point_set.h> // for Internal_property_map and Point_set_with_id
#include <CGAL/Distributed_point_set.h>
#include <CGAL/DDT/insert.h>
#include <CGAL/DDT/Tile_triangulation.h>
#include <CGAL/DDT/Tile_container.h>
#include <CGAL/DDT/serializer/No_serializer.h>
#include <CGAL/DDT/serializer/VRT_file_serializer.h>
#include <CGAL/DDT/IO/trace_logger.h>
#include <CGAL/assertions.h>

namespace CGAL {

/// \ingroup PkgDDTRef
/// The Distributed_triangulation class wraps a Container to expose a triangulation interface.
/// \tparam Triangulation model of `Triangulation`, which stores local triangulations.
/// \tparam TileIndexProperty model of `VertexPropertyMap`, which provides access to the tile index of the triangulation vertices.
/// \tparam Serializer model of `Serializer`. If not provided, serialization is disabled and computations are performed in core using `CGAL::DDT::No_serializer`
/// \todo missing doc for public types that you want to expose
/// \todo brief should be improved
template<typename Triangulation_,
         typename TileIndexProperty_,
         typename Serializer = CGAL::DDT::No_serializer >
class Distributed_triangulation
{
public:
    /// the local `Triangulation` type
    using Triangulation        = Triangulation_;
    /// the `VertexPropertyMap` type
    using TileIndexProperty    = TileIndexProperty_;

    /// the type for Tile indices
    using Tile_index           = typename TileIndexProperty::value_type;

    /// the type for Tile data, wrapping the local trangulation and the tile index property.
    using Tile_triangulation   = CGAL::DDT::Tile_triangulation<Triangulation, TileIndexProperty>;

private:
    using AssociativeContainer = std::map<Tile_index, Tile_triangulation>; // unordered_map is not suitable as its iterators may get invalidated by try_emplace

public:
    /// the associative container type that maps `Tile_index` to `Tile_triangulation`
    using Container = std::conditional_t<
        std::is_same_v<Serializer, CGAL::DDT::No_serializer>,              // No serialization ?
        AssociativeContainer ,                                             // y: tiles are kept in memory
        CGAL::DDT::Tile_container<AssociativeContainer, Serializer> >;     // n: using serialization and a tile container

private:
    typedef typename Container::iterator            Tile_iterator;
    typedef typename Container::const_iterator      Tile_const_iterator;

    typedef CGAL::DDT::Statistics Statistics;
    typedef CGAL::DDT::Triangulation_traits<Triangulation>    Traits;
    typedef typename Traits::Vertex_index               Tile_vertex_index;
    typedef typename Traits::Cell_index                 Tile_cell_index;
    typedef typename Traits::Facet_index                Tile_facet_index;
    typedef typename Traits::Point                      Point;
    typedef typename Traits::Point_const_reference      Point_const_reference;

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
    Distributed_triangulation(int dim, TileIndexProperty tile_indices = {}, Args&&... args)
    :   maximal_dimension_(dim),
        tiles(std::forward<Args>(args)...),
        tile_indices(tile_indices),
        statistics_()
    {}

    /// returns the ambient dimension of the triangulation
    inline int maximal_dimension() const { return maximal_dimension_; }
    /// returns the dimension of the triangulation
    /// \todo MB: It should be non mutable, but read_cgal_json needs to modify it when the dimension is dynamic
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

    /// returns a global id of the vertex iterator using its distance to `vertices_begin` (implementation is linear in the returned id)
    int vertex_id(Vertex_iterator v) const
    {
        if (is_infinite(v)) return -1;
        return std::distance(vertices_begin(), main(v));
    }

    /// returns a global id of the cell iterator using its distance to `cells_begin` (implementation is linear in the returned id)
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
                CGAL_assertion(tri.vertex_is_infinite(v) || (tri.vertex_is_local(v) + tri.vertex_is_foreign(v) == 1));
                if(tri.vertex_is_infinite(v)) continue;
                Tile_index tid = tri.vertex_id(v);
                if(tid == tri.id()) continue;
                Tile_const_iterator t = tiles.find(tid);
                const Tile_triangulation& tri2 = t->second;
                if(tri2.relocate_vertex(tri, v) == tri2.vertices_end())
                {
                    CGAL_assertion(! "relocate_vertex failed" );
                    return false;
                }
            }
            for(Tile_facet_index f = tri.facets_begin(); f != tri.facets_end(); ++f)
            {
                CGAL_assertion(tri.facet_is_local(f) + tri.facet_is_mixed(f) + tri.facet_is_foreign(f) == 1);
                if(!tri.facet_is_mixed(f)) continue;
                std::set<Tile_index> tids;
                for(int d = 0; d <= tri.current_dimension(); ++d)
                {
                    if(d==tri.index_of_covertex(f)) continue;
                    Tile_cell_index c = tri.cell_of_facet(f);
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
                      CGAL_assertion(! "relocate_facet failed" );
                      return false;
                    }
                }

            }
            for(Tile_cell_index c = tri.cells_begin(); c != tri.cells_end(); ++c)
            {
                CGAL_assertion(tri.cell_is_local(c) + tri.cell_is_mixed(c) + tri.cell_is_foreign(c) == 1);
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
                      CGAL_assertion(! "relocate_facet failed" );
                        return false;
                    }
                }
            }

            if(!tri.is_valid(verbose, level))
            {
                std::cerr << "Tile " << std::to_string(id) << " is invalid" << std::endl;
                //CGAL_assertion(! "CGAL tile not valid" );
                return false;
            }
        }
        return true;
    }

    /// \name Iterator tests
    /// @{

    /// returns whether the vertex iterator is referring to a tile triangulation where the vertex is local.
    bool is_local(const Vertex_iterator& v) const { return v.triangulation().vertex_is_local(*v); }
    /// returns whether the facet iterator is referring to a tile triangulation where the facet is local.
    bool is_local(const Facet_iterator&  f) const { return f.triangulation().facet_is_local(*f); }
    /// returns whether the cell iterator is referring to a tile triangulation where the cell is local.
    bool is_local(const Cell_iterator&   c) const { return c.triangulation().cell_is_local(*c); }

    /// returns whether the vertex iterator is valid.
    /// \todo MB: should we check if tile is loaded ?
    bool is_valid(const Vertex_iterator& v) const { return v.triangulation().vertex_is_valid(*v); }
    /// returns whether the facet iterator is valid.
    /// \todo MB: should we check if tile is loaded ?
    bool is_valid(const Facet_iterator&  f) const { return f.triangulation().facet_is_valid(*f); }
    /// returns whether the cell iterator is valid.
    /// \todo MB: should we check if tile is loaded ?
    bool is_valid(const Cell_iterator&   c) const { return c.triangulation().cell_is_valid(*c); }

    /// returns whether the facet iterator is referring to a tile triangulation where the facet is mixed (neither local nor foreign).
    bool is_mixed(const Facet_iterator& f) const { return f.triangulation().facet_is_mixed(*f); }
    /// returns whether the cell iterator is referring to a tile triangulation where the cell is mixed (neither local nor foreign).
    bool is_mixed(const Cell_iterator&  c) const { return c.triangulation().cell_is_mixed(*c); }

    /// returns whether the vertex iterator is referring to a tile triangulation where the vertex is foreign.
    bool is_foreign(const Vertex_iterator& v) const { return v.triangulation().vertex_is_foreign(*v); }
    /// returns whether the facet iterator is referring to a tile triangulation where the facet is foreign.
    bool is_foreign(const Facet_iterator&  f) const { return f.triangulation().facet_is_foreign(*f); }
    /// returns whether the cell iterator is referring to a tile triangulation where the cell is foreign.
    bool is_foreign(const Cell_iterator&   c) const { return c.triangulation().cell_is_foreign(*c); }

    /// returns whether the vertex iterator is referring to a tile triangulation where the vertex is main.
    bool is_main(const Vertex_iterator& v) const { return v.triangulation().vertex_is_main(*v); }
    /// returns whether the facet iterator is referring to a tile triangulation where the facet is main.
    bool is_main(const Facet_iterator&  f) const { return f.triangulation().facet_is_main(*f); }
    /// returns whether the cell iterator is referring to a tile triangulation where the cell is main.
    bool is_main(const Cell_iterator&   c) const { return c.triangulation().cell_is_main(*c); }

    /// returns whether the vertex is the infinite vertex.
    bool is_infinite(const Vertex_iterator& v) const { return v.triangulation().vertex_is_infinite(*v); }
    /// returns whether the facet is incident to the infinite vertex.
    bool is_infinite(const Facet_iterator&  f) const { return f.triangulation().facet_is_infinite(*f); }
    /// returns whether the cell is incident to the infinite vertex.
    bool is_infinite(const Cell_iterator&   c) const { return c.triangulation().cell_is_infinite(*c); }
    /// @}

    /// \name Tile identifiers from iterators
    /// @{
    /// returns the tile id of the vertex.
    Tile_index id(const Vertex_iterator&v) const { return v.triangulation().vertex_id(*v); }
    /// returns the tile id of the facet.
    Tile_index id(const Facet_iterator& f) const { return f.triangulation().facet_id(*f); }
    /// returns the tile id of the cell.
    Tile_index id(const Cell_iterator&  c) const { return c.triangulation().cell_id(*c); }

    /// returns the tile id of the tile referred by the iterator.
    Tile_index tile_id(const Vertex_iterator& v) const { return v.id(); }
    /// returns the tile id of the tile referred by the iterator.
    Tile_index tile_id(const Facet_iterator&  f) const { return f.id(); }
    /// returns the tile id of the tile referred by the iterator.
    Tile_index tile_id(const Cell_iterator&   c) const { return c.id(); }
    /// @}

    /// \name Iterator relocation
    /// @{
    /// returns an alternative iterator that represents the same simplex,
    /// but that lives in the tile corresponding to the tile index passed.
    /// If the simplex is not presented that tile, the corresponding end iterator is returned

    /// returns a vertex iterator equivalent to `v` in the tile with index `id`. They represent the same vertex of the global triangulation.
    Vertex_iterator relocate(const Vertex_iterator& v, Tile_index id) const
    {
        CGAL_assertion(is_valid(v));
        if (id == tile_id(v)) return v; // v is already in tile id
        Tile_const_iterator tile = tiles.find(id);
        if (tile == tiles.end()) return vertices_end();
        const Tile_triangulation& tri = tile->second;
        Tile_vertex_index vertex = tri.relocate_vertex(v.triangulation(), *v);
        if (vertex==tri.vertices_end()) return vertices_end();
        return Vertex_iterator(&tiles, tile, vertex);
    }

    /// returns a facet iterator equivalent to `f` in the tile with index `id`. They represent the same facet of the global triangulation.
    Facet_iterator relocate(const Facet_iterator& f, Tile_index id) const
    {
        CGAL_assertion(is_valid(f));
        if (id == tile_id(f)) return f; // f is already in tile id
        Tile_const_iterator tile = tiles.find(id);
        if (tile == tiles.end()) return facets_end();
        const Tile_triangulation& tri = tile->second;
        Tile_facet_index facet = tri.relocate_facet(f.triangulation(), *f);
        if (facet==tri.facets_end()) return facets_end();
        return Facet_iterator(&tiles, tile, facet);
    }

    /// returns a cell iterator equivalent to `c` in the tile with index `id`. They represent the same cell of the global triangulation.
    Cell_iterator relocate(const Cell_iterator& c, Tile_index id) const
    {
        CGAL_assertion(is_valid(c));
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
    /// \todo loaded is not defined
    /// MB: I agree that this doc needs to define "loaded". But my guess is that we should lift this precondition and enable provide a dummy Vertex_iterator for the infinite vertex that does not need any loaded tile (eg with tile=tiles.end())
    inline Vertex_iterator infinite_vertex() const
    {
        CGAL_assertion(!tiles.empty());
        Tile_const_iterator tile = tiles.cbegin();
        const Tile_triangulation& tri = tile->second;
        return Vertex_iterator(&tiles, tile, tri.infinite_vertex());
    }

    /// returns the ith vertex of cell `c`.
    /// Indexing by `i` is consistent over all representatives of cell `c`, as its main representative is looked up first.
    Vertex_iterator vertex (const Cell_iterator& c, const int i) const
    {
        CGAL_assertion(is_valid(c));
        return local_vertex(main(c), i); // i is defined wrt the main representative
    }

    /// returns the point embedding of the vertex.
    /// This can be done locally without considering the main tile of the vertex, as point coordinates
    /// are replicated in all tiles.
    Point_const_reference point(Vertex_iterator v) const
    {
        CGAL_assertion(is_valid(v));
        return v.triangulation().triangulation_point(*v);
    }

    /// returns the mirror facet. This operation is performed locally: the resulting facet belongs
    /// to the same tile as the input facet.
    /// Precondition: the facet `f` is valid (ie. at least one vertex is local among the covertex and the mirror vertex and the facet points)
    Facet_iterator mirror_facet(const Facet_iterator& f) const
    {
        CGAL_assertion(is_valid(f));
        return Facet_iterator(&tiles, f.tile(), f.triangulation().mirror_facet(*f));
    }

    /// returns the mirror index of facet `f`, such that `neighbor(cell(mirror_facet(f)), mirror_index)==cell(f)`
    /// The index of covertex of the main version of the mirror facet of f is considered, as indices may not be consistent across
    /// the representatives of mirror facet in other tiles.
    /// \todo covertex is not defined and/or I don't know if it is a standard terminology MB: I thought it was. That's the only vertex of the cell of the facet that is not incident to the facet.
    inline int mirror_index(const Facet_iterator& f) const
    {
        CGAL_assertion(is_valid(f));
        return index_of_covertex(mirror_facet(f));
    }

    /// returns the cell that is incident to the input facet `f` and that joins the covertex with the vertices of `f`
    /// The operation is local iff the local cell of f is not foreign.
    /// \todo full is not defined MB: ok. "full cell" -> "cell"
    Cell_iterator cell(const Facet_iterator& f) const
    {
        CGAL_assertion(is_valid(f));
        const Tile_triangulation& tri = f.triangulation();
        Tile_cell_index c = tri.cell_of_facet(*f);
        if(tri.cell_is_foreign(c)) return local_cell(main(f)); // any non foreign representative could do
        return Cell_iterator(&tiles, f.tile(), c);
    }

    /// returns one of the cells that is incident `v` in the triangulation of the same tile as `v`.
    Cell_iterator cell(const Vertex_iterator& v) const
    {
        const Tile_triangulation& triangulation = v.triangulation();
        Tile_vertex_index tv = *v;
        Tile_cell_index tc = triangulation.cell_of_vertex(tv);
        if(!triangulation.cell_is_foreign(tc))
            return Cell_iterator(&tiles, v.tile(), tc);

        std::vector<Tile_cell_index> cells;
        triangulation.incident_cells(tv, std::back_inserter(cells));
        for(Tile_cell_index c: cells)
            if(!triangulation.cell_is_foreign(c))
                return Cell_iterator(&tiles, v.tile(), c);
        CGAL_assertion(false); // all incident cells are foreign, v should have been simplified !
        return cells_end();
    }

    /// returns whether `v` is incident to `c`. The operation is local in the tile of `c` and looks for a vertex in this tile
    /// if there is a vertex with the same coordinates as `v`
    /// \todo will it return false if v and c not in the same tile triangulation? MB: no if another representant exists there. (updated doc)
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
    /// returns the covertex of a facet f, which is the only vertex of the facet's cell that is not incident to this facet.
    /// The operation is local iff the local cell of f is not foreign
    Vertex_iterator covertex(const Facet_iterator& f) const
    {
        CGAL_assertion(is_valid(f));
        const Tile_triangulation& tri = f.triangulation();
        Tile_cell_index c = tri.cell_of_facet(*f);
        if(tri.cell_is_foreign(c)) return local_covertex(main(f)); // any non foreign representative could do
        return Vertex_iterator(&tiles, f.tile(), tri.covertex(*f));
    }

    /// returns the index of the covertex of a facet f
    /// The operation is local iff the local cell of f is main
    inline int index_of_covertex(const Facet_iterator& f) const
    {
        CGAL_assertion(is_valid(f));
        const Tile_triangulation& tri = f.triangulation();
        if (tri.cell_is_main(tri.cell_of_facet(*f)))
            return local_index_of_covertex(f);
        return local_index_of_covertex(relocate(f, id(cell(f))));
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
        CGAL_assertion(is_valid(c));
        return local_facet(main(c),i); // i is defined wrt the main representative
    }

    /// returns the neighboring cell, opposite to the ith vertex.
    /// The operation may require to change tile twice : once if the given cell c is not main, and once if the mirror_facet of facet(main(c),i) is foreign.
    inline Cell_iterator neighbor(const Cell_iterator& c, int i) const
    {
        CGAL_assertion(is_valid(c));
        return cell(mirror_facet(facet(c, i)));
    }

    /// returns the mirror index of a cell, such that neighbor(neighbor(cell,i), mirror_index(cell,i))==cell .
    /// The operation is local if may require to change tile twice : once if the given cell c is not main, and once if the cell of mirror_facet of facet(main(c),i) is not main.
    inline int mirror_index(const Cell_iterator& c, int i) const
    {
        CGAL_assertion(is_valid(c));
        return mirror_index(facet(c,i));
    }
    /// @}

    /// \name Iterator Local operations
    /// @{
    /// \cgalAdvancedBegin
    /// The following functions prefixed with `local_` are useful for advanced usages when the function calls can be done locally on the tile triangulation, without changing tile.
    /// This is more efficient as the operation is garanteed to be local.
    /// Functions that have an input or output vertex index yield different results if performed locally,
    /// as the vertex ordering in cells is not garanteed to be consistent across tiles. In some cases however, like
    /// iterating over the vertices of a cell, a globally consistent indexing of the cell vertices may not be required
    /// and the local indexing of the local functions may be used instead for better performance.
    /// \cgalAdvancedEnd

    /// returns the ith vertex of the cell `c` in its local tile.
    /// Advanced use: Access is local, thus more more effective, but the vertex index i corresponds
    /// to the index in the local tile representative of cell `c`, which may not be consistent with
    /// the vertex ordering in its main representative.
    /// \todo Replace Advance use: with advanced macro + preconditions? MB:ok
    Vertex_iterator local_vertex (const Cell_iterator& c, const int i) const
    {
        CGAL_assertion(is_valid(c));
        return Vertex_iterator(&tiles, c.tile(), c.triangulation().vertex(*c, i));
    }

    /// returns the index of the covertex of `f` in its local cell
    /// Advanced use: Access is local, thus more more effective, but the returned index relates to
    /// the local cell incident to `f`. The vertex ordering of the local cell may not correspond to the one of
    /// its main representative.
    /// Precondition: the local cell of `f` is not foreign.
    inline int local_index_of_covertex(const Facet_iterator& f) const
    {
        CGAL_assertion(is_valid(f));
        return f.triangulation().index_of_covertex(*f);
    }

    /// constructs a facet locally given a cell and a local index `i`
    /// Advanced use: Access is local, thus more more effective, but the returned facet is defined using the local index `i`,
    /// which may indexing of the main representative of the cell `c`.
    Facet_iterator local_facet(const Cell_iterator& c, int i) const
    {
        CGAL_assertion(is_valid(c));
        return Facet_iterator(&tiles, c.tile(), c.triangulation().facet(*c, i));
    }

    /// returns the index of the mirror vertex of `f` locally
    /// Advanced use: Access is local, thus more more effective, but the returned index relates to the local indexing of the local cell of the mirror of `f`
    /// Precondition : the local cell of the mirror of f should not be foreign
    inline int local_mirror_index(const Facet_iterator& f) const
    {
        CGAL_assertion(is_valid(f));
        const Tile_triangulation& tri= f.triangulation();
        Tile_cell_index c = tri.cell_of_facet(*f);
        CGAL_assertion(!tri.cell_is_foreign(c));
        return tri.mirror_index(c,tri.index_of_covertex(*f));
    }

    /// returns the cell that is adjacent to the input facet `f` and that joins the covertex with the vertices of `f`
    /// Advanced use: Access is local, thus more more effective, but assumes that the local cell of `f` is not foreign.
    Cell_iterator local_cell(const Facet_iterator& f) const
    {
        CGAL_assertion(is_valid(f));
        const Tile_triangulation& tri= f.triangulation();
        Tile_cell_index c = tri.cell_of_facet(*f);
        CGAL_assertion(!tri.cell_is_foreign(c));
        return Cell_iterator(&tiles, f.tile(), c);
    }

    /// @returns the covertex of the input facet f
    /// Advanced use: Access is local, thus more more effective, but assumes that the local cell of `f` is not foreign.
    Vertex_iterator local_covertex(const Facet_iterator& f) const
    {
        CGAL_assertion(is_valid(f));
        const Tile_triangulation& tri = f.triangulation();
        Tile_cell_index c = tri.cell_of_facet(*f);
        CGAL_assertion(!tri.cell_is_foreign(c));
        return Vertex_iterator(&tiles, f.tile(), tri.covertex(*f));
    }
    /// @}


    /// \ingroup PkgDDTInsert
    /// triangulates the points of a distributed point set using the tile partition of the distributed point set, so that each of its tile triangulations contain a local view of the overall
    /// triangulation of all inserted points.
    /// The scheduler provides the distribution environment (single thread, multithread, MPI...)
    /// @returns the number of newly inserted vertices
    template<typename PointSet1, typename IndexMap1, typename Serializer1, typename Scheduler>
    std::size_t insert(CGAL::Distributed_point_set<PointSet1, IndexMap1, Serializer1>& points, Scheduler& sch)
    {
        std::size_t n0 = number_of_finite_vertices();
        CGAL_DDT_TRACE1(sch, "DDT", "insert", 0, "B", in, n0);

        typedef CGAL::Distributed_point_set<PointSet1, IndexMap1, Serializer1>  Distributed_point_set1;
        typedef typename Distributed_point_set1::Point     Point;
        typedef typename CGAL::DDT::Kernel_traits<Point>::Point_set_with_id<Tile_index> PointSet2;
        typedef CGAL::DDT::Internal_property_map<PointSet2> IndexMap2;
        IndexMap2 indices2;
        std::multimap<Tile_index, CGAL::DDT::Tile_point_set<PointSet2, IndexMap2>> points2;

        CGAL_DDT_TRACE1(sch, "DDT", "splay_stars", 0, "B", in, CGAL::DDT::to_summary(points.begin(), points.end()));
        CGAL::DDT::impl::splay_stars(tiles, tile_indices, points.tiles, points2, indices2, sch, root, maximal_dimension());
        CGAL_DDT_TRACE1(sch, "DDT", "splay_stars", 0, "E", out, CGAL::DDT::to_summary(points.begin(), points.end()));

        finalize(sch);
        std::size_t n1 = number_of_finite_vertices();
        std::size_t n = n1 - n0;
        CGAL_DDT_TRACE2(sch, "DDT", "insert", 0, "E", out, n1, inserted, n);

        return n;
    }


    /// \ingroup PkgDDTInsert
    /// inserts `point` in the tile given with index `id`, in the Delaunay triangulation stored in the tile container.
    /// The scheduler provides the distribution environment (single thread, multithread, MPI...)
    /// \todo Only Delaunay? MB: yes in the first release ;). For the following release, we may try to support other insertion order independent triangulations.
    /// @returns v a descriptor to the inserted vertex and a bool
    template<typename Point_const_reference, typename Tile_index, typename Scheduler>
    std::pair<Vertex_iterator, bool> insert(Point_const_reference point, Tile_index id, Scheduler& sch)
    {
        CGAL_DDT_TRACE0(sch, "DDT", "insert1", 0, "B");
        auto emplaced = tiles.emplace(std::piecewise_construct,
                std::forward_as_tuple(id),
                std::forward_as_tuple(id, maximal_dimension_, tile_indices));
        // insert the point in its local tile
        Tile_iterator tile = emplaced.first;
        Tile_triangulation& tri  = tile->second;

        std::pair<Tile_vertex_index, bool> p = tri.insert(point, id);
        Tile_vertex_index v = p.first;
        Vertex_iterator res(&tiles, tile, v);
        if (!p.second) {
            CGAL_DDT_TRACE1(sch, "DDT", "insert1", 0, "E", new, 0);
            return std::make_pair(res, false);
        }

        typedef typename CGAL::DDT::Kernel_traits<Point>::Point_set_with_id<Tile_index> PointSet;
        typedef CGAL::DDT::Internal_property_map<PointSet> IndexMap;
        typedef CGAL::Distributed_point_set<PointSet, IndexMap>  Distributed_point_set;
        Distributed_point_set points;

        if (emplaced.second) { // tile did not exist
            // send (id, point) to the root tile
            points.insert(point, id, root);

        } else {
            // get its neighbors
            std::vector<Tile_vertex_index> adj;
            tri.adjacent_vertices(v, std::inserter(adj, adj.begin()));

            // get the unique tile indices of the finite foreign neighbors
            std::set<Tile_index> indices;
            for (auto w : adj) {
                if(tri.vertex_is_infinite(w)) continue;
                Tile_index idw = tri.vertex_id(w);
                if(idw != id) indices.insert(idw);
            }

            // send (id, point) to each such neighboring tile
            for (auto idw : indices)
                points.insert(point, id, idw);
        }

        insert(points, sch);
        CGAL_DDT_TRACE1(sch, "DDT", "insert1", 0, "E", new, 1);
        return std::make_pair(res, true);
    }

    /// finalizes the distributed triangulation by updating its number of simplex statistics.
    template <typename Scheduler>
    void finalize(Scheduler& sch)
    {
        CGAL_DDT_TRACE0(sch, "DDT", "finalize", 0, "B");
        Statistics stats;
        Tile_index& root = this->root;
        statistics_ = sch.ranges_reduce(tiles, [&root](Tile_const_iterator first, Tile_const_iterator last) {
            first->second.finalize();
            return (first->first == root) ? Statistics{} : first->second.statistics();
        }, stats, std::plus<>());
        CGAL_DDT_TRACE0(sch, "DDT", "finalize", 0, "E");
    }

    /// writes the distributed triangulation using the provided Writer and Scheduler
    template <typename Writer, typename Scheduler>
    bool write(const Writer& writer, Scheduler& sch) const
    {
        CGAL_DDT_TRACE0(sch, "DDT", "write", 0, "B");
        bool ok =
            writer.write_begin(*this, sch.thread_index()) &&
            sch.ranges_reduce(tiles, [&writer](Tile_const_iterator first, Tile_const_iterator last) {
                CGAL_assertion(std::distance(first, last) == 1);
                return writer.write(first->second);
            }, true, std::logical_and<>()) &&
            writer.write_end(*this, sch.thread_index());
        CGAL_DDT_TRACE0(sch, "DDT", "write", 0, "E");
        return ok;
    }

    /// reads the distributed triangulation using the provided Reader and Scheduler
    /// \todo : what happens to the serializer ? MB: good point. I think that the current implementation will overwrite the data provided by the serializer with data provided by the reader
    /// first in memory and then on disk for tiles that have been swaped out of memory
        template <typename Reader, typename Scheduler>
        bool read(const Reader& reader, Scheduler& sch)
    {
        clear();
        CGAL_DDT_TRACE0(sch, "DDT", "read", 0, "B");
        bool ok =
            reader.read_begin(*this, sch.thread_index()) &&
            sch.ranges_reduce(tiles, [&reader](Tile_iterator first, Tile_iterator last) {
                CGAL_assertion(std::distance(first, last) == 1);
                return reader.read(first->second);
            }, true, std::logical_and<>()) &&
            reader.read_end(*this, sch.thread_index());
        CGAL_DDT_TRACE0(sch, "DDT", "read", 0, "E");
        return ok;
    }

    /// combined read and write. This is equivalent to calling `read(reader,sch)` followed by `write(writer,sch)`,
    /// but is more optimal as it is implemented in a single pass over the tiles.
    template <typename Reader, typename Writer, typename Scheduler>
    bool read_write(const Reader& reader, const Writer& writer, Scheduler& sch)
    {
        clear();
        CGAL_DDT_TRACE0(sch, "DDT", "read_write", 0, "B");
        bool ok =
            reader.read_begin(*this, sch.thread_index()) &&
            writer.write_begin(*this, sch.thread_index()) &&
            sch.ranges_reduce(tiles, [&reader,&writer](Tile_iterator first, Tile_iterator last) {
                CGAL_assertion(std::distance(first, last) == 1);
                return reader.read(first->second) && writer.write(first->second);
            }, true, std::logical_and<>()) &&
            reader.read_end(*this, sch.thread_index()) &&
            writer.write_end(*this, sch.thread_index());
        CGAL_DDT_TRACE0(sch, "DDT", "read_write", 0, "E");
        return ok;
    }

    /// initiliazes in `this` the distributed triangulation resulting from repartitioning the input distributed triangulation `that`
    template <typename DistributedTriangulation, typename Partitioner, typename Scheduler>
    void partition(const Partitioner& part, const DistributedTriangulation& that, Scheduler& sch) {
        CGAL_DDT_TRACE0(sch, "DDT", "partition", 0, "B");
        typedef typename Tile_triangulation::Vertex_index Vertex_index;
        typedef typename CGAL::DDT::Kernel_traits<Point>::Point_set_with_id<Tile_index> PointSet;
        typedef std::multimap<Tile_index, PointSet> PointSetContainer;
        PointSetContainer point_sets;
        sch.template ranges_transform<typename PointSetContainer::value_type>(that.tiles,
            [&part](auto first, auto last, auto out) {
                for(auto it = first; it != last; ++it) {
                    auto& tri = it->second;
                    std::map<Tile_index, std::set<Vertex_index>> vertex_set_map;
                    for(Vertex_index v = tri.vertices_begin(); v != tri.vertices_end(); ++v)
                    {
                        if (tri.vertex_is_infinite(v)) continue;
                        Tile_index key = part(tri.triangulation_point(v));
                        std::set<Vertex_index>& vertex_set = vertex_set_map[key];
                        vertex_set.insert(v);
                        tri.adjacent_vertices(v, std::inserter(vertex_set, vertex_set.end()));
                    }
                    for(const auto& p : vertex_set_map)
                    {
                        PointSet points;
                        Tile_index key = p.first;
                        const std::set<Vertex_index>& vertex_set = p.second;
                        for(const auto& v : vertex_set)
                        {
                            if (tri.vertex_is_infinite(v)) continue;
                            Point_const_reference p = tri.triangulation_point(v);
                            points.emplace_back(part(p), p);
                        }
                        *out++ = {key, points};
                    }
                }
                return out;
            }, std::inserter(point_sets, point_sets.begin())
        );
        maximal_dimension_ = that.maximal_dimension();
        Statistics stats;
        tiles.clear();
        statistics_ = sch.template ranges_transform_reduce<typename AssociativeContainer::value_type>(point_sets,
            [&](auto first, auto last, auto out) {
                Tile_index key = first->first;
                Tile_triangulation tri(key, maximal_dimension_, tile_indices);
                Vertex_index hint;
                // simplification is not needed (local and adjacent to local points only are received)
                // simplification would be incorrect, as foreign vertices may come first and be simplified before their local neighbor is inserted
                // should we spatial sort ?
                for(auto it = first; it != last; ++it)
                    for(const auto& p : it->second)
                        hint = tri.insert(p.second, p.first, hint).first;
                Statistics stats = tri.statistics();
                *out++ = { key, std::move(tri) };
                return std::make_pair(stats, out);
            }, stats, std::plus<>(), std::inserter(tiles, tiles.begin())
        ).first;
        CGAL_DDT_TRACE0(sch, "DDT", "partition", 0, "E");
    }

    /// clears the triangulation
    /// \todo: what is the semantics of clearing when a serializer is present ? should we delete the files ? MB: I am not sure...
    void clear() {
        statistics_ = {};
        statistics_.valid = false;
        tiles.clear();
    }

    const Statistics& statistics() const { return statistics_; }
    Statistics& statistics() { return statistics_; }

    Container tiles;
    TileIndexProperty tile_indices;

private:
    Statistics statistics_;
    int maximal_dimension_;
    Tile_index root = {};
};

}

#endif // CGAL_DISTRIBUTED_TRIANGULATION_H
