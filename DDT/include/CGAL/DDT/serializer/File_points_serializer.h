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

#ifndef CGAL_DDT_FILE_POINTS_SERIALIZER_H
#define CGAL_DDT_FILE_POINTS_SERIALIZER_H
#include <boost/filesystem.hpp>

namespace CGAL {
namespace DDT {

template <class Tile>
struct File_points_serializer
{
  typedef typename Tile::Id Id;

  File_points_serializer(const std::string& prefix) : m_prefix(prefix) {
      boost::filesystem::path p(prefix);
      boost::filesystem::path q(p.parent_path());
      if(p.has_parent_path() && !boost::filesystem::exists(q))
          boost::filesystem::create_directories(q);
  }
#ifdef CGAL_DEBUG_DDT
  ~File_points_serializer()
  {
    std::cout << *this << "\n";
    std::cout << "nb_loads " << nb_loads << "\n";
    std::cout << "nb_save " << nb_save << "\n";
  }
#endif

  bool has_tile(Id id) const
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
    typename Tile::Delaunay_triangulation dt;
    std::swap(dt, tile.triangulation());

    const std::string fname = filename(tile.id());
    std::ifstream in(fname, std::ios::in | std::ios::binary);
    size_t count;
    in >> count;
    typename Tile::Vertex_handle v;
    for(size_t i = 0; i < count; ++i) {
        typedef typename Tile::Point Point;
        Point p;
        Id id;
        in >> p >> id;
        v = tile.insert(p,id,v).first;
    }
    if(in.fail()) std::swap(dt, tile.triangulation());
    return !in.fail();
  }

  bool save(const Tile& tile) const {
#ifdef CGAL_DEBUG_DDT
    ++nb_save;
#endif
    typedef typename Tile::Point Point;
    typedef typename Tile::Vertex_const_iterator Vertex_const_iterator;
    const std::string fname = filename(tile.id());
    std::ofstream out(fname, std::ios::out | std::ios::binary);
    out << std::setprecision(17) << tile.number_of_vertices() << "\n";
    std::vector<size_t> indices;
    std::vector<Point>  points;
    std::vector<Vertex_const_iterator> vertices;
    size_t index = 0;
    for(Vertex_const_iterator v = tile.vertices_begin(); v != tile.vertices_end(); ++v) {
      if (!tile.vertex_is_infinite(v)) {
        points.push_back(tile.point(v));
        vertices.push_back(v);
        indices.push_back(index++);
      }
    }
    tile.spatial_sort(indices, points);
    for (size_t index : indices)
      out << points[index] << " " << tile.vertex_id(vertices[index]) << "\n";
    return !out.fail();
  }

  const std::string& prefix() const { return m_prefix; }

private:
  std::string filename(Id i) const
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
std::ostream& operator<<(std::ostream& out, const File_points_serializer<Tile>& serializer) {
    return out << "File_points_serializer(prefix=" << serializer.prefix() << ")";
}

}
}

#endif // CGAL_DDT_FILE_POINTS_SERIALIZER_H
