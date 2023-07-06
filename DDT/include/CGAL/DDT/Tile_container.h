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
#include <CGAL/DDT/iterator/Tile_iterator.h>

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
    typedef Serializer_                                Serializer;
    typedef CGAL::DDT::Tile<Value>                     Tile;

    typedef std::map<key_type, Tile>                   Container;
    typedef typename Container::iterator               raw_iterator;
    typedef typename Container::const_iterator         raw_const_iterator;

    typedef CGAL::DDT::Tile_iterator<Tile_container, raw_iterator>  iterator;
    typedef CGAL::DDT::Tile_iterator<const Tile_container, raw_const_iterator>  const_iterator;

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
    const_iterator cbegin () const { return {this, tiles.cbegin ()}; }
    const_iterator cend   () const { return {this, tiles.cend   ()}; }
    const_iterator begin  () const { return {this, tiles.begin ()}; }
    const_iterator end    () const { return {this, tiles.end   ()}; }
    const_iterator find(key_type key) const { return {this, tiles.find(key)}; }
    iterator begin  () { return {this, tiles.begin ()}; }
    iterator end    () { return {this, tiles.end   ()}; }
    iterator find(key_type key) { return {this, tiles.find(key)}; }


    raw_iterator raw_end    () { return tiles.end   (); }
    raw_const_iterator raw_end () const { return tiles.end   (); }

    template< class... Args >
    std::pair<iterator,bool> try_emplace(key_type key, Args&&... args) {
        auto res = tiles.try_emplace(key, std::forward<Args>(args)...);
        return {{this, res.first}, res.second};
    }


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

    std::ostream& write(std::ostream& out, key_type green_key, key_type red_key) const {
        for(const auto& [k, t] : tiles) {
            if(t.use_count      ) out << "\x1b[1m\x1b[4m"  ; // bold, underscore
            if(t.in_mem         ) out << "\x1b[44m\x1b[37m"; // bg blue, fg white
            if(k == green_key   ) out << "\x1b[42m\x1b[37m"; // bg green, fg white
            else if(k == red_key) out << "\x1b[41m\x1b[37m"; // bg red, fg white
            out << std::to_string(k) << "\x1B[0m" ; // reset
        }
        out << std::setfill(' ') << " (" << number_of_tiles_mem_ << " in mem)";
        return out;
    }

    /// load a tile to memory, automatically saving it.
    bool prepare_load(key_type key, const Tile& tile) const {
        if(tile.in_mem) {
            write(std::cout << std::endl << "in mem ", key, key);
            return true;
        }

        if (number_of_tiles_mem_ < number_of_tiles_mem_max_) {
            ++number_of_tiles_mem_;
            write(std::cout << std::endl << "       ", key, key);
            return true;
		}

        // make room
        while(number_of_tiles_mem_ >= number_of_tiles_mem_max_) {
		    // pick a loaded key at random and try to unload it
		    std::size_t n = rand() % number_of_tiles_mem_;
		    for(auto& [k, t] : tiles) {
		        if(t.in_mem) {
		            if (n == 0) {
						if (t.use_count==0 && t.unload(serializer_)) {
							write(std::cout << std::endl << "unload ", key, k);
							return true;
		                }
						break;
		            }
		            --n;
		         }
		    }
		}

		write(std::cout << std::endl << "failed ", key, key) << std::endl;
		return false;
	}

    const Serializer& serializer() const { return serializer_; }



private:
    Container tiles;
    Serializer serializer_;

    std::size_t number_of_tiles_mem_max_;
    mutable std::size_t number_of_tiles_mem_;
};

}
}

#endif // CGAL_DDT_TILE_CONTAINER_H
