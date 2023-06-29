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

#include <boost/filesystem.hpp>
#include <iomanip>

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTSerializerClasses
/// This serializer saves and loads the bounding box and triangulation of each tile to the filesystem.
/// It contains the iostream serialization of the bounding box and the tile triangulation.
/// \cgalModels Serializer
struct File_serializer
{
  /// Each tile is saved as the file "{prefix}{tile_index}.txt".
  File_serializer(const std::string& prefix = "") : m_prefix(prefix) {
      boost::filesystem::path p(prefix);
      boost::filesystem::path q(p.parent_path());
      if(p.has_parent_path() && !boost::filesystem::exists(q))
          boost::filesystem::create_directories(q);
  }
#ifdef CGAL_DEBUG_DDT
  ~File_serializer()
  {
    std::cout << *this << "\n";
    std::cout << "nb_loads " << nb_loads << "\n";
    std::cout << "nb_save " << nb_save << "\n";
  }
#endif


  template <typename TileIndex>
  bool has_tile(TileIndex id) const
  {
    const std::string fname = filename(id);
    std::ifstream in(fname, std::ios::in | std::ios::binary);
    return in.is_open();
  }

  template <typename TileIndex, typename Bbox>
  bool load(TileIndex id, Bbox& bbox) const {
      const std::string fname = filename(id);
      std::ifstream in(fname, std::ios::in | std::ios::binary);
      in >> bbox;
      return !in.fail();
  }

  template<typename Tile> bool load(Tile& tile) const
  {
#ifdef CGAL_DEBUG_DDT
    ++nb_loads;
#endif
    const std::string fname = filename(tile.triangulation().id());
    std::ifstream in(fname, std::ios::in | std::ios::binary);
    tile.triangulation().clear();
    in >> tile.triangulation().bbox();
    in >> tile.triangulation();
    if(!in.fail()) return true;
    tile.triangulation().clear();
    return false;
  }

  template<typename Tile> bool save(const Tile& tile) const {
#ifdef CGAL_DEBUG_DDT
    ++nb_save;
#endif
    const std::string fname = filename(tile.triangulation().id());
    std::ofstream out(fname, std::ios::out | std::ios::binary);
    out << std::setprecision(17) << tile.triangulation().bbox() << "\n" << tile.triangulation();
    return !out.fail();
  }

  /// File system prefix
  const std::string& prefix() const { return m_prefix; }

private:
  template <typename TileIndex>
  std::string filename(TileIndex i) const
  {
    return m_prefix+std::to_string(i)+".txt";
  }

  std::string m_prefix;
#ifdef CGAL_DEBUG_DDT
  mutable int nb_loads = 0;
  mutable int nb_save = 0;
#endif
};

std::ostream& operator<<(std::ostream& out, const File_serializer& serializer) {
    return out << "File_serializer(prefix=" << serializer.prefix() << ")";
}

}
}

#endif // CGAL_DDT_FILE_SERIALIZER_H
