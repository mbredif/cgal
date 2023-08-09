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
#include <CGAL/DDT/iterator/Tile_iterator.h>

#include <map>
#include <iomanip>
#include <limits>

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTClasses
/// Tile Container
template<typename AssociativeContainer,
         typename Serializer_ >
class Tile_container
{
public:
    typedef typename AssociativeContainer::key_type    key_type;
    typedef typename AssociativeContainer::mapped_type mapped_type;
    typedef Serializer_                                Serializer;

    typedef AssociativeContainer                       ValueContainer;
    typedef typename ValueContainer::iterator          value_iterator;
    typedef typename ValueContainer::const_iterator    value_const_iterator;

    typedef CGAL::DDT::Usage<value_iterator>           Usage;
    typedef std::map<key_type, Usage>                  UsageContainer;
    typedef typename UsageContainer::iterator          usage_iterator;
    typedef typename UsageContainer::const_iterator    usage_const_iterator;

    typedef CGAL::DDT::Tile_iterator<Tile_container, usage_iterator>  iterator;
    typedef CGAL::DDT::Tile_iterator<const Tile_container, usage_iterator>  const_iterator;

    Tile_container(std::size_t number_of_values_mem_max = 0, const Serializer& serializer = Serializer()) :
        values(),
        serializer_(serializer),
        number_of_values_mem_(0),
        number_of_values_mem_max_(number_of_values_mem_max)
    {
        if (number_of_values_mem_max_ == 0) number_of_values_mem_max_ = std::numeric_limits<std::size_t>::max();
    }

    inline std::size_t number_of_values_mem_max() const { return number_of_values_mem_max_; }
    inline std::size_t number_of_values_mem() const { return number_of_values_mem_; }

    bool empty() const { return values.empty(); }
    const_iterator cbegin () const { return {this, usages.begin()}; }
    const_iterator cend   () const { return {this, usages.end()}; }
    const_iterator begin  () const { return {this, usages.begin()}; }
    const_iterator end    () const { return {this, usages.end()}; }
    const_iterator find(key_type key) const { return {this, usages.find(key)}; }
    iterator begin  () { return {this, usages.begin()}; }
    iterator end    () { return {this, usages.end()}; }
    iterator find(key_type key) { return {this, usages.find(key)}; }


    value_iterator values_end    () { return values.end   (); }
    value_const_iterator values_end () const { return values.end   (); }

    usage_iterator usages_end    () { return usages.end   (); }
    usage_const_iterator usages_end () const { return usages.end   (); }

    template< class... Args >
    std::pair<iterator,bool> try_emplace(key_type key, Args&&... args) {
        auto val = values.try_emplace(key, std::forward<Args>(args)...);
        auto use = usages.try_emplace(key, val.first);
        return {{this, use.first}, use.second};
    }


    /*
     *             typename TileContainer::Tile_const_iterator tile = tc.find(*it);
            if(tile == tc.end()) {
                while(tc.number_of_values_mem_ >= tc.number_of_values_mem_max_) {
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
        for(const auto& [k, u] : usages) {
            if(u.use_count      ) out << "\x1b[1m\x1b[4m"  ; // bold, underscore
            if(u.in_mem         ) out << "\x1b[44m\x1b[37m"; // bg blue, fg white
            if(k == green_key   ) out << "\x1b[42m\x1b[37m"; // bg green, fg white
            else if(k == red_key) out << "\x1b[41m\x1b[37m"; // bg red, fg white
            out << std::to_string(k) << "\x1B[0m" ; // reset
        }
        out << std::setfill(' ') << " (" << number_of_values_mem_ << " in mem)";
        return out;
    }

    /// load a tile to memory, automatically saving it.
    bool prepare_load(Usage& usage) const {
        key_type key = usage->first;
        if(usage.in_mem) {
            write(std::cout << std::endl << "in mem ", key, key);
            return true;
        }

        if (number_of_values_mem_ < number_of_values_mem_max_) {
            ++number_of_values_mem_;
            write(std::cout << std::endl << "       ", key, key);
            return true;
        }

        // make room
        while(number_of_values_mem_ >= number_of_values_mem_max_) {
            // pick a loaded key at random and try to unload it
            std::size_t n = rand() % number_of_values_mem_;
            for(auto& [k, u] : usages) {
                if(u.in_mem) {
                    if (n == 0) {
                        if (u.use_count==0 && u.unload(serializer_)) {
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
    ValueContainer values;
    mutable UsageContainer usages;
    Serializer serializer_;

    std::size_t number_of_values_mem_max_;
    mutable std::size_t number_of_values_mem_;

};

}
}

#endif // CGAL_DDT_TILE_CONTAINER_H
