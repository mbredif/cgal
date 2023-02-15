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
#include <CGAL/DDT/Tile.h>

namespace CGAL {
namespace DDT {

template <class Traits>
struct File_serializer
{
  typedef CGAL::DDT::Tile<Traits>           Tile;
  typedef typename Traits::Tile_index       Tile_index;
  typedef typename Traits::Bbox             Bbox;
  typedef typename Tile::Tile_triangulation Tile_triangulation;

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

  bool has_tile(Tile_index id) const
  {
    const std::string fname = filename(id);
    std::ifstream in(fname, std::ios::in | std::ios::binary);
    return in.is_open();
  }

  bool load(Tile_index id, Bbox& bbox) const {
      const std::string fname = filename(id);
      std::ifstream in(fname, std::ios::in | std::ios::binary);
      in >> bbox;
      return !in.fail();
  }

  bool load(Tile& tile) const
  {
#ifdef CGAL_DEBUG_DDT
    ++nb_loads;
#endif
    const std::string fname = filename(tile.id());
    std::ifstream in(fname, std::ios::in | std::ios::binary);
    tile.triangulation().clear();
    in >> tile.bbox();
    in >> tile.triangulation();
    if(!in.fail()) return true;
    tile.triangulation().clear();
    return false;
  }

  bool save(const Tile& tile) const {
#ifdef CGAL_DEBUG_DDT
    ++nb_save;
#endif
    const std::string fname = filename(tile.id());
    std::ofstream out(fname, std::ios::out | std::ios::binary);
    out << std::setprecision(17) << tile.bbox() << "\n" << tile.triangulation();
    return !out.fail();
  }

  const std::string& prefix() const { return m_prefix; }

private:
  std::string filename(Tile_index i) const
  {
    return m_prefix+std::to_string(i)+".txt";
  }

  std::string m_prefix;
#ifdef CGAL_DEBUG_DDT
  mutable int nb_loads = 0;
  mutable int nb_save = 0;
#endif
};

template<typename Tile>
std::ostream& operator<<(std::ostream& out, const File_serializer<Tile>& serializer) {
    return out << "File_serializer(prefix=" << serializer.prefix() << ")";
}

}
}

#endif // CGAL_DDT_FILE_SERIALIZER_H
