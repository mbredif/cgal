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

#ifndef CGAL_DDT_TILE_CONTAINER_H
#define CGAL_DDT_TILE_CONTAINER_H

#include <CGAL/DDT/Tile.h>
#include <map>

namespace CGAL {
namespace DDT {

template<typename Map_const_iterator>
class Key_const_iterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = typename Map_const_iterator::value_type::first_type;
    using difference_type = std::ptrdiff_t;
    using pointer = const value_type*;
    using reference = const value_type&;

    Key_const_iterator ( ) : it ( ) { }
    Key_const_iterator ( Map_const_iterator it_ ) : it( it_ ) { }

    pointer operator -> ( ) const { return &(it->first); }
    reference operator * ( ) const { return it->first; }
    bool operator==(const Key_const_iterator& rhs) const { return it == rhs.it; }
    bool operator!=(const Key_const_iterator& rhs) const { return it != rhs.it; }
    Key_const_iterator& operator++() { ++it; return *this; }

private:
    Map_const_iterator it;
};

template<typename Map_const_iterator>
class Mapped_const_iterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = typename Map_const_iterator::value_type::second_type;
    using difference_type = std::ptrdiff_t;
    using pointer = const value_type*;
    using reference = const value_type&;

    Mapped_const_iterator ( ) : it ( ) { }
    Mapped_const_iterator ( Map_const_iterator it_ ) : it ( it_ ) { }

    pointer operator -> ( ) const { return &(it->second); }
    reference operator * ( ) const { return it->second; }
    bool operator==(const Mapped_const_iterator& rhs) const { return it == rhs.it; }
    bool operator!=(const Mapped_const_iterator& rhs) const { return it != rhs.it; }
    Mapped_const_iterator& operator++() { ++it; return *this; }

private:
    Map_const_iterator it;
};

template<typename Map_iterator>
class Mapped_iterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = typename Map_iterator::value_type::second_type;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    Mapped_iterator ( ) : it ( ) { }
    Mapped_iterator ( Map_iterator it_ ) : it ( it_ ) { }

    pointer operator -> ( ) const { return &(it->second); }
    reference operator * ( ) const { return it->second; }
    bool operator==(const Mapped_iterator& rhs) const { return it == rhs.it; }
    bool operator!=(const Mapped_iterator& rhs) const { return it != rhs.it; }
    Mapped_iterator& operator++() { ++it; return *this; }

private:
    Map_iterator it;
};

/// \ingroup PkgDDTClasses
template<typename _Traits, typename Serializer, typename _Tile = CGAL::DDT::Tile<_Traits>>
class Tile_container
{
public:
    typedef _Traits                                  Traits;
    typedef _Tile                                    Tile;

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

    typedef std::map<Id, Tile>                                              Container;
    typedef Mapped_const_iterator<typename Container::const_iterator>  Tile_const_iterator ;
    typedef Mapped_iterator<typename Container::iterator>              Tile_iterator ;
    typedef Key_const_iterator<typename Container::const_iterator>     Tile_id_const_iterator ;

    // typedef std::set<Id> Tile_id_set;
    // typedef typename Tile_id_set::const_iterator Tile_id_set_const_iterator;

    enum { D = Traits::D };

    inline int maximal_dimension() const
    {
        return D;
    }

    Tile_container(Serializer& serializer) :
        tiles(),
        serializer(serializer),
        number_of_finite_vertices_(0),
        number_of_finite_facets_  (0),
        number_of_finite_cells_   (0),
        number_of_facets_  (0),
        number_of_cells_   (0)
    {
    }

    inline size_t number_of_tiles   () const { return tiles.size();   }

    Tile_id_const_iterator tile_ids_begin() const { return tiles.begin(); }
    Tile_id_const_iterator tile_ids_end  () const { return tiles.end  (); }


    bool empty() const { return tiles.empty(); }
    Tile_const_iterator cbegin  () const { return tiles.begin (); }
    Tile_const_iterator cend    () const { return tiles.end   (); }
    Tile_const_iterator begin  () const { return tiles.begin (); }
    Tile_const_iterator end    () const { return tiles.end   (); }
    Tile_const_iterator get_tile(Id id) const { return tiles.find(id); }

    Tile_iterator begin  () { return tiles.begin (); }
    Tile_iterator end    () { return tiles.end   (); }
    Tile_iterator get_tile(Id id) { return tiles.find(id); }
    bool is_loaded(Id id) const { return tiles.find(id) != tiles.end(); }

    void init(Id id)
    {
        tiles.emplace(id, id);
    }

    /// @todo attention à la perennité des handles (tile is possibly unloaded), ou alors lock ou shared pointer.
    /// unload a tile from memory (no automatic saving)
    void unload(Id id)
    {
        save(id);
        tiles.remove(id);
    }

    /// load the tile using the serializer, given its id.
    void load(Id id)
    {
        /// @todo unload tiles if needed
        tiles.emplace(id, serializer.load(id));
    }

    /// saves a tile using the serializer (no unloading)
    void save(Id id)
    {
        serializer.save(tiles[id]);
    }

    void get_adjacency_graph(std::unordered_multimap<Id,Id>& edges) const
    {
        for(auto tile = begin(); tile != end(); ++tile)
        {
            std::set<Id> out_edges;
            tile->get_adjacency_graph_edges(out_edges);
            Id source = tile->id();
            for(Id target : out_edges)
                edges.emplace(source, target);
        }
    }

    bool is_adjacency_graph_symmetric() const
    {
        std::unordered_multimap<Id,Id> edges;
        std::unordered_multimap<Id,Id> reversed;
        get_adjacency_graph(edges);
        for(auto& edge : edges)
            reversed.emplace(edge.second, edge.first);
        return edges == reversed;
    }

    void finalize()
    {
        number_of_finite_vertices_ = 0;
        number_of_finite_facets_ = 0;
        number_of_finite_cells_ = 0;
        number_of_facets_ = 0;
        number_of_cells_ = 0;
        for(Tile_iterator tile = begin(); tile != end(); ++tile)
        {
            tile->finalize();
            number_of_finite_vertices_ += tile->number_of_main_finite_vertices();
            number_of_finite_facets_ += tile->number_of_main_finite_facets();
            number_of_finite_cells_ += tile->number_of_main_finite_cells();
            number_of_facets_ += tile->number_of_main_facets();
            number_of_cells_ += tile->number_of_main_cells();
        }
    }

    inline size_t number_of_finite_vertices() const { return number_of_finite_vertices_; }
    inline size_t number_of_finite_facets  () const { return number_of_finite_facets_;   }
    inline size_t number_of_finite_cells   () const { return number_of_finite_cells_;    }
    inline size_t number_of_vertices() const { return number_of_finite_vertices_ + 1; }
    inline size_t number_of_facets  () const { return number_of_facets_;   }
    inline size_t number_of_cells   () const { return number_of_cells_;    }


    bool is_valid() const
    {
        size_t number_of_finite_vertices = 0;
        size_t number_of_finite_facets = 0;
        size_t number_of_finite_cells = 0;
        size_t number_of_facets = 0;
        size_t number_of_cells = 0;
        for(auto tile = begin(); tile != end(); ++tile)
        {
            if(!tile->is_valid())
            {
                std::cerr << "Tile " << int(tile->id()) << " is invalid" << std::endl;
                //assert(! "CGAL tile not valid" );
                return false;
            }
            number_of_finite_vertices += tile->number_of_main_finite_vertices();
            number_of_finite_facets += tile->number_of_main_finite_facets();
            number_of_finite_cells += tile->number_of_main_finite_cells();
            number_of_facets += tile->number_of_main_facets();
            number_of_cells += tile->number_of_main_cells();
        }
        if (number_of_finite_vertices != number_of_finite_vertices_) { std::cerr << "incorrect number_of_finite_vertices" << std::endl; return false; }
        if (number_of_finite_facets != number_of_finite_facets_) { std::cerr << "incorrect number_of_finite_facets" << std::endl; return false; }
        if (number_of_finite_cells != number_of_finite_cells_) { std::cerr << "incorrect number_of_finite_cells" << std::endl; return false; }
        if (number_of_facets != number_of_facets_) { std::cerr << "incorrect number_of_facets" << std::endl; return false; }
        if (number_of_cells != number_of_cells_) { std::cerr << "incorrect number_of_cells" << std::endl; return false; }
        return true;
    }

private:
    Container tiles; /// loaded tiles
    Serializer& serializer;

    size_t number_of_finite_vertices_;
    size_t number_of_finite_facets_;
    size_t number_of_finite_cells_;
    size_t number_of_facets_;
    size_t number_of_cells_;
};

}
}

#endif // CGAL_DDT_TILE_CONTAINER_H
