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

#ifndef CGAL_DISTRIBUTED_DELAUNAY_TRIANGULATION_H
#define CGAL_DISTRIBUTED_DELAUNAY_TRIANGULATION_H

#include <CGAL/DDT/iterator/Vertex_const_iterator.h>
#include <CGAL/DDT/iterator/Facet_const_iterator.h>
#include <CGAL/DDT/iterator/Cell_const_iterator.h>

namespace CGAL {

/// \ingroup PkgDDTRef
/// \tparam TileContainer is a container that abstracts the storage of the triangulation tiles.
/// The Distributed_Delaunay_triangulation class wraps a TileContainer to expose a triangulation interface.
template<typename TileContainer>
class Distributed_Delaunay_triangulation
{
private:
    typedef typename TileContainer::Traits              Traits;
    typedef typename TileContainer::Tile_const_iterator Tile_const_iterator;
    typedef typename Traits::Vertex_const_iterator      Tile_vertex_const_iterator;
    typedef typename Traits::Cell_const_iterator        Tile_cell_const_iterator;
    typedef typename Traits::Facet_const_iterator       Tile_facet_const_iterator;

public:
/// \name Types
/// @{

    typedef TileContainer                            Tile_container;
    typedef typename Traits::Point                   Point;
    typedef typename Traits::Id                      Id;

    typedef CGAL::DDT::Vertex_const_iterator<TileContainer> Vertex_const_iterator;
    typedef CGAL::DDT::Facet_const_iterator <TileContainer> Facet_const_iterator;
    typedef CGAL::DDT::Cell_const_iterator  <TileContainer> Cell_const_iterator;
/// @}

    /// contructor
    Distributed_Delaunay_triangulation(TileContainer& tc) : tiles(tc) {}
    /// the dimension of the triangulation
    inline int maximal_dimension() const { return tiles.maximal_dimension(); }
    /// The number of finite cells in the triangulation, including cells incident to the vertex at infinity.
    inline size_t number_of_finite_cells   () const { return tiles.number_of_finite_cells();    }
    /// The number of finite vertices in the triangulation, including the vertex at infinity.
    inline size_t number_of_finite_vertices() const { return tiles.number_of_finite_vertices(); }
    /// The number of facets in the triangulation, including facets incident to the vertex at infinity.
    inline size_t number_of_finite_facets  () const { return tiles.number_of_finite_facets();   }
    /// The number of finite cells in the triangulation.
    inline size_t number_of_cells   () const { return tiles.number_of_cells();    }
    /// The number of finite vertices in the triangulation.
    inline size_t number_of_vertices() const { return tiles.number_of_vertices(); }
    /// The number of finite facets in the triangulation.
    inline size_t number_of_facets  () const { return tiles.number_of_facets();   }



    /// \name Iterators
    /// @{

    /// @returns A const iterator at the start of the range of finite vertices.
    Vertex_const_iterator vertices_begin() const { return Vertex_const_iterator(&tiles, tiles.cbegin()); }
    /// @returns A const iterator past the end of the range of finite vertices.
    Vertex_const_iterator vertices_end  () const { return Vertex_const_iterator(&tiles, tiles.cend()); }

    /// @returns A const iterator at the start of the range of finite cells.
    Cell_const_iterator cells_begin() const { return Cell_const_iterator(&tiles, tiles.cbegin()); }
    /// @returns A const iterator past the end of the range of finite cells.
    Cell_const_iterator cells_end  () const { return Cell_const_iterator(&tiles, tiles.cend()); }

    /// @returns A const iterator at the start of the range of finite facets.
    Facet_const_iterator facets_begin() const { return Facet_const_iterator(&tiles, tiles.cbegin()); }
    /// @returns A const iterator past the end of the range of finite facets.
    Facet_const_iterator facets_end  () const { return Facet_const_iterator(&tiles, tiles.cend()); }

    /// @}

    /// \name Global Identifiers
    /// @{

    /// Get a global id of the vertex iterator using its distance to vertices_begin. (implementation is linear in the returned id)
    int vertex_id(Vertex_const_iterator v) const
    {
        if (is_infinite(v)) return -1;
        return std::distance(vertices_begin(), main(v));
    }

    /// Get a global id of the cell iterator using its distance to cells_begin. (implementation is linear in the returned id)
    int cell_id(Cell_const_iterator c) const
    {
        return std::distance(cells_begin(), main(c));
    }
    /// @}

    /// Get in the cell set cset, the cells that are at deg hops from the input cell c.
    void get_ring(Cell_const_iterator c, int deg, std::set<Cell_const_iterator>& cset) const
    {
        std::set<Cell_const_iterator> seeds;
        c = main(c);
        cset.insert(c);
        seeds.insert(c);
        for(int i=0; i<deg; ++i)
        {
            std::set<Cell_const_iterator> next;
            next_ring(seeds, next);
            cset.insert(next.begin(), next.end());
            seeds.swap(next);
        }
    }

    /// Get in the cell set next, the cells that are adjacent to the cells in the input set seeds.
    void next_ring(const std::set<Cell_const_iterator>& seeds, std::set<Cell_const_iterator>& next) const
    {
        for(auto seed : seeds)
        {
            for(int d = 0; d <= maximal_dimension(); d++)
            {
                auto c = main(neighbor(seed, d));
                if(seeds.find(c) == seeds.end())
                    next.insert(c);
            }
        }
    }


    /// checks the validity of the Distributed_Delaunay_triangulation
    bool is_valid() const
    {
        if (!tiles.is_valid()) return false;
        for(auto tile = tiles.begin(); tile != tiles.end(); ++tile)
        {
            for(auto v = tile->vertices_begin(); v != tile->vertices_end(); ++v)
            {
                assert(tile->vertex_is_infinite(v) || (tile->vertex_is_local(v) + tile->vertex_is_foreign(v) == 1));
                if(tile->vertex_is_infinite(v)) continue;
                Id tid = tile->id(v);
                if(tid == tile->id()) continue;
                auto t = tiles.find(tid);
                if(t->relocate_vertex(*tile, v) == t->vertices_end())
                {
                    assert(! "relocate_vertex failed" );
                    return false;
                }
            }
            for(auto f = tile->facets_begin(); f != tile->facets_end(); ++f)
            {
                assert(tile->facet_is_local(f) + tile->facet_is_mixed(f) + tile->facet_is_foreign(f) == 1);
                if(!tile->facet_is_mixed(f)) continue;
                std::set<Id> tids;
                for(int d = 0; d <= tile->current_dimension(); ++d)
                {
                    if(d==tile->index_of_covertex(f)) continue;
                    auto c = tile->cell(f);
                    auto v = tile->vertex(c, d);
                    if(tile->vertex_is_infinite(v)) continue;
                    Id tid = tile->id(v);
                    if(tid == tile->id()) continue;
                    tids.insert(tid);
                }
                for(auto tid : tids)
                {
                    auto t = tiles.find(tid);
                    if(t->relocate_facet(*tile, f) == t->facets_end())
                    {
                      assert(! "relocate_facet failed" );
                      return false;
                    }
                }

            }
            for(auto c = tile->cells_begin(); c != tile->cells_end(); ++c)
            {
                assert(tile->cell_is_local(c) + tile->cell_is_mixed(c) + tile->cell_is_foreign(c) == 1);
                if(!tile->cell_is_mixed(c)) continue;
                std::set<Id> tids;
                for(int d = 0; d <= tile->current_dimension(); ++d)
                {
                    auto v = tile->vertex(c, d);
                    if(tile->vertex_is_infinite(v)) continue;
                    Id tid = tile->id(v);
                    if(tid == tile->id()) continue;
                    tids.insert(tid);
                }
                for(auto tid : tids)
                {
                    auto t = tiles.find(tid);
                    if(t->relocate_cell(*tile, c) == t->cells_end())
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

    bool is_local(const Vertex_const_iterator& v) const { return v.tile()->vertex_is_local(*v); }
    bool is_local(const Facet_const_iterator&  f) const { return f.tile()->facet_is_local(*f); }
    bool is_local(const Cell_const_iterator&   c) const { return c.tile()->cell_is_local(*c); }

    bool is_valid(const Vertex_const_iterator& v) const { return v.tile()->vertex_is_valid(*v); } // + tile toujours chargée ?
    bool is_valid(const Facet_const_iterator&  f) const { return f.tile()->facet_is_valid(*f); } // + tile toujours chargée ?
    bool is_valid(const Cell_const_iterator&   c) const { return c.tile()->cell_is_valid(*c); } // + tile toujours chargée ?

    // vertices are never mixed
    bool is_mixed(const Facet_const_iterator& f) const { return f.tile()->facet_is_mixed(*f); }
    bool is_mixed(const Cell_const_iterator&  c) const { return c.tile()->cell_is_mixed(*c); }

    bool is_foreign(const Vertex_const_iterator& v) const { return v.tile()->vertex_is_foreign(*v); }
    bool is_foreign(const Facet_const_iterator&  f) const { return f.tile()->facet_is_foreign(*f); }
    bool is_foreign(const Cell_const_iterator&   c) const { return c.tile()->cell_is_foreign(*c); }

    bool is_main(const Vertex_const_iterator& v) const { return v.tile()->vertex_is_main(*v); }
    bool is_main(const Facet_const_iterator&  f) const { return f.tile()->facet_is_main(*f); }
    bool is_main(const Cell_const_iterator&   c) const { return c.tile()->cell_is_main(*c); }

    bool is_infinite(const Vertex_const_iterator& v) const { return v.tile()->vertex_is_infinite(*v); }
    bool is_infinite(const Facet_const_iterator&  f) const { return f.tile()->facet_is_infinite(*f); }
    bool is_infinite(const Cell_const_iterator&   c) const { return c.tile()->cell_is_infinite(*c); }
    /// @}

    /// \name Tile identifiers from iterators
    /// @{
    Id main_id(const Vertex_const_iterator&v) const { return v.tile()->id(*v); }
    Id main_id(const Facet_const_iterator& f) const { return f.tile()->minimum_id(*f); }
    Id main_id(const Cell_const_iterator&  c) const { return c.tile()->minimum_id(*c); }

    Id tile_id(const Vertex_const_iterator& v) const { return v.tile()->id(); }
    Id tile_id(const Facet_const_iterator&  f) const { return f.tile()->id(); }
    Id tile_id(const Cell_const_iterator&   c) const { return c.tile()->id(); }
    /// @}

    /// \name Iterator relocation
    /// @{
    /// `change_tile` functions return an alternative iterator that represents the same simplex,
    /// but that lives in the tile with the provided Id.
    /// If the simplex is not represented there, the end iterator is returned

    /// Get a vertex iterator equivalent to v in tile id. They represent the same vertex of the global triangulation.
    Vertex_const_iterator relocate(const Vertex_const_iterator& v, Id id) const
    {
        assert(is_valid(v));
        if (id == tile_id(v)) return v; // v is already in tile id
        Tile_const_iterator tile = tiles.load(id);
        Tile_vertex_const_iterator vertex = tile->relocate_vertex(*(v.tile()), *v);
        if (vertex==tile->vertices_end()) return vertices_end();
        return Vertex_const_iterator(&tiles, tile, vertex);
    }

    /// Get a facet iterator equivalent to f in tile id. They represent the same facet of the global triangulation.
    Facet_const_iterator relocate(const Facet_const_iterator& f, Id id) const
    {
        assert(is_valid(f));
        if (id == tile_id(f)) return f; // f is already in tile id
        Tile_const_iterator tile = tiles.load(id);
        Tile_facet_const_iterator facet = tile->relocate_facet(*(f.tile()), *f);
        if (facet==tile->facets_end()) return facets_end();
        return Facet_const_iterator(&tiles, tile, facet);
    }

    /// get a cell iterator equivalent to c in tile id. They represent the same cell of the global triangulation.
    Cell_const_iterator relocate(const Cell_const_iterator& c, Id id) const
    {
        assert(is_valid(c));
        if (id == tile_id(c)) return c; // c is already in tile id
        Tile_const_iterator tile = tiles.load(id);
        Tile_cell_const_iterator cell = tile->relocate_cell(*(c.tile()), *c);
        if (cell==tile->cells_end()) return cells_end();
        return Cell_const_iterator(&tiles, tile, cell);
    }

    /// get the main representative of a vertex iterator
    inline Vertex_const_iterator main(const Vertex_const_iterator& v) const { return relocate(v, main_id(v)); }
    /// get the main representative of a facet iterator
    inline Facet_const_iterator main(const Facet_const_iterator& f) const { return relocate(f, main_id(f)); }
    /// get the main representative of a cell iterator
    inline Cell_const_iterator main(const Cell_const_iterator& c) const { return relocate(c, main_id(c)); }

    /// @}


    /// \name Iterator operations
    /// @{

    /// get a representative iterator for the infinite vertex
    /// precondition : at least one tile is loaded.
    inline Vertex_const_iterator infinite_vertex() const
    {
        assert(!tiles.empty());
        Tile_const_iterator tile = tiles.cbegin();
        return Vertex_const_iterator(&tiles, tile, tile->infinite_vertex());
    }

    /// Access the ith vertex of cell c.
    /// Indexing by i is consistent over all representatives of cell c, as its main representative is looked up.
    Vertex_const_iterator vertex (const Cell_const_iterator& c, const int i) const
    {
        assert(is_valid(c));
        return local_vertex(main(c), i); // i is defined wrt the main representative
    }

    /// Retrieve the point embedding of the vertex.
    /// This can be done locally without considering the main tile of the vertex, as point coordinates
    /// are replicated in all tiles.
    const Point& point(Vertex_const_iterator v) const
    {
        assert(is_valid(v));
        return v.tile()->point(*v);
    }

    /// @returns the mirror facet. This operation is performed locally: the resulting facet belongs
    /// to the same tile as the input facet.
    /// Precondition: the facet f is valid (ie: at least one of the facet points, the covertex and the mirror vertex is local)
    Facet_const_iterator mirror_facet(const Facet_const_iterator& f) const
    {
        assert(is_valid(f));
        Tile_const_iterator tile = f.tile();
        assert(tile->facet_is_valid(*f));
        return Facet_const_iterator(&tiles, tile, tile->mirror_facet(*f));
    }

    /// Access the mirror index of facet f, such that neighbor(cell(mirror_facet(f)), mirror_index)==cell(f)
    /// The the index of covertex of the main version of the mirror facet of f is considered, as indices may not be consistent across
    /// the representatives of mirror facet in other tiles.
    inline int mirror_index(const Facet_const_iterator& f) const
    {
        assert(is_valid(f));
        return index_of_covertex(mirror_facet(f));
    }

    /// @returns the full cell that is incident to the input facet f and that joins the covertex with the vertices of f
    /// The operation is local iff the local cell of f is not foreign.
    Cell_const_iterator cell(const Facet_const_iterator& f) const
    {
        assert(is_valid(f));
        Tile_const_iterator tile = f.tile();
        Tile_cell_const_iterator c = tile->cell(*f);
        if(tile->cell_is_foreign(c)) return local_cell(main(f)); // any non foreign representative could do
        return Cell_const_iterator(&tiles, tile, c);
    }

    /// @returns one of the full cells that is incident to the input vertex v. The operation is local
    Cell_const_iterator cell(const Vertex_const_iterator& v) const
    {
        Tile_const_iterator tile = v.tile();
        Tile_vertex_const_iterator tv = *v;
        Tile_cell_const_iterator tc = tile->cell(tv);
        if(!tile->cell_is_foreign(tc))
            return Cell_const_iterator(&tiles, tile, tc);

        std::vector<Tile_cell_const_iterator> cells;
        tile->incident_cells(tv, std::back_inserter(cells));
        for(Tile_cell_const_iterator c: cells)
            if(!tile->cell_is_foreign(c))
                return Cell_const_iterator(&tiles, tile, c);
        assert(false); // all incident cells are foreign, v should have been simplified !
        return cells_end();
    }

    /// @returns whether vertex v is incident to cell c. The operation is local in the tile of c
    bool has_vertex(const Cell_const_iterator& c, const Vertex_const_iterator& v) const
    {
        Tile_const_iterator ctile = c.tile();
        Tile_const_iterator vtile = v.tile();
        Tile_cell_const_iterator tc = *c;
        Tile_vertex_const_iterator tv = *v;
        if (ctile == vtile)
            for(int d = 0; d <= ctile->current_dimension(); ++d)
                if(tc->vertex(d) == tv)
                    return true;
        for(int d = 0; d <= ctile->current_dimension(); ++d)
            if(ctile->are_vertices_equal(ctile->vertex(tc, d), *vtile, tv))
                return true;
        return false;
    }

    /// @returns the index of the covertex of a facet f
    /// The operation is local iff the local cell of f is main
    inline int index_of_covertex(const Facet_const_iterator& f) const
    {
        assert(is_valid(f));
        Tile_const_iterator tile = f.tile();
        Tile_cell_const_iterator c = tile->cell(*f);
        if(tile->cell_is_main(c)) return local_index_of_covertex(f);
        return local_index_of_covertex(relocate(f, tile->minimum_id(c)));
    }

    /// @returns the covertex of a facet f
    /// The operation is local iff the local cell of f is not foreign
    Vertex_const_iterator covertex(const Facet_const_iterator& f) const
    {
        assert(is_valid(f));
        Tile_const_iterator tile = f.tile();
        Tile_cell_const_iterator c = tile->cell(*f);
        if(tile->cell_is_foreign(c)) return local_covertex(main(f)); // any non foreign representative could do
        return Vertex_const_iterator(&tiles, tile, tile->covertex(*f));
    }

    /// @returns the mirror_vertex of a facet f, as the covertex of its mirror facet.
    /// The operation is local iff the local cell of the mirror facet of f is not foreign
    Vertex_const_iterator mirror_vertex(const Facet_const_iterator& f) const
    {
        return covertex(mirror_facet(f));
    }


    /// @returns the facet defined by a cell and the index of its covertex.
    /// The operation is local iff the given cell is main, to ensure consistency across representatives of the cell.
    inline Facet_const_iterator facet(const Cell_const_iterator& c, int i) const
    {
        assert(is_valid(c));
        return local_facet(main(c),i); // i is defined wrt the main representative
    }

    /// @returns the neighboring cell, opposite to the ith vertex.
    /// The operation may require to change tile twice : once if the given cell c is not main, and once if the mirror_facet of facet(main(c),i) is foreign.
    inline Cell_const_iterator neighbor(const Cell_const_iterator& c, int i) const
    {
        assert(is_valid(c));
        return cell(mirror_facet(facet(c, i)));
    }

    /// @returns the mirror index of a cell, such that neighbor(neighbor(cell,i), mirror_index(cell,i))==cell .
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

    /// Access the ith vertex of the cell c in its local tile.
    /// Advanced use: Access is local, thus more more effective, but the vertex index i corresponds
    /// to the index in the local tile representative of cell c, which may not be consistent with
    /// the vertex ordering in its main representative.
    Vertex_const_iterator local_vertex (const Cell_const_iterator& c, const int i) const
    {
        assert(is_valid(c));
        Tile_const_iterator tile = c.tile();
        return Vertex_const_iterator(&tiles, tile, tile->vertex(*c, i));
    }

    /// gets the index of the covertex of f in its local cell
    /// Advanced use: Access is local, thus more more effective, but the returned index relates to
    /// the local cell incident to f. The vertex ordering of the local cell may not correspond to the one of
    /// its main representative.
    /// Precondition: the local cell of f is not foreign.
    inline int local_index_of_covertex(const Facet_const_iterator& f) const
    {
        assert(is_valid(f));
        Tile_const_iterator tile = f.tile();
        assert(!tile->cell_is_foreign(tile->cell(*f)));
        return tile->index_of_covertex(*f);
    }

    /// Constructs a facet locally given a cell and a local index i
    /// Advanced use: Access is local, thus more more effective, but the returned facet is defined using the local index i,
    /// which may indexing of the main representative of the cell c.
    Facet_const_iterator local_facet(const Cell_const_iterator& c, int i) const
    {
        assert(is_valid(c));
        Tile_const_iterator tile = c.tile();
        return Facet_const_iterator(&tiles, tile, tile->facet(*c, i));
    }

    /// gets the index of the mirror vertex of f locally
    /// Advanced use: Access is local, thus more more effective, but the returned index relates to the local indexing of the local cell of the mirror of f
    /// Precondition : the local cell of the mirror of f should not be foreign
    inline int local_mirror_index(const Facet_const_iterator& f) const
    {
        assert(is_valid(f));
        Tile_const_iterator tile = f.tile();
        Tile_cell_const_iterator c = tile->cell(*f);
        assert(!tile->cell_is_foreign(c));
        return tile->mirror_index(c,tile->index_of_covertex(*f));
    }

    /// @returns the full cell that is adjacent to the input facet f and that joins the covertex with the vertices of f
    /// Advanced use: Access is local, thus more more effective, but assumes that the local cell of f is not foreign.
    Cell_const_iterator local_cell(const Facet_const_iterator& f) const
    {
        assert(is_valid(f));
        Tile_const_iterator tile = f.tile();
        Tile_cell_const_iterator c = tile->cell(*f);
        assert(!tile->cell_is_foreign(c));
        return Cell_const_iterator(&tiles, tile, c);
    }

    /// @returns the covertex of the input facet f
    /// Advanced use: Access is local, thus more more effective, but assumes that the local cell of f is not foreign.
    Vertex_const_iterator local_covertex(const Facet_const_iterator& f) const
    {
        assert(is_valid(f));
        Tile_const_iterator tile = f.tile();
        Tile_cell_const_iterator c = tile->cell(*f);
        assert(!tile->cell_is_foreign(c));
        return Vertex_const_iterator(&tiles, tile, tile->covertex(*f));
    }
    /// @}

private:
    Tile_container& tiles; /// loaded tiles
};

}

#endif // CGAL_DISTRIBUTED_DELAUNAY_TRIANGULATION_H
