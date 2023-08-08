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

/// \ingroup PkgDDTSerializerClasses
/// This serializer saves and loads the point set of each tile to the filesystem.
/// It contains the iostream serialization of the point set of the tile triangulation.
/// The point set of each tile is sorted spatially before saving, so that the Delaunay triangulation could be recomputed efficiently when the tile is reloaded.
/// This trades off decreased disk usage and bandwith for increased computations.
/// \cgalModels Serializer
struct File_points_serializer
{

  /// Each tile is saved as the file "{prefix}{tile_index}.txt".
  File_points_serializer(const std::string& prefix = "") : m_prefix(prefix) {
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


  template <typename TileIndex>
  bool readable(TileIndex id) const
  {
    const std::string fname = filename(id);
    std::ifstream in(fname, std::ios::in | std::ios::binary);
    return in.is_open();
  }

  template<typename TileTriangulation> bool read(TileTriangulation& tri) const
  {
#ifdef CGAL_DEBUG_DDT
    ++nb_loads;
#endif
    typedef typename TileTriangulation::Point Point;
    typedef typename TileTriangulation::Vertex_index Vertex_index;
    typedef typename TileTriangulation::Traits Traits;
    typedef typename TileTriangulation::Tile_index Tile_index;

    const std::string fname = filename(tri.id());
    std::ifstream in(fname, std::ios::in | std::ios::binary);

    std::size_t count;
    Vertex_index v;
    for(std::size_t i = 0; i < count; ++i) {
        Point p;
        Tile_index id;
        in >> p >> id;
        v = tri.insert(p,id,v).first;
    }
    if(!in.fail()) return true;
    tri.clear();
    return false;
  }

  template<typename TileTriangulation> bool write(const TileTriangulation& tri) const {
#ifdef CGAL_DEBUG_DDT
    ++nb_save;
#endif
    typedef typename TileTriangulation::Point Point;
    typedef typename TileTriangulation::Vertex_index Vertex_index;
    const std::string fname = filename(tri.id());
    std::ofstream out(fname, std::ios::out | std::ios::binary);
    out << std::setprecision(17) << tri.number_of_vertices() << "\n";
    std::vector<std::size_t> indices;
    std::vector<Point>  points;
    std::vector<Vertex_index> vertices;
    std::size_t index = 0;
    for(Vertex_index v = tri.vertices_begin(); v != tri.vertices_end(); ++v) {
      if (!tri.vertex_is_infinite(v)) {
        points.push_back(tri.point(v));
        vertices.push_back(v);
        indices.push_back(index++);
      }
    }
    tri.spatial_sort(indices, points);
    for (std::size_t index : indices)
      out << points[index] << " " << tri.vertex_id(vertices[index]) << "\n";
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

std::ostream& operator<<(std::ostream& out, const File_points_serializer& serializer) {
    return out << "File_points_serializer(prefix=" << serializer.prefix() << ")";
}

}
}

#endif // CGAL_DDT_FILE_POINTS_SERIALIZER_H
