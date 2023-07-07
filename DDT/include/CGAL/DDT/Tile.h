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
class Tile
{
public:
    Tile() :
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
    template<typename Value, typename Serializer>
    bool unload(Value& value, Serializer& serializer) {
        if (use_count==0 && in_mem && serializer.save(value)) {
            value.finalize();
            value.clear();
            in_mem = false;
            return true;
        }
        return false;
    }

    template<typename Key, typename Value, typename Serializer>
    bool load(Key key, Value& value, Serializer& serializer) {
        if(in_mem) return true;
        if (!serializer.has_tile(key) || serializer.load(value)) {
            in_mem = true;
            return true;
        }
        return false;
    }
};

}
}

#endif // CGAL_DDT_TILE_H
