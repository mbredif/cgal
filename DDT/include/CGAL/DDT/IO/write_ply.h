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

inline void write_ply_header_begin(std::ostream& out)
{
    out << "ply" << std::endl;
    out << "format binary_little_endian 1.0" << std::endl;
    out << "comment creator: Mathieu Bredif" << std::endl;
}

inline void write_ply_header_end(std::ostream& out)
{
    out << "end_header" << std::endl;
}

template<typename TileContainer>
void write_ply_element_cell(const TileContainer& tc, std::ostream& out)
{
    int nc = tc.number_of_cells();
    out << "element face " << nc << std::endl;
    out << "property list uint8 int vertex_indices" << std::endl;
    out << "property uint8 tile" << std::endl;
    out << "property uint8 local" << std::endl;
}

template<typename TileContainer>
void write_ply_element_vert(const TileContainer& tc, std::ostream& out)
{
    int D = tc.maximal_dimension();
    int nv = tc.number_of_vertices();
    out << "element vertex " << nv << std::endl;
    out << "property float32 x" << std::endl;
    out << "property float32 y" << std::endl;
    if(D>2) out << "property float32 z" << std::endl;
    out << "property uint8 tile" << std::endl;
    out << "property uint8 id" << std::endl;
}

template<typename DelaunayTriangulationTile>
void write_ply_property_cell(const DelaunayTriangulationTile& dt, std::ostream& out)
{
    unsigned char N = (unsigned char)(dt.maximal_dimension()+1);
    unsigned char tid = dt.id();
    std::map<typename DelaunayTriangulationTile::Vertex_const_handle, int> dict;

    int id = 0;
    for(auto it = dt.vertices_begin(); it != dt.vertices_end(); ++it)
        if(!dt.vertex_is_infinite(it)) dict[it] = id++;

    for(auto cit = dt.cells_begin(); cit != dt.cells_end(); ++cit)
    {
        if(dt.cell_is_infinite(cit)) continue;
        unsigned char local = 0;
        out.write(reinterpret_cast<char *>(&N), sizeof(N));
        for(int i=0; i<N; ++i)
        {
            typename DelaunayTriangulationTile::Vertex_const_handle v = dt.vertex(cit, i);
            int id = dict[v];
            out.write(reinterpret_cast<char *>(&id), sizeof(id));
            local += dt.vertex_id(v) == tid;
        }
        out.write(reinterpret_cast<char *>(&tid), sizeof(tid));
        out.write(reinterpret_cast<char *>(&local), sizeof(local));

    }
}

template<typename DelaunayTriangulationTile>
void write_ply_property_vert(const DelaunayTriangulationTile& dt, std::ostream& out)
{
    int D = dt.maximal_dimension();
    unsigned char tid = dt.id();
    for(auto it = dt.vertices_begin(); it != dt.vertices_end(); ++it)
    {
        if(dt.vertex_is_infinite(it)) continue;
        unsigned char id = dt.vertex_id(it);
        for(int d=0; d<D; ++d)
        {
            float coord = float(dt.coord(dt.point(it),d));
            out.write(reinterpret_cast<char *>(&coord), sizeof(coord));
        }
        out.write(reinterpret_cast<char *>(&tid), sizeof(tid));
        out.write(reinterpret_cast<char *>(&id), sizeof(id));
    }
}

template<typename TileContainer>
void write_ply_cell(const TileContainer& tc, const std::string& filename)
{
    std::ofstream out(filename, std::ios::binary);
    write_ply_header_begin(out);
    write_ply_element_cell(tc, out);
    write_ply_header_end(out);
    for(const auto& tile : tc)
        write_ply_property_cell(tile.triangulation(), out);
    out.close();
}

template<typename TileContainer>
void write_ply_vert(const TileContainer& tc, const std::string& filename)
{
    std::ofstream out(filename, std::ios::binary);
    write_ply_header_begin(out);
    write_ply_element_vert(tc, out);
    write_ply_header_end(out);
    for(const auto& tile : tc)
        write_ply_property_vert(tile.triangulation(), out);
    out.close();
}

template<typename TileContainer>
void write_ply(const TileContainer& tc, const std::string& filename)
{
    std::ofstream out(filename, std::ios::binary);
    write_ply_header_begin(out);
    write_ply_element_vert(tc, out);
    write_ply_element_cell(tc, out);
    write_ply_header_end(out);
    for(const auto& tile : tc)
        write_ply_property_vert(tile.triangulation(), out);
    for(const auto& tile : tc)
        write_ply_property_cell(tile.triangulation(), out);
    out.close();
}

}
}

#endif // CGAL_DDT_WRITE_PLY_H
