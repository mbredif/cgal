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

#ifndef CGAL_DDT_WRITE_PVTU_H
#define CGAL_DDT_WRITE_PVTU_H

#undef BOOST_NO_CXX11_SCOPED_ENUMS

#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <CGAL/IO/io.h>
#include <CGAL/IO/VTK/VTK_writer.h>

namespace CGAL {
namespace DDT {


namespace Impl {
template <typename T> struct vtk_types {};
template <> struct vtk_types <char >{ constexpr static const char *string = "Int8";  };
template <> struct vtk_types <short>{ constexpr static const char *string = "Int16"; };
template <> struct vtk_types <int  >{ constexpr static const char *string = "Int32"; };
template <> struct vtk_types <long >{ constexpr static const char *string = "Int64"; };
template <> struct vtk_types <unsigned char >{ constexpr static const char *string = "UInt8";  };
template <> struct vtk_types <unsigned short>{ constexpr static const char *string = "UInt16"; };
template <> struct vtk_types <unsigned int  >{ constexpr static const char *string = "UInt32"; };
template <> struct vtk_types <unsigned long >{ constexpr static const char *string = "UInt64"; };
template <> struct vtk_types <float         >{ constexpr static const char *string = "Float32"; };
template <> struct vtk_types <double        >{ constexpr static const char *string = "Float64"; };
}

void write_vtk_header(std::ostream& os, const std::string& type, const std::string& version)
{
    os << "<?xml version=\"1.0\"?>\n"
       << "<VTKFile type=\"" << type << "\" version=\"" << version << "\"";
  #ifdef CGAL_LITTLE_ENDIAN
    os << " byte_order=\"LittleEndian\"";
  #else // CGAL_BIG_ENDIAN
    os << " byte_order=\"BigEndian\"";
  #endif

    switch(sizeof(std::size_t)) {
    case 4: os << " header_type=\"UInt32\""; break;
    case 8: os << " header_type=\"UInt64\""; break;
    default: CGAL_error_msg("Unknown size of std::size_t");
    }
    os << ">\n";
}

template<typename DistributedTriangulation>
bool write_pvtu(std::ostream& os, const DistributedTriangulation& tri)
{
    typedef std::remove_cv_t<typename DistributedTriangulation::Tile_index> Id;
    const char *tile_attr = Impl::vtk_types<Id>::string;
    const char *size_attr = Impl::vtk_types<std::size_t>::string;
    const char *type_attr = Impl::vtk_types<unsigned char>::string;
    const char *coor_attr = Impl::vtk_types<double>::string;
    write_vtk_header(os, "PUnstructuredGrid", "1.0");
    os <<" <PUnstructuredGrid GhostLevel=\"1\">\n";
    os <<"  <PPointData>\n";
    os <<"  </PPointData>\n";
    os <<"  <PPoints>\n";
    os <<"   <PDataArray type=\"" << coor_attr << "\" NumberOfComponents=\"3\" Name=\"Points\"/>\n";
    os <<"  </PPoints>\n";
    os <<"  <PCellData>\n";
    os <<"   <PDataArray type=\"" << tile_attr << "\" NumberOfComponents=\"1\" Name=\"tile\"/>\n";
    os <<"  </PCellData>\n";
    os <<"  <PCells>\n";
    os <<"   <PDataArray type=\"" << size_attr << "\" NumberOfComponents=\"1\" Name=\"connectivity\"/>\n";
    os <<"   <PDataArray type=\"" << size_attr << "\" NumberOfComponents=\"1\" Name=\"offsets\"/>\n";
    os <<"   <PDataArray type=\"" << type_attr << "\" NumberOfComponents=\"1\" Name=\"types\"/>\n";
    os <<"  </PCells>\n";
    for(const auto& [id, triangulation] : tri.tiles)
          os << "  <Piece Source=\"" << std::to_string(id) << ".vtu\"/>\n";
    os <<" </PUnstructuredGrid>\n";
    os <<"</VTKFile>\n";
    return true;
}

template<typename V>
void write_vtu_data_array_tag(std::ostream& os, const std::string& name,
                              bool binary, std::size_t size, std::size_t& offset, const V& v)
{
    const char *format = binary ? "appended" : "ascii";
    os << "    <DataArray Name=\"" << name
       << "\" format=\"" << format
       << "\" type=\"" << Impl::vtk_types<V>::string;
    if (binary) {  // if binary output, just write the xml tag
      os << "\" offset=\"" << offset << "\"/>\n";
      offset += sizeof(V) * size + sizeof(std::size_t);
      // data size + length of the encoded data (size_t)
    }
    else {
      std::string s = std::to_string(v) + " ";
      os << "\">\n";
      for( std::size_t i = 0; i < size; ++i)
        os << s;
      os << "\n    </DataArray>\n";
    }
}

template <class Tr, typename Vertex_index = typename Tr::Vertex_index>
void
write_vtu_cells_tag(std::ostream& os,
                const Tr & tr,
                std::size_t size_of_cells,
                std::map<Vertex_index, std::size_t> & V,
                bool binary,
                std::size_t& offset)
{
  typedef typename Tr::Cell_index Cell_index;
  const char *format = binary ? "appended" : "ascii";
  const char *type = Impl::vtk_types<std::size_t>::string;
  int dim = tr.maximal_dimension();

  // Write connectivity table
  os << "   <Cells>\n"
     << "    <DataArray Name=\"connectivity\" format =\"" << format << "\" type=\"" << type;

  if (binary) { // if binary output, just write the xml tag
    os << "\" offset=\"" << offset << "\"/>\n";
    offset += ((dim+1) * size_of_cells + 1) * sizeof(std::size_t);
    // 4 indices (size_t) per cell + length of the encoded data (size_t)
  }
  else {
    os << "\">\n";
    for( Cell_index c = tr.cells_begin() ;
         c != tr.cells_end() ;
         ++c )
    {
      if(tr.cell_is_infinite(c) || !tr.cell_is_main(c)) continue;
      for (int i=0; i<=dim; ++i) {
        os << V[tr.vertex(c, i)] << " ";
      }
    }
    os << "\n    </DataArray>\n";
  }

  std::size_t c;
  // Write offsets
  os << "    <DataArray Name=\"offsets\" format =\"" << format << "\" type=\"" << type;

  if (binary) {  // if binary output, just write the xml tag
    os << "\" offset=\"" << offset << "\"/>\n";
    offset += (size_of_cells + 1) * sizeof(std::size_t);
    // 1 offset (size_t) per cell + length of the encoded data (size_t)
  }
  else {
    os << "\">\n";
    std::size_t c_end = 4 * size_of_cells;
    for( c = 4; c <= c_end; c += 4)
       os << c << " ";
    os << "\n    </DataArray>\n";
  }

  const unsigned char VTK_TRIANGLE = 5;
  const unsigned char VTK_TETRA = 10;
  unsigned char vtk_type = dim == 3 ? VTK_TETRA : VTK_TRIANGLE;
  write_vtu_data_array_tag(os, "types", binary, size_of_cells, offset, vtk_type);
  os << "   </Cells>\n";
  os << "   <CellData Scalars=\"tile\">\n";
  write_vtu_data_array_tag(os, "tile", binary, size_of_cells, offset, tr.id());
  os << "   </CellData>\n";
}



template<class Tr, typename Vertex_index = typename Tr::Vertex_index>
void write_vtu_points_ascii(std::ostream& os,
                      const Tr & tr,
                      std::map<Vertex_index, std::size_t> & V)
{
  typedef typename Tr::Point_const_reference Point_const_reference;
  int dim = tr.maximal_dimension();
  std::size_t inum = 0;
  for( Vertex_index v = tr.vertices_begin(); v != tr.vertices_end(); ++v)
  {
    if (tr.vertex_is_infinite(v)) continue;
    V[v] = inum++;
    Point_const_reference p = tr.triangulation_point(v);
    os << approximate_cartesian_coordinate(p,0) << " ";
    os << approximate_cartesian_coordinate(p,1) << " ";
    if(dim == 3)
      os << approximate_cartesian_coordinate(p,2) << "\n";
    else
      os << 0.0 << "\n";
  }
}

template<class Tr, typename Vertex_index = typename Tr::Vertex_index>
void write_vtu_points_tag(std::ostream& os,
                      const Tr & tr,
                      std::size_t size_of_vertices,
                      std::map<typename Tr::Vertex_index, std::size_t> & V,
                      bool binary,
                      std::size_t& offset)
{
  typedef double FT;
  std::size_t inum = 0;
  const char *format = binary ? "appended" : "ascii";
  const char *type = (sizeof(FT) == 8) ? "Float64" : "Float32";

  os << "   <Points>\n"
     << "    <DataArray type =\"" << type << "\" NumberOfComponents=\"3\" format=\""
     << format;

  if (binary) {
    os << "\" offset=\"" << offset << "\"/>\n";
    offset += 3 * size_of_vertices * sizeof(FT) + sizeof(std::size_t);
    // 3 coords per points + length of the encoded data (size_t)
  } else {
    os << "\">\n";
    write_vtu_points_ascii(os, tr, V);
    os << "\n    </DataArray>\n";
  }
  os << "   </Points>\n";
  os << "   <PointData Scalars=\"tile\">\n";
  write_vtu_data_array_tag(os, "tile", binary, size_of_vertices, offset, tr.id());
  os << "   </PointData>\n";
}

template <class Triangulation, typename Vertex_index = typename Triangulation::Vertex_index>
void
write_vtu_points_binary(std::ostream& os,
            const Triangulation & tr,
            std::map<Vertex_index, std::size_t> & V)
{
  int dim = tr.maximal_dimension();
  typedef typename Triangulation::Tile_index            Tile_index;
  typedef typename Triangulation::Point_const_reference Point_const_reference;

  std::size_t inum = 0;
  std::vector<double> coordinates;
  std::vector<Tile_index> tiles(tr.number_of_vertices(),tr.id());
  coordinates.reserve(tr.number_of_vertices()*3);
  for( Vertex_index v = tr.vertices_begin(); v != tr.vertices_end(); ++v)
    {
      if (tr.vertex_is_infinite(v)) continue;
      V[v] = inum++;  // binary output => the map has not been filled yet
      Point_const_reference p = tr.triangulation_point(v);
      coordinates.push_back(approximate_cartesian_coordinate(p,0));
      coordinates.push_back(approximate_cartesian_coordinate(p,1));
      coordinates.push_back(dim == 3 ? approximate_cartesian_coordinate(p,2) : 0.0);
    }
  CGAL::IO::internal::write_vector<double>(os,coordinates);
  CGAL::IO::internal::write_vector<Tile_index>(os,tiles);
}

template <class Triangulation, typename Vertex_index = typename Triangulation::Vertex_index>
void
write_vtu_cells_binary(std::ostream& os,
            const Triangulation & tr,
            std::map<Vertex_index, std::size_t> & V)
{
  typedef typename Triangulation::Cell_index Cell_index;
  typedef typename Triangulation::Tile_index Tile_index;
  std::vector<std::size_t> connectivity_table;
  std::vector<std::size_t> offsets;
  const unsigned char VTK_TRIANGLE = 5;
  const unsigned char VTK_TETRA = 10;
  int dim = tr.maximal_dimension();
  unsigned char vtk_type = dim == 3 ? VTK_TETRA : VTK_TRIANGLE;
  std::vector<unsigned char> cell_type(tr.number_of_main_finite_cells(), vtk_type);
  std::vector<Tile_index> tiles(tr.number_of_main_finite_cells(),tr.id());
  connectivity_table.reserve(tr.number_of_vertices());
  offsets.reserve(tr.number_of_vertices());
  std::size_t off = 0;
  for( Cell_index c = tr.cells_begin() ; c != tr.cells_end() ; ++c )
    {
      if(tr.cell_is_infinite(c) || !tr.cell_is_main(c)) continue;
      off += (dim+1);
      offsets.push_back(off);
      for (int i=0; i<=dim; ++i)
        connectivity_table.push_back(V[tr.vertex(c, i)]);
    }

  CGAL::IO::internal::write_vector<std::size_t>(os,connectivity_table);
  CGAL::IO::internal::write_vector<std::size_t>(os,offsets);
  CGAL::IO::internal::write_vector<unsigned char>(os,cell_type);
  CGAL::IO::internal::write_vector<Tile_index>(os,tiles);
}


// VTU tile writers

template <class TileTriangulation>
void write_vtu_tile(std::ostream& os,
                    const TileTriangulation& tri,
                    bool binary = true)
{
  typedef typename TileTriangulation::Vertex_index Vertex_index;
  const std::size_t number_of_vertices = tri.number_of_vertices();
  const std::size_t number_of_cells = tri.number_of_main_finite_cells();
  std::map<Vertex_index, std::size_t> V;
  std::size_t offset = 0;

  if (!binary) os << std::setprecision(17);
  write_vtk_header(os, "UnstructuredGrid", "0.1");
  os << " <UnstructuredGrid>\n"
     << "  <Piece NumberOfPoints=\"" << number_of_vertices << "\" NumberOfCells=\"" << number_of_cells << "\">\n";
  write_vtu_points_tag(os, tri, number_of_vertices, V, binary, offset); // fills V if the mode is ASCII
  write_vtu_cells_tag (os, tri, number_of_cells   , V, binary, offset);
  os << "  </Piece>\n"
     << " </UnstructuredGrid>\n";
  if (binary) {
    os << "<AppendedData encoding=\"raw\">\n_";
    write_vtu_points_binary(os, tri, V); // fills V if the mode is BINARY
    write_vtu_cells_binary(os, tri, V);
    os << "</AppendedData>\n";
  }
  os << "</VTKFile>\n";
}

}
}

#endif // CGAL_DDT_WRITE_PVTU_H
