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

namespace CGAL {
namespace DDT {

template<typename Tile>
struct No_serializer
{
  typedef typename Tile::Tile_index Tile_index;
  typedef typename Tile::Bbox Bbox;
  bool load(Tile_index id, Bbox& bbox) const { return false; }
  bool has_tile(Tile_index) const { return false; }
  bool load(Tile&) const { return false; }
  bool save(const Tile& ) const { return false;}
};

template<typename Tile>
std::ostream& operator<<(std::ostream& out, const No_serializer<Tile>& serializer) {
    return out << "No_serializer";
}

} }  // CGAL::DDT namespace

#endif // CGAL_DDT_NO__SERIALIZER_H
