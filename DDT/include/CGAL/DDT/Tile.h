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
template<class Value>
class Tile
{
public:
    typedef Value value_type;

    template<typename... Args>
    Tile(Args&&... args) :
        value_(std::forward<Args>(args)...),
        in_mem(false),
        use_count(0)
    {
    }

    value_type& value() { return value_; }
    const value_type& value() const { return value_; }
    operator value_type&() { return value_; }
    operator const value_type&() const { return value_; }

    /// lock the tile for exclusive use (no unloading, no concurrent processing)
    mutable int use_count;
    /// is the triangulation in memory ?
    mutable bool in_mem;

    /// unloads a tile from memory, automatically saving it.
    /// returns true after the loaded tile id is successfully saved and unloaded from memory.
    /// @todo attention à la perennité des handles (tile is possibly unloaded), ou alors lock ou shared pointer.
    template<typename Serializer>
    bool unload(Serializer& serializer) const {
        if (use_count==0 && in_mem && serializer.save(value_)) {
            value_.finalize();
            value_.clear();
            in_mem = false;
            return true;
        }
        return false;
    }

    template<typename Key, typename Serializer>
    bool load(Key key, Serializer& serializer) const {
        if(in_mem) return true;
        if (!serializer.has_tile(key) || serializer.load(value_)) {
            in_mem = true;
            return true;
        }
        return false;
    }

private:
    mutable value_type value_;
};

}
}

#endif // CGAL_DDT_TILE_H
