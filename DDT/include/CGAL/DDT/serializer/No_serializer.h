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

#ifndef CGAL_DDT_NO__SERIALIZER_H
#define CGAL_DDT_NO__SERIALIZER_H

#include <CGAL/DDT/Tile.h>

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTSerializerClasses
/// This serializer is a fake serializer that fails all saving and loading requests.
/// It should be used to disable any serialization and perform all computations in memory.
/// That provides the fastest computation, provided enough memory is available.
/// \cgalModels Serializer
struct No_serializer
{
  template<typename Tile_index, typename Bbox> bool load(Tile_index id, Bbox& bbox) const { return false; }
  template<typename Tile_index> bool has_tile(Tile_index) const { return false; }
  template<typename Tile> bool load(Tile&) const { return false; }
  template<typename Tile> bool save(const Tile& ) const { return false;}
};

std::ostream& operator<<(std::ostream& out, const No_serializer& serializer) {
    return out << "No_serializer";
}

} }  // CGAL::DDT namespace

#endif // CGAL_DDT_NO__SERIALIZER_H
