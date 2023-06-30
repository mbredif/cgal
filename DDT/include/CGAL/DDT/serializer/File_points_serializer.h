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
/// This serializer saves and loads the bounding box and point set of each tile to the filesystem.
/// It contains the iostream serialization of the bounding box and the point set of the tile triangulation.
/// The point set of each tile is sorted spatially before saving, so that the Delaunay triangulation could be recomputed efficiently when the tile is reloaded.
/// This trades off decreased disc usage and bandwith for increased computations.
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
  bool has_tile(TileIndex id) const
  {
    const std::string fname = filename(id);
    std::ifstream in(fname, std::ios::in | std::ios::binary);
    return in.is_open();
  }

  template<typename TileIndex, typename Bbox>
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
    typedef typename Tile::value_type  Tile_triangulation;
    typedef typename Tile_triangulation::Point Point;
    typedef typename Tile_triangulation::Vertex_index Vertex_index;
    typedef typename Tile_triangulation::Traits Traits;
    typedef typename Tile_triangulation::Tile_index Tile_index;

    const std::string fname = filename(tile.value().id());
    std::ifstream in(fname, std::ios::in | std::ios::binary);
    in >> tile.value().bbox();
    std::size_t count;
    in >> count;
    Vertex_index v;
    for(std::size_t i = 0; i < count; ++i) {
        Point p;
        Tile_index id;
        in >> p >> id;
        tile.value().bbox() += Traits::bbox(p);
        v = tile.value().insert(p,id,v).first;
    }
    if(!in.fail()) return true;
    tile.value().clear();
    return false;
  }

  template<typename Tile> bool save(const Tile& tile) const {
#ifdef CGAL_DEBUG_DDT
    ++nb_save;
#endif
    typedef typename Tile::value_type  Tile_triangulation;
    typedef typename Tile_triangulation::Point Point;
    typedef typename Tile_triangulation::Vertex_index Vertex_index;
    const std::string fname = filename(tile.value().id());
    std::ofstream out(fname, std::ios::out | std::ios::binary);
    out << std::setprecision(17) << tile.value().bbox() << "\n";

    const Tile_triangulation& triangulation = tile.value();
    out << triangulation.number_of_vertices() << "\n";
    std::vector<std::size_t> indices;
    std::vector<Point>  points;
    std::vector<Vertex_index> vertices;
    std::size_t index = 0;
    for(Vertex_index v = triangulation.vertices_begin(); v != triangulation.vertices_end(); ++v) {
      if (!triangulation.vertex_is_infinite(v)) {
        points.push_back(triangulation.point(v));
        vertices.push_back(v);
        indices.push_back(index++);
      }
    }
    triangulation.spatial_sort(indices, points);
    for (std::size_t index : indices)
      out << points[index] << " " << triangulation.vertex_id(vertices[index]) << "\n";
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
