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

#ifndef CGAL_DDT_TILE_H
#define CGAL_DDT_TILE_H

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTClasses
/// \tparam T is a model of the Triangulation concept
/// The Tile stores a local triangulation.
template<typename PairIterator>
class Usage
{
public:
    using iterator_type = PairIterator;
    using value_type    = typename iterator_type::value_type;
    using pointer       = typename iterator_type::pointer;

    Usage(PairIterator iterator) :
        it(iterator),
        in_mem(false),
        use_count(0)
    {
    }

    /// lock the tile for exclusive use (no unloading, no concurrent processing)
    int use_count;
    /// is the triangulation in memory ?
    bool in_mem;

    /// unloads a tile from memory, automatically saving it.
    /// returns true after the loaded tile id is successfully saved and unloaded from memory.
    /// @todo attention à la perennité des handles (tile is possibly unloaded), ou alors lock ou shared pointer.
    template<typename Serializer>
    bool unload(Serializer& serializer) {
        if (use_count==0 && in_mem && serializer.save(it->second)) {
            it->second.finalize();
            it->second.clear();
            in_mem = false;
            return true;
        }
        return false;
    }

    template<typename Serializer>
    bool load(Serializer& serializer) {
        if(in_mem) return true;
        if (!serializer.has_tile(it->first) || serializer.load(it->second)) {
            in_mem = true;
            return true;
        }
        return false;
    }

    const value_type& operator*() const { return *it; }
    pointer operator->() const { return it.operator->(); }

private:
    iterator_type it;
};

}
}

#endif // CGAL_DDT_TILE_H
