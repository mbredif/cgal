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
template<class TileTriangulation>
class Tile
{
public:
    typedef TileTriangulation                       Tile_triangulation;
    typedef typename Tile_triangulation::Tile_index Tile_index;

    Tile(Tile_index id, int dimension) :
        triangulation_(id, dimension),
        in_mem(false),
        locked(false)
    {
    }

    const Tile_triangulation& triangulation() const { return triangulation_; }
    Tile_triangulation& triangulation() { return triangulation_; }

    /// lock the tile for exclusive use (no unloading, no concurrent processing)
    bool locked;
    /// is the triangulation in memory ?
    bool in_mem;

private:
    Tile_triangulation triangulation_;
};

}
}

#endif // CGAL_DDT_TILE_H
