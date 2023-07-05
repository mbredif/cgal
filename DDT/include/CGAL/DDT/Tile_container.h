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
#include <CGAL/DDT/Tile.h>

#include <map>
#include <iomanip>
#include <limits>

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTClasses
/// Tile Container
template<typename Key,
         typename Value,
         typename Serializer_ = No_serializer >
class Tile_container
{
public:
    typedef Key                                        key_type;
    typedef Value                                      mapped_type;
    typedef CGAL::DDT::Tile<Value>                     Tile;
    typedef Serializer_                                Serializer;

    typedef std::map<key_type, Tile>                   Container;
    typedef typename Container::iterator               iterator;
    typedef typename Container::const_iterator         const_iterator;
//    typedef Key_const_iterator<const_iterator>         key_const_iterator ;


    Tile_container(std::size_t number_of_tiles_mem_max = 0, const Serializer& serializer = Serializer()) :
        tiles(),
        serializer_(serializer),
        number_of_tiles_mem_(0),
        number_of_tiles_mem_max_(number_of_tiles_mem_max)
    {
        if (number_of_tiles_mem_max_ == 0) number_of_tiles_mem_max_ = std::numeric_limits<std::size_t>::max();
    }

    inline std::size_t number_of_tiles_mem_max() const { return number_of_tiles_mem_max_; }
    inline std::size_t number_of_tiles_mem() const { return number_of_tiles_mem_; }

    bool empty() const { return tiles.empty(); }
    const_iterator cbegin  () const { return tiles.begin (); }
    const_iterator cend    () const { return tiles.end   (); }
    const_iterator begin  () const { return tiles.begin (); }
    const_iterator end    () const { return tiles.end   (); }
    const_iterator find(key_type key) const { return tiles.find(key); }
    iterator begin  () { return tiles.begin (); }
    iterator end    () { return tiles.end   (); }
    iterator find(key_type key) { return tiles.find(key); }

    template< class... Args >
    std::pair<iterator,bool> try_emplace(key_type key, Args&&... args) {
        return tiles.try_emplace(key, std::forward<Args>(args)...);
    }



    // const Tile& at(key_type k) const { return tiles.at(k); }
    // Tile& at(key_type k) { return tiles.at(k); }

    /*
     *             typename TileContainer::Tile_const_iterator tile = tc.find(*it);
            if(tile == tc.end()) {
                while(tc.number_of_tiles_mem_ >= tc.number_of_tiles_mem_max_) {
                    auto it = tc.begin();
                    key_type k0 = it->id();
                    std::size_t count0 = inbox[id0].size();
                    for(++it; it != tc.end() && count0; ++it)
                    {
                        key_type k = it->id();
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
    void unload(key_type key, Tile& tile) {

        std::cout << "[" << std::setw(4) << std::to_string(key) << "] " << std::setfill('_');
        for(const auto& [k, t] : tiles) {
            if(t.locked      ) std::cout << "\x1b[1m" ; // blocked
            if(k == key      ) std::cout << "\x1b[41m\x1b[1m" ; // bg red
            else if(!t.in_mem) std::cout << "\x1b[37m" ; // gray
            std::cout << std::to_string(k) << "\x1B[0m" ; // reset
        }
        std::cout << std::setfill(' ') << " (" << number_of_tiles_mem_ << " in mem)" << std::endl;

        if (!tile.locked && tile.in_mem && serializer_.save(tile.value())) {
            tile.value().finalize();
            tile.value().clear();
            tile.in_mem = false;
            --number_of_tiles_mem_;
        }
    }

    /// load a tile to memory, automatically saving it.
    void prepare_load(key_type key, Tile& tile) {
        if(tile.in_mem) return;

        std::cout << "[" << std::setw(4) << std::to_string(key) << "] " << std::setfill('_');
        for(const auto& [k, t] : tiles) {
            if(t.locked      ) std::cout << "\x1b[1m" ; // blocked
            if(k == key      ) std::cout << "\x1b[42m" ; // bg green
            else if(!t.in_mem) std::cout << "\x1b[37m" ; // gray
            std::cout << std::to_string(k) << "\x1B[0m" ; // reset
        }
        std::cout << std::setfill(' ') << " (" << number_of_tiles_mem_ << " in mem)" << std::endl;

        // make room if necessary
        while(number_of_tiles_mem_ >= number_of_tiles_mem_max_) {
            // pick a loaded key at random and try to unload it
            std::size_t n = rand() % number_of_tiles_mem_;
            for(auto& [k, t] : tiles) {
                if(t.in_mem) {
                    if (n == 0) {
                        if (!t.locked) unload(k, t);
                        break;
                    }
                    --n;
                 }
            }
        }
        // reserve the mem slot in memory, so that it is not stealed by a concurrent thread
        ++number_of_tiles_mem_;
    }

    bool safe_load(key_type key, Tile& tile) {
        if(tile.in_mem) return true;
        if (!serializer_.has_tile(key) || serializer_.load(tile.value())) {
            tile.in_mem = true;
            return true;
        } else {
            --number_of_tiles_mem_;
            return false;
        }
    }

    /// load a tile to memory.
    bool load(key_type key, Tile& tile) {
        prepare_load(key, tile);
        return safe_load(key, tile);
    }

    const Serializer& serializer() const { return serializer_; }

private:
    Container tiles;
    Serializer serializer_;

    std::size_t number_of_tiles_mem_max_;
    std::size_t number_of_tiles_mem_;
};

}
}

#endif // CGAL_DDT_TILE_CONTAINER_H
