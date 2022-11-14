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

#ifndef CGAL_DDT_FILE_SERIALIZER_H
#define CGAL_DDT_FILE_SERIALIZER_H

namespace CGAL {
namespace DDT {

template <class Tile>
struct File_serializer
{
  File_serializer(const std::string& prefix) : m_prefix(prefix) {}

  bool has_tile(typename Tile::Id id) const
  {
    const std::string fname = filename(id);
    std::ifstream in(fname, std::ios::in | std::ios::binary);
    return in.is_open();
  }

  Tile load(typename Tile::Id id) const
  {
    const std::string fname = filename(id);
    std::ifstream in(fname, std::ios::in | std::ios::binary);
    Tile tile(id);
    in >> tile.triangulation();
    return tile;
  }

  bool save(const Tile& tile) const {
    const std::string fname = filename(tile.id());
    std::ofstream out(fname, std::ios::out | std::ios::binary);
    out << std::setprecision(17) << tile.triangulation();
    return true;
  }

private:
  std::string filename(typename Tile::Id i) const
  {
    return m_prefix+std::to_string(i);
  }

  std::string m_prefix;
};


}
}

#endif // CGAL_DDT_FILE_SERIALIZER_H
