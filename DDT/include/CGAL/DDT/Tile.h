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

#include <CGAL/DDT/Tile_triangulation.h>

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTClasses
/// \tparam T is a model of the Triangulation concept
/// The Tile stores a local triangulation.
template<class Triangulation, class TileIndexProperty>
class Tile
{
public:
    typedef Triangulation_traits<Triangulation>       Traits;
    typedef typename TileIndexProperty::value_type    Tile_index;
    typedef typename Traits::Bbox                     Bbox;
    typedef CGAL::DDT::Tile_triangulation<Triangulation, TileIndexProperty>          Tile_triangulation;

    Tile(Tile_index id, int dimension) :
        triangulation_(id, dimension),
        bbox_(Traits::bbox(dimension)),
        in_mem(false),
        locked(false)
    {
    }

    const Bbox& bbox() const { return bbox_; }
    Bbox& bbox() { return bbox_; }

    const Tile_triangulation& triangulation() const { return triangulation_; }
    Tile_triangulation& triangulation() { return triangulation_; }

    /// lock the tile for exclusive use (no unloading, no concurrent processing)
    bool locked;
    /// is the triangulation in memory ?
    bool in_mem;

    bool is_valid(bool verbose = false, int level = 0) const
    {
        return triangulation_.is_valid(verbose, level);
    }

private:
    Tile_triangulation triangulation_;
    Bbox bbox_;
};

}
}

#endif // CGAL_DDT_TILE_H
