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

#include <CGAL/DDT/serializer/No_serializer.h>

#include <map>
#include <iomanip>
#include <limits>

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
    Key_const_iterator operator++(int) { Key_const_iterator it(*this); ++it; return it; }
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
    Mapped_iterator operator++(int) { Mapped_iterator it(*this); ++it; return it; }

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
    Mapped_const_iterator operator++(int) { Mapped_const_iterator it(*this); ++it; return it; }

private:
    Map_const_iterator it;
};

/// \ingroup PkgDDTClasses
/// Tile Container
template<typename Tile_,
         typename Serializer_ = No_serializer >
class Tile_container
{
public:
    typedef Tile_                                      Tile;
    typedef Serializer_                                Serializer;

    typedef typename Tile::Tile_index                  Tile_index;
    typedef std::map<Tile_index, Tile>                 Container;
    typedef typename Container::iterator               Pair_iterator;
    typedef typename Container::const_iterator         Pair_const_iterator;
    typedef Mapped_const_iterator<Pair_const_iterator> const_iterator ;
    typedef Mapped_iterator<Pair_iterator>             iterator ;
    typedef Key_const_iterator<Pair_const_iterator>    Tile_index_const_iterator ;

    typedef typename Tile::Tile_triangulation          Tile_triangulation;

    inline constexpr int maximal_dimension() const
    {
        return dimension_;
    }

    Tile_container(int dimension, std::size_t number_of_tiles_mem_max = 0, const Serializer& serializer = Serializer()) :
        tiles(),
        dimension_(dimension),
        serializer_(serializer),
        number_of_tiles_mem_(0),
        number_of_tiles_mem_max_(number_of_tiles_mem_max)
    {
        if (number_of_tiles_mem_max_ == 0) number_of_tiles_mem_max_ = std::numeric_limits<std::size_t>::max();
    }

    inline std::size_t number_of_tiles_mem_max() const { return number_of_tiles_mem_max_; }
    inline std::size_t number_of_tiles_mem() const { return number_of_tiles_mem_; }

    Tile_index_const_iterator ids_begin() const { return tiles.begin(); }
    Tile_index_const_iterator ids_end  () const { return tiles.end  (); }

    bool empty() const { return tiles.empty(); }
    const_iterator cbegin  () const { return tiles.begin (); }
    const_iterator cend    () const { return tiles.end   (); }
    const_iterator begin  () const { return tiles.begin (); }
    const_iterator end    () const { return tiles.end   (); }
    const_iterator find(Tile_index id) const { return tiles.find(id); }
    iterator begin  () { return tiles.begin (); }
    iterator end    () { return tiles.end   (); }
    iterator find(Tile_index id) { return tiles.find(id); }

    std::pair<Pair_iterator,bool> emplace(Tile_index id) { return tiles.emplace(id, std::move(Tile(id, dimension_))); }
    Tile& operator[](Tile_index id) { return emplace(id).first->second; }
    const Tile& at(Tile_index id) const { return tiles.at(id); }
    Tile& at(Tile_index id) { return tiles.at(id); }

    /*
     *             typename TileContainer::Tile_const_iterator tile = tc.find(*it);
            if(tile == tc.end()) {
                while(tc.number_of_tiles_mem_ >= tc.number_of_tiles_mem_max_) {
                    auto it = tc.begin();
                    Tile_index id0 = it->id();
                    std::size_t count0 = inbox[id0].size();
                    for(++it; it != tc.end() && count0; ++it)
                    {
                        Tile_index id = it->id();
                        std::size_t count = inbox[id].size();
                        if(count0 > count) {
                            count0 = count;
                            id0 = id;
                        }
                    }
                    tc.unload(id0);
                }
*/

    /// unloads a tile from memory, automatically saving it.
    /// returns true after the loaded tile id is successfully saved and unloaded from memory.
    /// @todo attention à la perennité des handles (tile is possibly unloaded), ou alors lock ou shared pointer.
    void unload(Tile& tile) {

        std::cout << "[" << std::setw(4) << std::to_string(tile.id()) << "] " << std::setfill('_');
        for(const Tile& t : *this) {
            if(t.locked           ) std::cout << "\x1b[1m" ; // blocked
            if(t.id() == tile.id()) std::cout << "\x1b[41m\x1b[1m" ; // bg red
            else if(!t.in_mem     ) std::cout << "\x1b[37m" ; // gray
            std::cout << std::to_string(t.id()) << "\x1B[0m" ; // reset
        }
        std::cout << std::setfill(' ') << " (" << number_of_tiles_mem_ << " in mem)" << std::endl;

        if (!tile.locked && tile.in_mem && serializer_.save(tile)) {
            tile.triangulation().finalize();
            tile.triangulation().clear();
            tile.in_mem = false;
            --number_of_tiles_mem_;
        }
    }

    /// load a tile to memory, automatically saving it.
    void prepare_load(Tile& tile) {
        if(tile.in_mem) return;

        std::cout << "[" << std::setw(4) << std::to_string(tile.id()) << "] " << std::setfill('_');
        for(const Tile& t : *this) {
            if(t.locked           ) std::cout << "\x1b[1m" ; // blocked
            if(t.id() == tile.id()) std::cout << "\x1b[42m" ; // bg green
            else if(!t.in_mem     ) std::cout << "\x1b[37m" ; // gray
            std::cout << std::to_string(t.id()) << "\x1B[0m" ; // reset
        }
        std::cout << std::setfill(' ') << " (" << number_of_tiles_mem_ << " in mem)" << std::endl;

        // make room if necessary
        while(number_of_tiles_mem_ >= number_of_tiles_mem_max_) {
            // pick a loaded id at random and try to unload it
            std::size_t n = rand() % number_of_tiles_mem_;
            for(Tile& tile : *this) {
                if(tile.in_mem) {
                    if (n == 0) {
                        if (!tile.locked) unload(tile);
                        break;
                    }
                    --n;
                 }
            }
        }
        // reserve the mem slot in memory, so that it is not stealed by a concurrent thread
        ++number_of_tiles_mem_;
    }

    bool safe_load(Tile& tile) {
        if(tile.in_mem) return true;
        if (!serializer_.has_tile(tile.id()) || serializer_.load(tile)) {
            tile.in_mem = true;
            return true;
        } else {
            --number_of_tiles_mem_;
            return false;
        }
    }

    /// load a tile to memory.
    bool load(Tile& tile) {
        prepare_load(tile);
        return safe_load(tile);
    }


    const Serializer& serializer() const { return serializer_; }

private:
    Container tiles;
    Serializer serializer_;

    int dimension_;

    std::size_t number_of_tiles_mem_max_;
    std::size_t number_of_tiles_mem_;
};

}
}

#endif // CGAL_DDT_TILE_CONTAINER_H
