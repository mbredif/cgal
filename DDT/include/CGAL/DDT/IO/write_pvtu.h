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

#include <boost/filesystem.hpp>
#undef BOOST_NO_CXX11_SCOPED_ENUMS

#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <set>
#include <CGAL/IO/io.h>

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

template<typename Iterator>
void write_pvtu_file(const std::string& dirname, Iterator begin, Iterator end)
{
    typedef std::remove_cv_t<typename Iterator::value_type> Id;
    boost::filesystem::path path(dirname);
    std::string stem = path.stem().string();
    std::string tile_attr = Impl::vtk_types<Id>::string;
    std::string size_attr = Impl::vtk_types<std::size_t>::string;
    std::string type_attr = Impl::vtk_types<unsigned char>::string;
    std::string coor_attr = Impl::vtk_types<double>::string;
    std::ofstream os(dirname+".pvtu");
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
    for(Iterator it = begin; it != end; ++it)
          os << "  <Piece Source=\"" << stem << "/" << std::to_string(*it) << ".vtu\"/>\n";
    os <<" </PUnstructuredGrid>\n";
    os <<"</VTKFile>\n";
}

template<typename V>
void write_vtu_data_array_tag(std::ostream& os, const std::string& name,
                              bool binary, std::size_t size, std::size_t& offset, const V& v)
{
    std::string format = binary ? "appended" : "ascii";
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

template <class Tr>
void
write_vtu_cells_tag(std::ostream& os,
                const Tr & tr,
                std::size_t size_of_cells,
                std::map<typename Tr::Vertex_index, std::size_t> & V,
                bool binary,
                std::size_t& offset)
{
  typedef typename Tr::Cell_index Cell_index;
  std::string format = binary ? "appended" : "ascii";
  std::string type = Impl::vtk_types<std::size_t>::string;

  // Write connectivity table
  os << "   <Cells>\n"
     << "    <DataArray Name=\"connectivity\" format =\"" << format << "\" type=\"" << type;

  if (binary) { // if binary output, just write the xml tag
    os << "\" offset=\"" << offset << "\"/>\n";
    offset += (4 * size_of_cells + 1) * sizeof(std::size_t);
    // 4 indices (size_t) per cell + length of the encoded data (size_t)
  }
  else {
    os << "\">\n";
    for( Cell_index c = tr.cells_begin() ;
         c != tr.cells_end() ;
         ++c )
    {
      if(tr.cell_is_infinite(c) || !tr.cell_is_main(c)) continue;
      for (int i=0; i<4; i++) {
        os << V[tr.vertex(c, i)] << " ";
      }
    }
  }
  os << "\n    </DataArray>\n";

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

  const unsigned char VTK_TETRA = 10;
  write_vtu_data_array_tag(os, "types", binary, size_of_cells, offset, VTK_TETRA);
  os << "   </Cells>\n";
  os << "   <CellData Scalars=\"tile\">\n";
  write_vtu_data_array_tag(os, "tile", binary, size_of_cells, offset, tr.id());
  os << "   </CellData>\n";
}

template<class Tr>
write_vtu_points_tag(std::ostream& os,
                      const Tr & tr,
                      std::size_t size_of_vertices,
                      std::map<typename Tr::Vertex_index, std::size_t> & V,
                      bool binary,
                      std::size_t& offset,
                      std::size_t dim = 3)
{
  typedef typename Tr::Vertex_index Vertex_index;
  typedef typename Tr::Traits Traits;
  typedef typename Traits::Geom_traits Gt;
  typedef typename Gt::FT FT;

  std::size_t inum = 0;
  std::string format = binary ? "appended" : "ascii";
  std::string type = (sizeof(FT) == 8) ? "Float64" : "Float32";

  os << "   <Points>\n"
     << "    <DataArray type =\"" << type << "\" NumberOfComponents=\"3\" format=\""
     << format;

  if (binary) {
    os << "\" offset=\"" << offset << "\"/>\n";
    offset += 3 * size_of_vertices * sizeof(FT) + sizeof(std::size_t);
    // dim coords per points + length of the encoded data (size_t)
  }
  else {
    os << "\">\n";
    os << std::setprecision(17);
    for( Vertex_index v = tr.vertices_begin();
         v != tr.vertices_end();
         ++v)
    {
      if (tr.vertex_is_infinite(v)) continue;
      V[v] = inum++;
      os << tr.point(v)[0] << " ";
      os << tr.point(v)[1] << " ";
      if(dim == 3)
        os << tr.point(v)[2] << " ";
      else
        os << 0.0 << " ";
    }
    os << "\n    </DataArray>\n";
  }
  os << "   </Points>\n";
  os << "   <PointData Scalars=\"tile\">\n";
  write_vtu_data_array_tag(os, "tile", binary, size_of_vertices, offset, tr.id());
  os << "   </PointData>\n";
}

// VTU tile writers

template <class Tile>
void write_vtu_tile(std::ostream& os,
                    const Tile& tile,
                    bool binary = true)
{
  typedef typename Tile::Tile_triangulation Tr;
  typedef typename Tr::Vertex_index Vertex_index;
  const Tr& tr = tile.triangulation();
  const std::size_t number_of_vertices = tr.number_of_vertices();
  const std::size_t number_of_cells = tr.number_of_main_finite_cells();
  std::map<Vertex_index, std::size_t> V;
  std::size_t offset = 0;

  write_vtk_header(os, "UnstructuredGrid", "0.1");
  os << " <UnstructuredGrid>\n"
     << "  <Piece NumberOfPoints=\"" << number_of_vertices << "\" NumberOfCells=\"" << number_of_cells << "\">\n";
  write_vtu_points_tag(os, tr, number_of_vertices, V, binary, offset); // fills V if the mode is ASCII
  write_vtu_cells_tag (os, tr, number_of_cells   , V, binary, offset);
  os << "  </Piece>\n"
     << " </UnstructuredGrid>\n";
  if (binary) {
    os << "<AppendedData encoding=\"raw\">\n_";
    /* todo: binary mode
    write_vtu_points_binary(os, tr, V); // fills V if the mode is BINARY
    write_vtu_cells_binary(os, tr, V);
    */
    os << "</AppendedData>\n";
  }
  os << "</VTKFile>\n";
}

template<typename TileContainer, typename Scheduler>
void write_pvtu(TileContainer& tc, Scheduler& sch, const std::string& dirname,
                bool binary = true)
{
    boost::filesystem::path p(dirname);
    boost::filesystem::create_directories(p);
    sch.for_each(tc, [&dirname,&binary](typename TileContainer::Tile& tile) {
        std::string filename(dirname + "/" + std::to_string(tile.id()));
        std::ofstream os(filename+".vtu");
        write_vtu_tile(os, tile, binary);
        return 1;
    });
    write_pvtu_file(dirname, tc.ids_begin(), tc.ids_end());
}

}
}

#endif // CGAL_DDT_WRITE_PVTU_H
