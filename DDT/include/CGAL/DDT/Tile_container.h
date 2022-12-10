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
#include <CGAL/DDT/serializer/No_serializer.h>
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

template<typename Map_iterator>
class Mapped_iterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = typename Map_iterator::value_type::second_type&;
    using difference_type = std::ptrdiff_t;
    using pointer = typename Map_iterator::value_type::second_type*;
    using reference = typename Map_iterator::value_type::second_type&;

    Mapped_iterator ( ) : it ( ) { }
    Mapped_iterator ( Map_iterator it_ ) : it ( it_ ) { }
    template<typename Map_const_iterator> friend class Mapped_const_iterator;

    pointer operator -> ( ) const { return &(it->second); }
    reference operator * ( ) const { return it->second; }
    bool operator==(const Mapped_iterator& rhs) const { return it == rhs.it; }
    bool operator!=(const Mapped_iterator& rhs) const { return it != rhs.it; }
    Mapped_iterator& operator++() { ++it; return *this; }

private:
    Map_iterator it;
};

template<typename Map_const_iterator>
class Mapped_const_iterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = typename Map_const_iterator::value_type::second_type const&;
    using difference_type = std::ptrdiff_t;
    using pointer =  typename Map_const_iterator::value_type::second_type const*;
    using reference =  typename Map_const_iterator::value_type::second_type const&;

    Mapped_const_iterator ( ) : it ( ) { }
    Mapped_const_iterator ( Map_const_iterator it_ ) : it ( it_ ) { }
    template<typename Map_iterator>
    Mapped_const_iterator ( Mapped_iterator<Map_iterator> it_ ) : it ( it_.it ) { }

    pointer operator -> ( ) const { return &(it->second); }
    reference operator * ( ) const { return it->second; }
    bool operator==(const Mapped_const_iterator& rhs) const { return it == rhs.it; }
    bool operator!=(const Mapped_const_iterator& rhs) const { return it != rhs.it; }
    Mapped_const_iterator& operator++() { ++it; return *this; }

private:
    Map_const_iterator it;
};

/// \ingroup PkgDDTClasses
template<typename _Traits, typename _Tile = CGAL::DDT::Tile<_Traits>, typename Serializer = No_serializer<_Tile> >
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

    typedef std::map<Id, Tile>                         Container;
    typedef typename Container::iterator               Pair_iterator;
    typedef typename Container::const_iterator         Pair_const_iterator;
    typedef Mapped_const_iterator<Pair_const_iterator> const_iterator ;
    typedef Mapped_iterator<Pair_iterator>             iterator ;
    typedef Key_const_iterator<Pair_const_iterator>    Id_const_iterator ;

    typedef typename Tile::Points             Points;
    typedef typename Tile::Tile_triangulation Tile_triangulation;

    inline constexpr int maximal_dimension() const
    {
        return traits.dimension();
    }

    Tile_container(int dimension, size_t number_of_triangulations_mem_max = 0, const Serializer& serializer = Serializer()) :
        traits(dimension),
        tiles(),
        serializer_(serializer),
        number_of_finite_vertices_(0),
        number_of_finite_facets_  (0),
        number_of_finite_cells_   (0),
        number_of_facets_  (0),
        number_of_cells_   (0),
        number_of_triangulations_mem_(0),
        number_of_triangulations_mem_max_(number_of_triangulations_mem_max)
    {
        if (number_of_triangulations_mem_max_ == 0) number_of_triangulations_mem_max_ = std::numeric_limits<size_t>::max();
    }

    inline size_t number_of_triangulations_mem_max() const { return number_of_triangulations_mem_max_; }
    inline size_t number_of_triangulations_mem() const { return number_of_triangulations_mem_; }

    Id_const_iterator ids_begin() const { return tiles.begin(); }
    Id_const_iterator ids_end  () const { return tiles.end  (); }

    bool empty() const { return tiles.empty(); }
    const_iterator cbegin  () const { return tiles.begin (); }
    const_iterator cend    () const { return tiles.end   (); }
    const_iterator begin  () const { return tiles.begin (); }
    const_iterator end    () const { return tiles.end   (); }
    const_iterator find(Id id) const { return tiles.find(id); }
    iterator begin  () { return tiles.begin (); }
    iterator end    () { return tiles.end   (); }
    iterator find(Id id) { return tiles.find(id); }

    Tile& operator[](Id id) { return tiles.emplace(id, std::move(Tile(id, traits))).first->second; }
    const Tile& at(Id id) const { return tiles.at(id); }
    Tile& at(Id id) { return tiles.at(id); }

    const Points& extreme_points() const { return extreme_points_; }
    Points& extreme_points() { return extreme_points_; }

    void receive_points(Tile& tile, Points& received) {
        received.swap(tile.points());
        size_t number_of_extreme_points = extreme_points_.size();
        received.insert(received.end(),
                        extreme_points_.begin() + tile.number_of_extreme_points_received,
                        extreme_points_.end());
        tile.number_of_extreme_points_received = number_of_extreme_points;

        // debug
        //if(!received.empty()) std::cout << "\x1B[31m" << tile.id() << "\t<-\t*\t:\t" << received.size()   << "\x1B[0m" << std::endl;

    }

    void send_point_to_its_tile(Id id, const Point& p) {
        operator[](id).points().push_back({id,p});
    }

    size_t send_vertices_to_one_tile(const Tile& tile, const std::map<Id, std::set<Tile_vertex_const_handle>>& vertices) {
        size_t count = 0;
        for(auto& vi : vertices)
        {
            count += vi.second.size();
            Points& points = operator[](vi.first).points();
            for(Tile_vertex_const_handle v : vi.second)
                points.emplace_back(tile.triangulation().vertex_id(v), tile.triangulation().point(v));

            // debug
            //if(!vi.second.empty()) std::cout << "\x1B[32m" << tile.id() << "\t->\t" << size_t(vi.first) << "\t:\t" << vi.second.size()   << "\x1B[0m"<< std::endl;
        }
        return count;
    }

    void send_vertices_to_all_tiles(const Tile& tile, const std::vector<Tile_vertex_const_handle>& vertices) {
        for(Tile_vertex_const_handle v : vertices) {
           if (tile.triangulation().vertex_is_infinite(v)) continue;
           Id id = tile.triangulation().vertex_id(v);
           Point p = tile.triangulation().point(v);
           extreme_points_.emplace_back(id, p);
        }
        // debug
        //if(!vertices.empty()) std::cout << "\x1B[33m" << tile.id() << "\t->\t*\t:\t" << vertices.size()   << "\x1B[0m" << std::endl;
    }

    /*
     *             typename TileContainer::Tile_iterator tile = tc.find(*it);
            if(tile == tc.end()) {
                while(tc.number_of_triangulations_mem_ >= tc.number_of_triangulations_mem_max_) {
                    auto it = tc.begin();
                    Id id0 = it->id();
                    size_t count0 = inbox[id0].size();
                    for(++it; it != tc.end() && count0; ++it)
                    {
                        Id id = it->id();
                        size_t count = inbox[id].size();
                        if(count0 > count) {
                            count0 = count;
                            id0 = id;
                        }
                    }
                    tc.unload(id0);
                }
*/

    /// unload a tile from memory, automatically saving it.
    /// returns true after the loaded tile id is successfully saved and unloaded from memory.
    /// @todo attention à la perennité des handles (tile is possibly unloaded), ou alors lock ou shared pointer.
    void unload(Tile& tile) {
        if (!tile.locked && tile.in_mem && serializer_.save(tile)) {
            tile.triangulation().clear();
            tile.in_mem = false;
            --number_of_triangulations_mem_;
            std::cout << int(tile.id()) << " saved and cleared : " << number_of_triangulations_mem_ << " -> ";
            for(const Tile& t : *this) {
                if(t.locked) std::cout <<  "\x1B[31m";
                if(t.in_mem) std::cout << size_t(t.id()); else std::cout << "_";
                if(t.locked) std::cout <<  "\x1B[0m";
            }
            std::cout << std::endl;
        }
    }

    /// load a tile to memory, automatically saving it.
    void load(Tile& tile) {
        // make room if necessary
        while(number_of_triangulations_mem_ >= number_of_triangulations_mem_max_) {
            // pick a loaded id at random and try to unload it
            size_t n = rand() % tiles.size();
            iterator it = begin();
            std::advance(it, n);
            if (it->id()!=tile.id()) unload(*it);
        }

        if (!tile.in_mem && (!serializer_.has_tile(tile.id()) || serializer_.load(tile))) {
            tile.in_mem = true;
            ++number_of_triangulations_mem_;
            std::cout << int(tile.id()) << " read  and loaded  : " << number_of_triangulations_mem_ << " -> ";
            for(const Tile& t : *this) {
                if(t.locked) std::cout <<  "\x1B[31m";
                if(t.in_mem) std::cout << size_t(t.id()); else std::cout << "_";
                if(t.locked) std::cout <<  "\x1B[0m";
            }
            std::cout << std::endl;
        }
    }

    /// @return whether the tile iterator is not at end and loading succeeded if a serialization was available.
    /// if necessary, tiles are automatically unloaded
    /// @todo implement other tile eviction strategies than the random strategy : LRU, prioritized by inbox size...
    void lock(Tile& tile) {
        tile.locked = true;
    }

    /// mark the tile as a candidate for unloading (no longer in use)
    void unlock(Tile& tile) const
    {
        tile.locked = false;
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
        for(Tile& tile : *this)
        {
            tile.finalize();
            number_of_finite_vertices_ += tile.number_of_main_finite_vertices();
            number_of_finite_facets_ += tile.number_of_main_finite_facets();
            number_of_finite_cells_ += tile.number_of_main_finite_cells();
            number_of_facets_ += tile.number_of_main_facets();
            number_of_cells_ += tile.number_of_main_cells();
        }
    }

    inline size_t number_of_finite_vertices() const { return number_of_finite_vertices_; }
    inline size_t number_of_finite_facets  () const { return number_of_finite_facets_;   }
    inline size_t number_of_finite_cells   () const { return number_of_finite_cells_;    }
    inline size_t number_of_vertices() const { return number_of_finite_vertices_ + 1; }
    inline size_t number_of_facets  () const { return number_of_facets_;   }
    inline size_t number_of_cells   () const { return number_of_cells_;    }


    bool is_valid(bool verbose = false, int level = 0) const
    {
        size_t number_of_finite_vertices = 0;
        size_t number_of_finite_facets = 0;
        size_t number_of_finite_cells = 0;
        size_t number_of_facets = 0;
        size_t number_of_cells = 0;

        for(const Tile& tile : *this)
        {

            if(!tile.is_valid(verbose, level))
            {
                std::cerr << "Tile " << int(tile.id()) << " is invalid" << std::endl;
                //assert(! "CGAL tile not valid" );
                return false;
            }
            number_of_finite_vertices += tile.number_of_main_finite_vertices();
            number_of_finite_facets += tile.number_of_main_finite_facets();
            number_of_finite_cells += tile.number_of_main_finite_cells();
            number_of_facets += tile.number_of_main_facets();
            number_of_cells += tile.number_of_main_cells();
        }
        if (number_of_finite_vertices != number_of_finite_vertices_) { std::cerr << "incorrect number_of_finite_vertices" << std::endl; return false; }
        if (number_of_finite_facets != number_of_finite_facets_) { std::cerr << "incorrect number_of_finite_facets" << std::endl; return false; }
        if (number_of_finite_cells != number_of_finite_cells_) { std::cerr << "incorrect number_of_finite_cells" << std::endl; return false; }
        if (number_of_facets != number_of_facets_) { std::cerr << "incorrect number_of_facets" << std::endl; return false; }
        if (number_of_cells != number_of_cells_) { std::cerr << "incorrect number_of_cells" << std::endl; return false; }
        return true;
    }

    const Serializer& serializer() const { return serializer_; }

private:
    Container tiles;
    Points extreme_points_;
    Serializer serializer_;
    Traits traits;

    size_t number_of_finite_vertices_;
    size_t number_of_finite_facets_;
    size_t number_of_finite_cells_;
    size_t number_of_facets_;
    size_t number_of_cells_;
    size_t number_of_triangulations_mem_max_;
    size_t number_of_triangulations_mem_;
};

}
}

#endif // CGAL_DDT_TILE_CONTAINER_H
