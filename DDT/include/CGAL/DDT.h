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

#ifndef CGAL_DDT_H
#define CGAL_DDT_H

#include <CGAL/DDT/iterator/Vertex_const_iterator.h>
#include <CGAL/DDT/iterator/Facet_const_iterator.h>
#include <CGAL/DDT/iterator/Cell_const_iterator.h>
#include <CGAL/DDT/tile.h>
#include <CGAL/DDT/tile_container.h>

#include <string>
#include <unordered_map>


namespace ddt
{

template<typename TileContainer>
class DDT
{
public:
    typedef TileContainer                            Tile_container;
    typedef typename Tile_container::Traits          Traits;
    typedef typename Tile_container::Tile            Tile;

    typedef typename Traits::Point                   Point;
    typedef typename Traits::Id                      Id;
    typedef typename Traits::Delaunay_triangulation  DT;
    typedef typename Traits::Vertex_handle           Tile_vertex_handle;
    typedef typename Traits::Vertex_iterator         Tile_vertex_iterator;
    typedef typename Traits::Vertex_const_handle     Tile_vertex_const_handle;
    typedef typename Traits::Vertex_const_iterator   Tile_vertex_const_iterator;
    typedef typename Traits::Cell_handle             Tile_cell_handle;
    typedef typename Traits::Cell_const_handle       Tile_cell_const_handle;
    typedef typename Traits::Cell_const_iterator     Tile_cell_const_iterator;
    typedef typename Traits::Facet_handle            Tile_facet_handle;
    typedef typename Traits::Facet_const_handle      Tile_facet_const_handle;
    typedef typename Traits::Facet_const_iterator    Tile_facet_const_iterator;
    typedef typename Tile_container::Tile_const_iterator  Tile_const_iterator;

    typedef std::pair<Tile_cell_const_handle,Id>     Tile_cell_const_handle_and_id;
    typedef std::pair<Tile_vertex_const_handle,Id>   Tile_vertex_const_handle_and_id;
    typedef std::tuple<Point,Id,Id>                  Point_id_source;

    /// Const iterator over the local vertices of selected tiles
    typedef ddt::Vertex_const_iterator<DDT>          Vertex_const_iterator;
    typedef ddt::Facet_const_iterator <DDT>          Facet_const_iterator;
    typedef ddt::Cell_const_iterator  <DDT>          Cell_const_iterator;

    enum { D = Traits::D };

    inline int maximal_dimension() const
    {
        return D;
    }

    DDT(TileContainer& tc) :
        tiles(tc)
    {
    }

    inline size_t number_of_cells   () const { return tiles.number_of_cells();    }
    inline size_t number_of_vertices() const { return tiles.number_of_vertices(); }
    inline size_t number_of_facets  () const { return tiles.number_of_facets();   }

    Vertex_const_iterator vertices_begin() const { return Vertex_const_iterator(tiles.cbegin(), tiles.cend()); }
    Vertex_const_iterator vertices_end  () const { return Vertex_const_iterator(tiles.cend(), tiles.cend()); }

    Cell_const_iterator cells_begin() const { return Cell_const_iterator(tiles.cbegin(), tiles.cend()); }
    Cell_const_iterator cells_end  () const { return Cell_const_iterator(tiles.cend(), tiles.cend()); }

    Facet_const_iterator facets_begin() const { return Facet_const_iterator(tiles.cbegin(), tiles.cend()); }
    Facet_const_iterator facets_end  () const { return Facet_const_iterator(tiles.cend(), tiles.cend()); }

    int vertex_id(Vertex_const_iterator v) const
    {
        if (is_infinite(v)) return -1;
        return std::distance(vertices_begin(), main(v));
    }

    int cell_id(Cell_const_iterator c) const
    {
        return std::distance(cells_begin(), main(c));
    }

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

    void next_ring(const std::set<Cell_const_iterator>& seeds, std::set<Cell_const_iterator>& next) const
    {
        for(auto seed : seeds)
        {
            for(int d = 0; d <= D; d++)
            {
                auto c = main(neighbor(seed, d));
                if(seeds.find(c) == seeds.end())
                    next.insert(c);
            }
        }
    }


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
                auto t = tiles.get_tile(tid);
                if(t->locate_vertex(*tile, v) == t->vertices_end())
                {
                    assert(! "locate_vertex failed" );
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
                    auto t = tiles.get_tile(tid);
                    if(t->locate_facet(*tile, f) == t->facets_end())
                    {
                      assert(! "locate_facet failed" );
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
                    auto t = tiles.get_tile(tid);
                    if(t->locate_cell(*tile, c) == t->cells_end())
                    {
                      assert(! "locate_facet failed" );
                        return false;
                    }
                }
            }
        }
        return true;
    }

    bool is_local(const Vertex_const_iterator& v) const { return v.tile()->vertex_is_local(v.vertex()); }
    bool is_local(const Facet_const_iterator&  f) const { return f.tile()->facet_is_local(f.facet()); }
    bool is_local(const Cell_const_iterator&   c) const { return c.tile()->cell_is_local(c.cell()); }

    bool is_valid(const Vertex_const_iterator& v) const { return v.tile()->vertex_is_valid(v.vertex()); } // + tile toujours chargée ?
    bool is_valid(const Facet_const_iterator&  f) const { return f.tile()->facet_is_valid(f.facet()); } // + tile toujours chargée ?
    bool is_valid(const Cell_const_iterator&   c) const { return c.tile()->cell_is_valid(c.cell()); } // + tile toujours chargée ?

    // vertices are never mixed
    bool is_mixed(const Facet_const_iterator& f) const { return f.tile()->facet_is_mixed(f.facet()); }
    bool is_mixed(const Cell_const_iterator&  c) const { return c.tile()->cell_is_mixed(c.cell()); }

    bool is_foreign(const Vertex_const_iterator& v) const { return v.tile()->vertex_is_foreign(v.vertex()); }
    bool is_foreign(const Facet_const_iterator&  f) const { return f.tile()->facet_is_foreign(f.facet()); }
    bool is_foreign(const Cell_const_iterator&   c) const { return c.tile()->cell_is_foreign(c.cell()); }

    bool is_main(const Vertex_const_iterator& v) const { return v.tile()->vertex_is_main(v.vertex()); }
    bool is_main(const Facet_const_iterator&  f) const { return f.tile()->facet_is_main(f.facet()); }
    bool is_main(const Cell_const_iterator&   c) const { return c.tile()->cell_is_main(c.cell()); }

    bool is_infinite(const Vertex_const_iterator& v) const { return v.tile()->vertex_is_infinite(v.vertex()); }
    bool is_infinite(const Facet_const_iterator&  f) const { return f.tile()->facet_is_infinite(f.facet()); }
    bool is_infinite(const Cell_const_iterator&   c) const { return c.tile()->cell_is_infinite(c.cell()); }

    Id main_id(const Vertex_const_iterator&v) const { return v.tile()->id(v.vertex()); }
    Id main_id(const Facet_const_iterator& f) const { return f.tile()->minimum_id(f.facet()); }
    Id main_id(const Cell_const_iterator&  c) const { return c.tile()->minimum_id(c.cell()); }

    Id tile_id(const Vertex_const_iterator& v) const { return v.tile()->id(); }
    Id tile_id(const Facet_const_iterator&  f) const { return f.tile()->id(); }
    Id tile_id(const Cell_const_iterator&   c) const { return c.tile()->id(); }

    /// Main
    Vertex_const_iterator locate(const Vertex_const_iterator& v, Id id) const
    {
        assert(is_valid(v));
        if (id == tile_id(v)) return v; // v is already in tile id

        // if (!is_loaded(id) ) load(id);
        Tile_container const& t = tiles; // why do we need this in a const function ?
        Tile_const_iterator tile = t.get_tile(id);
        Tile_vertex_const_iterator vertex = tile->locate_vertex(*(v.tile()), v.vertex());
        if (vertex==tile->vertices_end()) return vertices_end();
        return Vertex_const_iterator(tile, tiles.cend(), vertex);
    }

    Facet_const_iterator locate(const Facet_const_iterator& f, Id id) const
    {
        assert(is_valid(f));
        if (id == tile_id(f)) return f; // f is already in tile id

        Tile_container const& t = tiles; // why do we need this in a const function ?
        Tile_const_iterator tile = t.get_tile(id);
        Tile_facet_const_iterator facet = tile->locate_facet(*(f.tile()), f.facet());
        if (facet==tile->facets_end()) return facets_end();
        return Facet_const_iterator(tile, tiles.cend(), facet);
    }

    Cell_const_iterator locate(const Cell_const_iterator& c, Id id) const
    {
        assert(is_valid(c));
        if (id == tile_id(c)) return c; // c is already in tile id

        Tile_container const& t = tiles; // why do we need this in a const function ?
        Tile_const_iterator tile = t.get_tile(id);
        Tile_cell_const_iterator cell = tile->locate_cell(*(c.tile()), c.cell());
        if (cell==tile->cells_end()) return cells_end();
        return Cell_const_iterator(tile, tiles.cend(), cell);
    }

    inline Vertex_const_iterator main(const Vertex_const_iterator& v) const { return locate(v, main_id(v)); }
    inline Facet_const_iterator main(const Facet_const_iterator& f) const { return locate(f, main_id(f)); }
    inline Cell_const_iterator main(const Cell_const_iterator& c) const { return locate(c, main_id(c)); }

    inline Vertex_const_iterator infinite_vertex() const
    {
        assert(!tiles.empty());
        Tile_const_iterator tile = tiles.cbegin();
        return Vertex_const_iterator(tile, tiles.cend(), tile->infinite_vertex());
    }

    /// Access the ith vertex of the cell c.
    /// Result is consistent over all representatives of cell c, as its main representative is looked up.
    Vertex_const_iterator vertex (const Cell_const_iterator& c, const int i) const
    {
        assert(is_valid(c));
        return local_vertex(main(c), i); // i is defined wrt the main representative
    }

    /// Retrieve the point embedding of the vertex.
    /// This can be done locally without considering the main tile of the vertex, as point coordinates
    /// are replicated in all tile.
    const Point& point(Vertex_const_iterator v) const
    {
        assert(is_valid(v));
        return v.tile()->point(v.vertex());
    }

    /// @returns the mirror facet. This operation is performed locally: the resulting facet belongs
    /// to the same tile as the input facet.
    /// Precondition: the facet f is valid (ie: at least one of the facet points, the covertex and the mirror vertex is local)
    Facet_const_iterator mirror_facet(const Facet_const_iterator& f) const
    {
        assert(is_valid(f));
        Tile_const_iterator tile = f.tile();
        assert(tile->facet_is_valid(f.facet()));
        return Facet_const_iterator(tile, tiles.cend(), tile->mirror_facet(f.facet()));
    }

    // Access the mirror index of facet f, such that neighbor(cell(mirror_facet(f)), mirror_index)==cell(f)
    /// The main version of f is considered, as indices may not be consistent across
    /// the cell representatives of f and its mirror facets in other tiles
    inline int mirror_index(const Facet_const_iterator& f) const
    {
        assert(is_valid(f));
        return index_of_covertex(mirror_facet(f));
    }

    /// @returns the full cell that is adjacent to the input facet f and that joins the covertex with the vertices of f
    Cell_const_iterator cell(const Facet_const_iterator& f) const
    {
        assert(is_valid(f));
        Tile_const_iterator tile = f.tile();
        Tile_cell_const_iterator c = tile->cell(f.facet());
        if(tile->cell_is_foreign(c)) return local_cell(main(f)); // any non foreign representative could do
        return Cell_const_iterator(tile, tiles.cend(), c);
    }

    /// @returns the index of the covertex
    inline int index_of_covertex(const Facet_const_iterator& f) const
    {
        assert(is_valid(f));
        Tile_const_iterator tile = f.tile();
        Tile_cell_const_iterator c = tile->cell(f.facet());
        if(tile->cell_is_main(c)) return local_index_of_covertex(f);
        return local_index_of_covertex(locate(f, tile->minimum_id(c)));
    }

    Vertex_const_iterator covertex(const Facet_const_iterator& f) const
    {
        assert(is_valid(f));
        Tile_const_iterator tile = f.tile();
        Tile_cell_const_iterator c = tile->cell(f.facet());
        if(tile->cell_is_foreign(c)) return local_covertex(main(f)); // any non foreign representative could do
        return Vertex_const_iterator(tile, tiles.cend(), tile->covertex(f.facet()));
    }

    Vertex_const_iterator mirror_vertex(const Facet_const_iterator& f) const
    {
        return covertex(mirror_facet(f));
    }


    /// Cell iterator functions
    inline Facet_const_iterator facet(const Cell_const_iterator& c, int i) const
    {
        assert(is_valid(c));
        return local_facet(main(c),i); // i is defined wrt the main representative
    }

    inline Cell_const_iterator neighbor(const Cell_const_iterator& c, int i) const
    {
        assert(is_valid(c));
        return cell(mirror_facet(facet(c, i)));
    }

    inline int mirror_index(const Cell_const_iterator& c, int i) const
    {
        assert(is_valid(c));
        return mirror_index(facet(c,i));
    }

    /// Access the ith vectex of the cell c in its local tile.
    /// Advanced use: Access is local, thus more more effective, but the vertex index i corresponds
    /// to the index in the local tile representative of cell c, which may not be consistent with
    /// the one its in main representative.
    Vertex_const_iterator local_vertex (const Cell_const_iterator& c, const int i) const
    {
        assert(is_valid(c));
        Tile_const_iterator tile = c.tile();
        return Vertex_const_iterator(tile, tiles.cend(), tile->vertex(c.cell(), i));
    }

    inline int local_index_of_covertex(const Facet_const_iterator& f) const
    {
        assert(is_valid(f));
        Tile_const_iterator tile = f.tile();
        return tile->index_of_covertex(f.facet());
    }

    Facet_const_iterator local_facet(const Cell_const_iterator& c, int i) const
    {
        assert(is_valid(c));
        Tile_const_iterator tile = c.tile();
        return Facet_const_iterator(tile, tiles.cend(), tile->facet(c.cell(), i));
    }

    inline int local_mirror_index(const Facet_const_iterator& f) const
    {
        assert(is_valid(f));
        Tile_const_iterator tile = f.tile();
        Tile_cell_const_iterator c = tile->cell(f.facet());
        assert(!tile->cell_is_foreign(c));
        return tile->mirror_index(c,tile->index_of_covertex(f.facet()));
    }

    /// @returns the full cell that is adjacent to the input facet f and that joins the covertex with the vertices of f
    Cell_const_iterator local_cell(const Facet_const_iterator& f) const
    {
        assert(is_valid(f));
        Tile_const_iterator tile = f.tile();
        Tile_cell_const_iterator c = tile->cell(f.facet());
        assert(!tile->cell_is_foreign(c));
        return Cell_const_iterator(tile, tiles.cend(), c);
    }

    Vertex_const_iterator local_covertex(const Facet_const_iterator& f) const
    {
        assert(is_valid(f));
        Tile_const_iterator tile = f.tile();
        Tile_cell_const_iterator c = tile->cell(f.facet());
        assert(!tile->cell_is_foreign(c));
        return Vertex_const_iterator(tile, tiles.cend(), tile->covertex(f.facet()));
    }

private:
    Tile_container& tiles; /// loaded tiles
};

}

#endif // CGAL_DDT_H
