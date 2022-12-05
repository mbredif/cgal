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

namespace CGAL {
namespace DDT {

template <class Tile>
struct File_serializer
{
  File_serializer(const std::string& prefix) : m_prefix(prefix) {
      boost::filesystem::path p(prefix);
      boost::filesystem::path q(p.parent_path());
      if(p.has_parent_path() && !boost::filesystem::exists(q))
          boost::filesystem::create_directories(q);
  }
#ifdef CGAL_DEBUG_DDT
  ~File_serializer()
  {
    std::cout << "nb_loads " << nb_loads << "\n";
    std::cout << "nb_save " << nb_save << "\n";
  }
#endif

  bool has_tile(typename Tile::Id id) const
  {
    const std::string fname = filename(id);
    std::ifstream in(fname, std::ios::in | std::ios::binary);
    return in.is_open();
  }

  bool load(Tile& tile) const
  {
#ifdef CGAL_DEBUG_DDT
    ++nb_loads;
#endif
    const std::string fname = filename(tile.id());
    std::ifstream in(fname, std::ios::in | std::ios::binary);
    in >> tile.triangulation();
    return !in.fail();
  }

  bool save(const Tile& tile) const {
#ifdef CGAL_DEBUG_DDT
    ++nb_save;
#endif
    const std::string fname = filename(tile.id());
    std::ofstream out(fname, std::ios::out | std::ios::binary);
    out << std::setprecision(17) << tile.triangulation();
    return !out.fail();
  }

private:
  std::string filename(typename Tile::Id i) const
  {
    return m_prefix+std::to_string(i)+".txt";
  }

  std::string m_prefix;
#ifdef CGAL_DEBUG_DDT
  mutable int nb_loads = 0;
  mutable int nb_save = 0;
#endif
};


}
}

#endif // CGAL_DDT_FILE_SERIALIZER_H
