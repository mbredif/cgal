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
    Tile(Args... args) :
        value_(args...),
        in_mem(false),
        locked(false)
    {
    }

    const value_type& value() const { return value_; }
    value_type& value() { return value_; }

    operator Value&() { return value_; }
    operator const Value&() const { return value_; }

    /// lock the tile for exclusive use (no unloading, no concurrent processing)
    bool locked;
    /// is the triangulation in memory ?
    bool in_mem;

private:
    value_type value_;
};

}
}

#endif // CGAL_DDT_TILE_H
