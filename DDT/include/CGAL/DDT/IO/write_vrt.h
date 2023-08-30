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

#ifndef CGAL_DDT_WRITE_VRT_H
#define CGAL_DDT_WRITE_VRT_H

#include <boost/filesystem.hpp>
#undef BOOST_NO_CXX11_SCOPED_ENUMS

#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <set>

namespace CGAL {
namespace DDT {

// VRT header writers

template<typename DistributedTriangulation>
bool write_union_vrt_header(const std::string& dirname, const std::string& type, const std::string& union_name, DistributedTriangulation& tri, bool local)
{
    std::ofstream f(dirname+".vrt");
    boost::filesystem::path path(dirname);
    std::string stem = path.stem().string();
    f <<"<OGRVRTDataSource>" << std::endl;
    f <<"<OGRVRTUnionLayer name=\""<< union_name << "\">" << std::endl;
    f <<"<SourceLayerFieldName>tile</SourceLayerFieldName>" << std::endl;
    for(const auto& [id, tile] : tri.tiles) {
        std::string name = std::to_string(id);
        f <<  "<OGRVRTLayer name=\"" << name <<  "\">" << std::endl;
        f <<    "<SrcDataSource relativeToVRT=\"1\">" << stem << "/" << name << ".csv</SrcDataSource>" << std::endl;
        f <<    "<SrcLayer>" << name <<  "</SrcLayer>" << std::endl;
        f <<    "<LayerSRS>IGNF:LAMB93</LayerSRS> " << std::endl;
        f <<    "<GeometryType>" << type << "</GeometryType>" << std::endl;
        f <<    "<GeometryField encoding=\"WKT\" field=\"geom\"/>" << std::endl;
        f <<    "<Field name=\"id\" type=\"Integer\"/>" << std::endl;
        if (local) f <<    "<Field name=\"local\" type=\"Integer\"/>" << std::endl;
        f <<  "</OGRVRTLayer>" << std::endl;
    }
    f <<"</OGRVRTUnionLayer>" << std::endl;
    f <<"</OGRVRTDataSource>" << std::endl;
    return !f.fail();
}

bool write_vrt_header(const std::string& filename, const std::string& type, bool local)
{
    boost::filesystem::path path(filename);
    std::string stem = path.stem().string();
    std::ofstream f(filename+".vrt");
    f <<"<OGRVRTDataSource>" << std::endl;
    f <<  "<OGRVRTLayer name=\"" << stem <<  "\">" << std::endl;
    f <<    "<SrcDataSource relativeToVRT=\"1\">" << stem << ".csv</SrcDataSource>" << std::endl;
    f <<    "<SrcLayer>" << stem <<  "</SrcLayer>" << std::endl;
    f <<    "<LayerSRS>IGNF:LAMB93</LayerSRS> " << std::endl;
    f <<    "<GeometryType>" << type << "</GeometryType>" << std::endl;
    f <<    "<GeometryField encoding=\"WKT\" field=\"geom\"/>" << std::endl;
    f <<    "<Field name=\"id\" type=\"Integer\"/>" << std::endl;
    if (local) f <<    "<Field name=\"local\" type=\"Integer\"/>" << std::endl;
    f <<  "</OGRVRTLayer>" << std::endl;
    f <<"</OGRVRTDataSource>" << std::endl;
    return !f.fail();
}

// CSV tile writers

template<typename TileTriangulation>
bool write_csv_vert(const std::string& filename, const TileTriangulation& triangulation)
{
    typedef typename TileTriangulation::Vertex_index Vertex_index;
    std::ofstream csv(filename+".csv");
    csv << "geom,id" << std::endl;
    int D = triangulation.maximal_dimension();
    for(Vertex_index v = triangulation.vertices_begin(); v != triangulation.vertices_end(); ++v)
    {
        if(triangulation.vertex_is_infinite(v)) continue;
        csv << "POINT( ";
        for(int d=0; d<D; ++d)
            csv << triangulation.approximate_cartesian_coordinate(v,d) << " ";
        csv << ")," << std::to_string(triangulation.vertex_id(v)) << "\n";
    }
    return !csv.fail();
}

template<typename TileTriangulation>
bool write_csv_facet(const std::string& filename, const TileTriangulation& triangulation)
{
    typedef typename TileTriangulation::Vertex_index Vertex_index;
    typedef typename TileTriangulation::Facet_index  Facet_index;
    typedef typename TileTriangulation::Cell_index   Cell_index;
    std::ofstream csv(filename+".csv");
    csv << "geom,id,local" << std::endl;
    int D = triangulation.maximal_dimension();
    for(Facet_index f = triangulation.facets_begin(); f != triangulation.facets_end(); ++f)
    {
        if(triangulation.facet_is_infinite(f)) continue;
        Cell_index c = triangulation.cell(f);
        int idx = triangulation.index_of_covertex(f);
        csv << "\"LINESTRING(";
        int local = 0;
        int j = 0;
        for(int i=0; i<=D; ++i)
        {
            if(i == idx) continue;
            Vertex_index v = triangulation.vertex(c,i);
            local += triangulation.vertex_is_local(v);
            for(int d=0; d<D; ++d)
                csv << triangulation.approximate_cartesian_coordinate(v, d) << " ";
            if (++j < D) csv << ",";
        }
        csv << ")\"," << std::to_string(triangulation.facet_id(f)) << "," << local << "\n";
    }
    return !csv.fail();
}

template<typename TileTriangulation>
bool write_csv_cell(const std::string& filename, const TileTriangulation& triangulation)
{
    typedef typename TileTriangulation::Vertex_index Vertex_index;
    typedef typename TileTriangulation::Cell_index   Cell_index;
    std::ofstream csv(filename+".csv");
    csv << "geom,id,local" << std::endl;
    int D = triangulation.maximal_dimension();
    for(Cell_index c = triangulation.cells_begin(); c != triangulation.cells_end(); ++c)
    {
        if(triangulation.cell_is_infinite(c)) continue;
        csv << "\"POLYGON((";
        int local = 0;
        for(int i=0; i<=D; ++i)
        {
            Vertex_index v = triangulation.vertex(c,i);
            local += triangulation.vertex_is_local(v);
            for(int d=0; d<D; ++d)
                csv << triangulation.approximate_cartesian_coordinate(v, d) << " ";
            csv << ",";
        }
        for(int d=0; d<D; ++d) // repeat first to close the polygon
            csv << triangulation.approximate_cartesian_coordinate(triangulation.vertex(c, 0), d) << " ";
        csv << "))\"," << std::to_string(triangulation.cell_id(c)) << "," << local << "\n";
    }
    return !csv.fail();
}

template<typename TileTriangulation>
bool write_csv_tin(const std::string& filename, const TileTriangulation& triangulation)
{
    typedef typename TileTriangulation::Vertex_index Vertex_index;
    typedef typename TileTriangulation::Cell_index   Cell_index;
    std::ofstream csv(filename+".csv");
    csv << "geom,id" << std::endl;
    int D = triangulation.maximal_dimension();
    bool first = true;
    for(Cell_index c = triangulation.cells_begin(); c != triangulation.cells_end(); ++c)
    {
        if(triangulation.cell_is_infinite(c)|| !triangulation.cell_is_main(c)) continue;
        csv << (first ? "\"TIN (((" : ", ((");
        first = false;
        Vertex_index v;
        for(int i=0; i<=D; ++i)
        {
            v = triangulation.vertex(c,i);
            for(int d=0; d<D; ++d)
                csv << triangulation.approximate_cartesian_coordinate(v, d) << " ";
            csv << ",";
        }
        v = triangulation.vertex(c,0);
        for(int d=0; d<D; ++d) // repeat first to close the polygon
            csv << triangulation.approximate_cartesian_coordinate(v,d) << " ";
        csv << "))";
    }
    if (!first) csv << ")\"," << std::to_string(triangulation.id()) << "\n";
    return !csv.fail();
}

// VRT+CSV tile writers

template<typename TileTriangulation>
bool write_tile_vrt_verts(const std::string& filename, const TileTriangulation& triangulation)
{
    return write_vrt_header(filename, "wkbPoint", false) && write_csv_vert(filename, triangulation);
}

template<typename TileTriangulation>
bool write_tile_vrt_facets(const std::string& filename, const TileTriangulation& triangulation)
{
    return write_vrt_header(filename, "wkbLineString", true) && write_csv_facet(filename, triangulation);
}

template<typename TileTriangulation>
bool write_tile_vrt_cells(const std::string& filename, const TileTriangulation& triangulation)
{
    return write_vrt_header(filename, "wkbPolygon", true) && write_csv_cell(filename, triangulation);
}

template<typename TileTriangulation>
bool write_tile_vrt_tins(const std::string& filename, const TileTriangulation& triangulation)
{
    return write_vrt_header(filename, "wkbTIN", false) && write_csv_tin(filename, triangulation);
}

// VRT+CSV writers

template<typename DistributedTriangulation, typename Scheduler>
void write_vrt_verts(DistributedTriangulation& tri, Scheduler& sch, const std::string& dirname)
{
    typedef typename DistributedTriangulation::Tile_triangulation Tile_triangulation;
    typedef typename DistributedTriangulation::Tile_index         Tile_index;
    boost::filesystem::path p(dirname);
    boost::filesystem::create_directories(p);
    sch.for_each(tri.tiles, 0, std::plus<>(), [&dirname](Tile_index id, const Tile_triangulation& triangulation) {
        std::string filename(dirname + "/" + std::to_string(id));
        write_tile_vrt_verts(filename, triangulation);
        return 1;
    });
    write_vrt_header(dirname, "wkbPoint", "vertices", tri, false);
}

template<typename DistributedTriangulation, typename Scheduler>
void write_vrt_facets(DistributedTriangulation& tri, Scheduler& sch, const std::string& dirname)
{
    typedef typename DistributedTriangulation::Tile_triangulation Tile_triangulation;
    typedef typename DistributedTriangulation::Tile_index         Tile_index;
    boost::filesystem::path p(dirname);
    boost::filesystem::create_directories(p);
    sch.for_each(tri.tiles, 0, std::plus<>(), [&dirname](Tile_index id, const Tile_triangulation& triangulation) {
        std::string filename(dirname + "/" + std::to_string(id));
        write_tile_vrt_facets(filename, triangulation);
        return 1;
    });
    write_vrt_header(dirname, "wkbLineString", "facets", tri, true);
}

template<typename DistributedTriangulation, typename Scheduler>
void write_vrt_cells(DistributedTriangulation& tri, Scheduler& sch, const std::string& dirname)
{
    typedef typename DistributedTriangulation::Tile_triangulation Tile_triangulation;
    typedef typename DistributedTriangulation::Tile_index         Tile_index;
    boost::filesystem::path p(dirname);
    boost::filesystem::create_directories(p);
    sch.for_each(tri.tiles, 0, std::plus<>(), [&dirname](Tile_index id, const Tile_triangulation& triangulation) {
        std::string filename(dirname + "/" + std::to_string(id));
        write_tile_vrt_cells(filename, triangulation);
        return 1;
    });
    write_vrt_header(dirname, "wkbPolygon", "cells", tri, true);
}

template<typename DistributedTriangulation, typename Scheduler>
void write_vrt_tins(DistributedTriangulation& tri, Scheduler& sch, const std::string& dirname)
{
    typedef typename DistributedTriangulation::Tile_triangulation Tile_triangulation;
    typedef typename DistributedTriangulation::Tile_index         Tile_index;
    boost::filesystem::path p(dirname);
    boost::filesystem::create_directories(p);
    sch.for_each(tri.tiles, 0, std::plus<>(), [&dirname](Tile_index id, const Tile_triangulation& triangulation) {
        std::string filename(dirname + "/" + std::to_string(id));
        write_tile_vrt_tins(filename, triangulation);
        return 1;
    });
    write_vrt_header(dirname, "wkbTIN", "tins", tri, false);
}

}
}

#endif // CGAL_DDT_WRITE_VRT_H
