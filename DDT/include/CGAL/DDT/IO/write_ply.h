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

#ifndef CGAL_DDT_WRITE_PLY_H
#define CGAL_DDT_WRITE_PLY_H

#include <string>
#include <fstream>
#include <map>

namespace CGAL {
namespace DDT {

namespace Impl {
template <typename T> struct ply_types { constexpr static const char *string = {}; };
template <> struct ply_types <char >{ constexpr static const char *string = "int8";  };
template <> struct ply_types <short>{ constexpr static const char *string = "int16"; };
template <> struct ply_types <int  >{ constexpr static const char *string = "int32"; };
template <> struct ply_types <long >{ constexpr static const char *string = "int64"; };
template <> struct ply_types <unsigned char >{ constexpr static const char *string = "uint8";  };
template <> struct ply_types <unsigned short>{ constexpr static const char *string = "uint16"; };
template <> struct ply_types <unsigned int  >{ constexpr static const char *string = "uint32"; };
template <> struct ply_types <unsigned long >{ constexpr static const char *string = "uint64"; };
}

inline void write_ply_header_begin(std::ostream& out)
{
    out << "ply" << std::endl;
    out << "format binary_little_endian 1.0" << std::endl;
    out << "comment creator: CGAL::DDT::write_ply" << std::endl;
}

inline void write_ply_header_end(std::ostream& out)
{
    out << "end_header" << std::endl;
}

template<typename DistributedTriangulation>
void write_ply_element_cell(const DistributedTriangulation& tri, std::ostream& out)
{
    const char *id_string = Impl::ply_types<typename DistributedTriangulation::Tile_index>::string;
    int nc = tri.number_of_cells();
    out << "element face " << nc << std::endl;
    out << "property list uint8 int vertex_indices" << std::endl;
    out << "property " << id_string << " tile" << std::endl;
    out << "property uint8 local" << std::endl;
}

template<typename DistributedTriangulation>
void write_ply_element_vert(const DistributedTriangulation& tri, std::ostream& out)
{
    const char *id_string = Impl::ply_types<typename DistributedTriangulation::Tile_index>::string;
    int D = tri.maximal_dimension();
    int nv = tri.number_of_vertices();
    out << "element vertex " << nv << std::endl;
    out << "property float32 x" << std::endl;
    out << "property float32 y" << std::endl;
    if(D>2) out << "property float32 z" << std::endl;
    out << "property " << id_string << " tile" << std::endl;
    out << "property " << id_string << " id" << std::endl;
}

template<typename TileTriangulation>
void write_ply_property_cell(const TileTriangulation& dt, std::ostream& out)
{
    typedef typename TileTriangulation::Tile_index Tile_index;
    typedef typename TileTriangulation::Vertex_index Vertex_index;
    typedef typename TileTriangulation::Cell_index Cell_index;
    unsigned char N = (unsigned char)(dt.maximal_dimension()+1);
    Tile_index tid = dt.id();
    std::map<Vertex_index, int> dict;

    int id = 0;
    for(Vertex_index v = dt.vertices_begin(); v != dt.vertices_end(); ++v)
        if(!dt.vertex_is_infinite(v)) dict[v] = id++;

    for(Cell_index c = dt.cells_begin(); c != dt.cells_end(); ++c)
    {
        if(dt.cell_is_infinite(c)) continue;
        unsigned char local = 0;
        out.write(reinterpret_cast<char *>(&N), sizeof(N));
        for(int i=0; i<N; ++i)
        {
            Vertex_index v = dt.vertex(c, i);
            int id = dict[v];
            out.write(reinterpret_cast<char *>(&id), sizeof(id));
            local += dt.vertex_id(v) == tid;
        }
        out.write(reinterpret_cast<char *>(&tid), sizeof(tid));
        out.write(reinterpret_cast<char *>(&local), sizeof(local));

    }
}

template<typename TileTriangulation>
void write_ply_property_vert(const TileTriangulation& dt, std::ostream& out)
{
    typedef typename TileTriangulation::Tile_index Tile_index;
    typedef typename TileTriangulation::Vertex_index Vertex_index;
    int D = dt.maximal_dimension();
    Tile_index tid = dt.id();
    for(Vertex_index v = dt.vertices_begin(); v != dt.vertices_end(); ++v)
    {
        if(dt.vertex_is_infinite(v)) continue;
        Tile_index id = dt.vertex_id(v);
        for(int d=0; d<D; ++d)
        {
            float coord = float(dt.approximate_cartesian_coordinate(v,d));
            out.write(reinterpret_cast<char *>(&coord), sizeof(coord));
        }
        out.write(reinterpret_cast<char *>(&tid), sizeof(tid));
        out.write(reinterpret_cast<char *>(&id), sizeof(id));
    }
}

template<typename DistributedTriangulation>
void write_ply_cell(const DistributedTriangulation& tri, const std::string& filename)
{
    std::ofstream out(filename, std::ios::binary);
    write_ply_header_begin(out);
    write_ply_element_cell(tri, out);
    write_ply_header_end(out);
    for(const auto& [id, tile] : tri.tiles)
        write_ply_property_cell(tile.value(), out);
    out.close();
}

template<typename DistributedTriangulation>
void write_ply_vert(const DistributedTriangulation& tri, const std::string& filename)
{
    std::ofstream out(filename, std::ios::binary);
    write_ply_header_begin(out);
    write_ply_element_vert(tri, out);
    write_ply_header_end(out);
    for(const auto& [id, tile] : tri.tiles)
        write_ply_property_vert(tile.value(), out);
    out.close();
}

template<typename DistributedTriangulation>
void write_ply(const DistributedTriangulation& tri, const std::string& filename)
{
    std::ofstream out(filename, std::ios::binary);
    write_ply_header_begin(out);
    write_ply_element_vert(tri, out);
    write_ply_element_cell(tri, out);
    write_ply_header_end(out);
    for(const auto& [id, tile] : tri.tiles)
        write_ply_property_vert(tile.value(), out);
    for(const auto& [id, tile]: tri.tiles)
        write_ply_property_cell(tile.value(), out);
    out.close();
}

}
}

#endif // CGAL_DDT_WRITE_PLY_H
