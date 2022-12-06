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

// CSV tile writers

template<typename Tile>
void write_csv_vert(const std::string& filename, const Tile& tile)
{
    std::ofstream csv(filename+".csv");
    csv << "geom,id" << std::endl;
    int D = tile.maximal_dimension();
    for(auto vit = tile.vertices_begin(); vit != tile.vertices_end(); ++vit)
    {
        if(tile.vertex_is_infinite(vit)) continue;
        csv << "POINT( ";
        for(int d=0; d<D; ++d)
            csv << tile.point(vit)[d] << " ";
        csv << ")," << int(tile.vertex_id(vit)) << "\n";
    }
}

template<typename Tile>
void write_csv_facet(const std::string& filename, const Tile& tile)
{
    std::ofstream csv(filename+".csv");
    csv << "geom,id,local" << std::endl;
    int D = tile.maximal_dimension();
    for(typename Tile::Facet_const_iterator fit = tile.facets_begin(); fit != tile.facets_end(); ++fit)
    {
        if(tile.facet_is_infinite(fit)) continue;
        auto cit = tile.cell(fit);
        int idx = tile.index_of_covertex(fit);
        csv << "\"LINESTRING(";
        int local = 0;
        int j = 0;
        for(int i=0; i<=D; ++i)
        {
            if(i == idx) continue;
            auto v = tile.vertex(cit,i);
            local += tile.vertex_is_local(v);
            for(int d=0; d<D; ++d)
                csv << tile.point(v)[d] << " ";
            if (++j < D) csv << ",";
        }
        csv << ")\"," << int(tile.facet_id(fit)) << "," << local << "\n";
    }
}

template<typename Tile>
void write_csv_cell(const std::string& filename, const Tile& tile)
{
    std::ofstream csv(filename+".csv");
    csv << "geom,id,local" << std::endl;
    int D = tile.maximal_dimension();
    for(auto cit = tile.cells_begin(); cit != tile.cells_end(); ++cit)
    {
        if(tile.cell_is_infinite(cit)) continue;
        csv << "\"POLYGON((";
        int local = 0;
        for(int i=0; i<=D; ++i)
        {
            const typename Tile::Vertex_const_handle v = tile.vertex(cit,i);
            local += tile.vertex_is_local(v);
            for(int d=0; d<D; ++d)
                csv << tile.point(v)[d] << " ";
            csv << ",";
        }
        for(int d=0; d<D; ++d) // repeat first to close the polygon
            csv << tile.point(tile.vertex(cit, 0))[d] << " ";
        csv << "))\"," << int(tile.cell_id(cit)) << "," << local << "\n";
    }
}

template<typename BboxMap>
void write_csv_bboxes(const std::string& filename, const BboxMap& bboxes)
{
    std::ofstream csv(filename+".csv");
    csv << "geom,id" << std::endl;
    for(auto& b : bboxes)
    {
        size_t id = b.first;
        auto& bbox = b.second;
        csv << "\"POLYGON((";
        csv << bbox.min(0) << " "<< bbox.min(1) << ", ";
        csv << bbox.max(0) << " "<< bbox.min(1) << ", ";
        csv << bbox.max(0) << " "<< bbox.max(1) << ", ";
        csv << bbox.min(0) << " "<< bbox.max(1) << ", ";
        csv << bbox.min(0) << " "<< bbox.min(1);
        csv << "))\"," << id << "\n";
    }
}


template<typename Tile>
void write_csv_tin(const std::string& filename, const Tile& tile)
{
    std::ofstream csv(filename+".csv");
    csv << "geom,id" << std::endl;
    int D = tile.maximal_dimension();
    bool first = true;
    for(auto cit = tile.cells_begin(); cit != tile.cells_end(); ++cit)
    {
        if(tile.cell_is_infinite(cit)|| !tile.cell_is_main(cit)) continue;
        csv << (first ? "\"TIN (((" : ", ((");
        first = false;
        typename Tile::Vertex_const_handle v;
        for(int i=0; i<=D; ++i)
        {
            v = tile.vertex(cit,i);
            for(int d=0; d<D; ++d)
                csv << tile.point(v)[d] << " ";
            csv << ",";
        }
        v = tile.vertex(cit,0);
        for(int d=0; d<D; ++d) // repeat first to close the polygon
            csv << tile.point(v)[d] << " ";
        csv << "))";
    }
    if (!first) csv << ")\"," << int(tile.id()) << "\n";
 }

// VRT header writers

void write_vrt_header(const std::string& filename, const std::string& type)
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
    f <<    "<Field name=\"local\" type=\"Integer\"/>" << std::endl;
    f <<  "</OGRVRTLayer>" << std::endl;
    f <<"</OGRVRTDataSource>" << std::endl;
}


template<typename Iterator>
void write_vrt_header(const std::string& dirname, const std::string& type, const std::string& union_name, Iterator begin, Iterator end)
{
    std::ofstream f(dirname+".vrt");
    boost::filesystem::path path(dirname);
    std::string stem = path.stem().string();
    f <<"<OGRVRTDataSource>" << std::endl;
    f <<"<OGRVRTUnionLayer name=\""<< union_name << "\">" << std::endl;
    f <<"<SourceLayerFieldName>tile</SourceLayerFieldName>" << std::endl;
    for(Iterator it = begin; it != end; ++it) {
        std::string name = std::to_string(*it);
        f <<  "<OGRVRTLayer name=\"" << name <<  "\">" << std::endl;
        f <<    "<SrcDataSource relativeToVRT=\"1\">" << stem << "/" << name << ".csv</SrcDataSource>" << std::endl;
        f <<    "<SrcLayer>" << name <<  "</SrcLayer>" << std::endl;
        f <<    "<LayerSRS>IGNF:LAMB93</LayerSRS> " << std::endl;
        f <<    "<GeometryType>" << type << "</GeometryType>" << std::endl;
        f <<    "<GeometryField encoding=\"WKT\" field=\"geom\"/>" << std::endl;
        f <<    "<Field name=\"id\" type=\"Integer\"/>" << std::endl;
        f <<    "<Field name=\"local\" type=\"Integer\"/>" << std::endl;
        f <<  "</OGRVRTLayer>" << std::endl;
    }
    f <<"</OGRVRTUnionLayer>" << std::endl;
    f <<"</OGRVRTDataSource>" << std::endl;
}


// VRT+CSV tile writers

template<typename Tile>
void write_tile_vrt_verts(const std::string& filename, const Tile& tile)
{
    write_vrt_header(filename, "wkbPoint");
    write_csv_vert(filename, tile);
}

template<typename Tile>
void write_tile_vrt_facets(const std::string& filename, const Tile& tile)
{
    write_vrt_header(filename, "wkbLineString");
    write_csv_facet(filename, tile);
}

template<typename Tile>
void write_tile_vrt_cells(const std::string& filename, const Tile& tile)
{
    write_vrt_header(filename, "wkbPolygon");
    write_csv_cell(filename, tile);
}

template<typename Tile>
void write_tile_vrt_tins(const std::string& filename, const Tile& tile)
{
    write_vrt_header(filename, "wkbTIN");
    write_csv_tin(filename, tile);
}

// VRT+CSV writers

template<typename TileContainer, typename Scheduler>
void write_vrt_verts(TileContainer& tc, Scheduler& sch, const std::string& dirname)
{
    boost::filesystem::path p(dirname);
    boost::filesystem::create_directories(p);
    sch.for_each(tc, [&dirname](const TileContainer&, typename TileContainer::Tile& tile) {
        std::string filename(dirname + "/" + std::to_string(tile.id()));
        write_tile_vrt_verts(filename, tile);
        return 1;
    });
    write_vrt_header(dirname, "wkbPoint", "vertices", tc.tile_ids_begin(), tc.tile_ids_end());
}

template<typename TileContainer, typename Scheduler>
void write_vrt_facets(TileContainer& tc, Scheduler& sch, const std::string& dirname)
{
    boost::filesystem::path p(dirname);
    boost::filesystem::create_directories(p);
    sch.for_each(tc, [&dirname](const TileContainer&, typename TileContainer::Tile& tile) {
        std::string filename(dirname + "/" + std::to_string(tile.id()));
        write_tile_vrt_facets(filename, tile);
        return 1;
    });
    write_vrt_header(dirname, "wkbLineString", "facets", tc.tile_ids_begin(), tc.tile_ids_end());
}

template<typename TileContainer, typename Scheduler>
void write_vrt_cells(TileContainer& tc, Scheduler& sch, const std::string& dirname)
{
    boost::filesystem::path p(dirname);
    boost::filesystem::create_directories(p);
    sch.for_each(tc, [&dirname](const TileContainer&, typename TileContainer::Tile& tile) {
        std::string filename(dirname + "/" + std::to_string(tile.id()));
        write_tile_vrt_cells(filename, tile);
        return 1;
    });
    write_vrt_header(dirname, "wkbPolygon", "cells", tc.tile_ids_begin(), tc.tile_ids_end());
}

template<typename TileContainer>
void write_vrt_bboxes(const TileContainer& tc, const std::string& filename)
{
    write_vrt_header(filename, "wkbPolygon");
    write_csv_bboxes(filename, tc.bboxes());
}

template<typename TileContainer, typename Scheduler>
void write_vrt_tins(TileContainer& tc, Scheduler& sch, const std::string& dirname)
{
    boost::filesystem::path p(dirname);
    boost::filesystem::create_directories(p);
    sch.for_each(tc, [&dirname](const TileContainer&, typename TileContainer::Tile& tile) {
        std::string filename(dirname + "/" + std::to_string(tile.id()));
        write_tile_vrt_tins(filename, tile);
        return 1;
    });
    write_vrt_header(dirname, "wkbTIN", "tins", tc.tile_ids_begin(), tc.tile_ids_end());
}

}
}

#endif // CGAL_DDT_WRITE_VRT_H
