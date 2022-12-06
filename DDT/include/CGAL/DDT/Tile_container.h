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
    using value_type = typename Map_iterator::value_type::second_type;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

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
    using value_type = typename Map_const_iterator::value_type::second_type;
    using difference_type = std::ptrdiff_t;
    using pointer = const value_type*;
    using reference = const value_type&;

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
    typedef typename Traits::Bbox                    Bbox;
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


    typedef Mapped_const_iterator<typename std::map<Id, Tile>::const_iterator>  Tile_const_iterator ;
    typedef Mapped_iterator<typename std::map<Id, Tile>::iterator>              Tile_iterator ;

    typedef std::set<Id> Tile_id_set;
    typedef typename Tile_id_set::const_iterator Tile_id_const_iterator;

    inline constexpr int maximal_dimension() const
    {
        return traits.dimension();
    }

    Tile_container(int dimension, size_t max_number_of_tiles = 0, const Serializer& serializer = Serializer()) :
        traits(dimension),
        tiles(),
        serializer(serializer),
        number_of_finite_vertices_(0),
        number_of_finite_facets_  (0),
        number_of_finite_cells_   (0),
        number_of_facets_  (0),
        number_of_cells_   (0),
        max_number_of_tiles(max_number_of_tiles)
    {
        if (max_number_of_tiles == 0) this->max_number_of_tiles = std::numeric_limits<size_t>::max();
    }

    inline size_t maximum_number_of_tiles() const { return max_number_of_tiles; }

    Tile_id_const_iterator tile_ids_begin() const { return ids.begin(); }
    Tile_id_const_iterator tile_ids_end  () const { return ids.end  (); }

    bool empty() const { return tiles.empty(); }
    Tile_const_iterator cbegin  () const { return tiles.begin (); }
    Tile_const_iterator cend    () const { return tiles.end   (); }
    Tile_const_iterator begin  () const { return tiles.begin (); }
    Tile_const_iterator end    () const { return tiles.end   (); }
    Tile_const_iterator find(Id id) const { return tiles.find(id); }
    Tile_iterator begin  () { return tiles.begin (); }
    Tile_iterator end    () { return tiles.end   (); }
    Tile_iterator find(Id id) { return tiles.find(id); }

    typedef std::map<Id, Bbox> Bbox_map;
    typedef typename Tile::Points Points;
    typedef typename Tile::Points_map Points_map;
    const Bbox_map& bboxes() const { return bboxes_; }
    Bbox_map& bboxes() { return bboxes_; }
    const Points_map& points() const { return points_; }
    Points_map& points() { return points_; }
    const Points& extreme_points() const { return extreme_points_; }
    Points& extreme_points() { return extreme_points_; }

    void receive_points(Tile& tile, Points& received) {
        Id id = tile.id();
        received.swap(points_[id]);
        size_t number_of_extreme_points = extreme_points_.size();
        received.insert(received.end(),
                        extreme_points_.begin() + tile.number_of_extreme_points_received,
                        extreme_points_.end());
        tile.number_of_extreme_points_received = number_of_extreme_points;
    }

    void send_point_to_its_tile(Id id, const Point& p) {
        points_[id].push_back({id,p});
        init(id);
    }

    size_t send_vertices_to_one_tile(const Tile& tile, const std::map<Id, std::set<Tile_vertex_const_handle>>& vertices) {
        size_t count = 0;
        for(auto& vi : vertices)
        {
            count += vi.second.size();
            Points& points = points_[vi.first];
            for(Tile_vertex_const_handle v : vi.second)
                points.emplace_back(tile.vertex_id(v), tile.point(v));
        }
        return count;
    }

    void send_vertices_to_all_tiles(const Tile& tile, const std::vector<Tile_vertex_const_handle>& vertices) {
        for(Tile_vertex_const_handle v : vertices) {
           if (tile.vertex_is_infinite(v)) continue;
           Id id = tile.vertex_id(v);
           Point p = tile.point(v);
           extreme_points_.emplace_back(id, p);
           bboxes_[id] += p;
        }
    }

    /*
     *             typename TileContainer::Tile_iterator tile = tc.find(*it);
            if(tile == tc.end()) {
                while(tc.tiles.size() >= tc.maximum_number_of_tiles()) {
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

    void init(Id id)
    {
        ids.insert(id);
    }

     /*     auto it = begin();
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
       */

    /// if necessary, tiles are automatically unloaded
    /// @return pair of tile iterator and bool : the tile is either end() on failure or has the query id. The bool is true upon insertion of a new empty tile for deferred loading.
    /// @todo implement other tile eviction strategies than the random strategy : LRU, prioritized by inbox size...
    std::pair<Tile_iterator, bool> insert(Id id)
    {
        // return it if already loaded
        Tile_iterator tile = tiles.find(id);
        if (tile != end()) {
            tile->in_use = true;
            return {tile, false};
        }

        // make room if necessary
        while(tiles.size() >= maximum_number_of_tiles()) {
            // pick a loaded id at random and try to unload it
            size_t n = rand() % tiles.size();
            Tile_iterator it = begin();
            std::advance(it, n);
            if (it->id()!=id && !erase(it))
                return {end(), false};
        }

        Tile t(id, traits);
        t.in_use = true;

        // initialize an empty tile
        return tiles.emplace(id, std::move(t));
    }

    /// @todo attention à la perennité des handles (tile is possibly unloaded), ou alors lock ou shared pointer.
    /// unload a tile from memory, automatically saving it.
    /// returns true after the loaded tile id is successfully saved and unloaded from memory.
    bool erase(Tile_iterator tile) {
        assert(tile != end());
        if (tile->in_use || !serializer.save(*tile)) return false;
        return tiles.erase(tile->id());
    }

    /// Instead of using `load(id)`, use this function on the output of `insert` to load for better multithreading,
    /// as a lock is only required during `insert` calls and but not during this `load` call.
    /// @return whether the tile iterator is not at end and loading succeeded if a serialization was available.
    bool load(const std::pair<Tile_iterator, bool>& insertion) const {
        Tile_iterator tile = insertion.first;
        if(Tile_const_iterator(tile) == end()) return false;
        // deserialize it if possible
        if (insertion.second && serializer.has_tile(tile->id()))
            serializer.load(*tile); // / @todo handle loading error
        return true;
    }

    /// ensure tile id is loaded, possibly using deserialization. not threadsafe.
    Tile_iterator load(Id id) {
        std::pair<Tile_iterator, bool> insertion;
        do { insertion = insert(id); } while (!load(insertion));
        return insertion.first;
    }

    /// mark the tile as a candidate for unloading (no longer in use)
    void unload(Tile_iterator tile) const
    {
        assert(Tile_const_iterator(tile) != end());
        tile->in_use = false;
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


    bool is_valid(bool verbose = false, int level = 0) const
    {
        size_t number_of_finite_vertices = 0;
        size_t number_of_finite_facets = 0;
        size_t number_of_finite_cells = 0;
        size_t number_of_facets = 0;
        size_t number_of_cells = 0;
        for(auto tile = begin(); tile != end(); ++tile)
        {
            if(!tile->is_valid(verbose, level))
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
    Tile_id_set ids;
    std::map<Id, Tile> tiles;
    Bbox_map bboxes_;
    Points_map points_;

    Points extreme_points_;
    Serializer serializer;
    Traits traits;

    size_t number_of_finite_vertices_;
    size_t number_of_finite_facets_;
    size_t number_of_finite_cells_;
    size_t number_of_facets_;
    size_t number_of_cells_;
    size_t max_number_of_tiles;
};

}
}

#endif // CGAL_DDT_TILE_CONTAINER_H
