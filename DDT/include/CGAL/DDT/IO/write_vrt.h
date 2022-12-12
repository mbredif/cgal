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

template<typename TileTriangulation>
void write_csv_vert(const std::string& filename, const TileTriangulation& triangulation)
{
    std::ofstream csv(filename+".csv");
    csv << "geom,id" << std::endl;
    int D = triangulation.maximal_dimension();
    for(auto vit = triangulation.vertices_begin(); vit != triangulation.vertices_end(); ++vit)
    {
        if(triangulation.vertex_is_infinite(vit)) continue;
        csv << "POINT( ";
        for(int d=0; d<D; ++d)
            csv << triangulation.point(vit)[d] << " ";
        csv << ")," << int(triangulation.vertex_id(vit)) << "\n";
    }
}

template<typename TileTriangulation>
void write_csv_facet(const std::string& filename, const TileTriangulation& triangulation)
{
    std::ofstream csv(filename+".csv");
    csv << "geom,id,local" << std::endl;
    int D = triangulation.maximal_dimension();
    for(typename TileTriangulation::Facet_const_iterator fit = triangulation.facets_begin(); fit != triangulation.facets_end(); ++fit)
    {
        if(triangulation.facet_is_infinite(fit)) continue;
        auto cit = triangulation.cell(fit);
        int idx = triangulation.index_of_covertex(fit);
        csv << "\"LINESTRING(";
        int local = 0;
        int j = 0;
        for(int i=0; i<=D; ++i)
        {
            if(i == idx) continue;
            auto v = triangulation.vertex(cit,i);
            local += triangulation.vertex_is_local(v);
            for(int d=0; d<D; ++d)
                csv << triangulation.point(v)[d] << " ";
            if (++j < D) csv << ",";
        }
        csv << ")\"," << int(triangulation.facet_id(fit)) << "," << local << "\n";
    }
}

template<typename TileTriangulation>
void write_csv_cell(const std::string& filename, const TileTriangulation& triangulation)
{
    std::ofstream csv(filename+".csv");
    csv << "geom,id,local" << std::endl;
    int D = triangulation.maximal_dimension();
    for(auto cit = triangulation.cells_begin(); cit != triangulation.cells_end(); ++cit)
    {
        if(triangulation.cell_is_infinite(cit)) continue;
        csv << "\"POLYGON((";
        int local = 0;
        for(int i=0; i<=D; ++i)
        {
            const typename TileTriangulation::Vertex_const_handle v = triangulation.vertex(cit,i);
            local += triangulation.vertex_is_local(v);
            for(int d=0; d<D; ++d)
                csv << triangulation.point(v)[d] << " ";
            csv << ",";
        }
        for(int d=0; d<D; ++d) // repeat first to close the polygon
            csv << triangulation.point(triangulation.vertex(cit, 0))[d] << " ";
        csv << "))\"," << int(triangulation.cell_id(cit)) << "," << local << "\n";
    }
}

template<typename TileContainer>
void write_csv_bboxes(const std::string& filename, const TileContainer& tc)
{
    std::ofstream csv(filename+".csv");
    csv << "geom,id" << std::endl;
    for(auto& tile : tc)
    {
        typename TileContainer::Traits::Bbox const& bbox = tile.bbox();
        typename TileContainer::Traits::Id id = tile.id();
        csv << "\"POLYGON((";
        csv << bbox.min(0) << " "<< bbox.min(1) << ", ";
        csv << bbox.max(0) << " "<< bbox.min(1) << ", ";
        csv << bbox.max(0) << " "<< bbox.max(1) << ", ";
        csv << bbox.min(0) << " "<< bbox.max(1) << ", ";
        csv << bbox.min(0) << " "<< bbox.min(1);
        csv << "))\"," << std::to_string(id) << "\n";
    }
}

template<typename TileTriangulation>
void write_csv_tin(const std::string& filename, const TileTriangulation& triangulation)
{
    std::ofstream csv(filename+".csv");
    csv << "geom,id" << std::endl;
    int D = triangulation.maximal_dimension();
    bool first = true;
    for(auto cit = triangulation.cells_begin(); cit != triangulation.cells_end(); ++cit)
    {
        if(triangulation.cell_is_infinite(cit)|| !triangulation.cell_is_main(cit)) continue;
        csv << (first ? "\"TIN (((" : ", ((");
        first = false;
        typename TileTriangulation::Vertex_const_handle v;
        for(int i=0; i<=D; ++i)
        {
            v = triangulation.vertex(cit,i);
            for(int d=0; d<D; ++d)
                csv << triangulation.point(v)[d] << " ";
            csv << ",";
        }
        v = triangulation.vertex(cit,0);
        for(int d=0; d<D; ++d) // repeat first to close the polygon
            csv << triangulation.point(v)[d] << " ";
        csv << "))";
    }
    if (!first) csv << ")\"," << std::to_string(triangulation.id()) << "\n";
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

template<typename TileTriangulation>
void write_tile_vrt_verts(const std::string& filename, const TileTriangulation& triangulation)
{
    write_vrt_header(filename, "wkbPoint");
    write_csv_vert(filename, triangulation);
}

template<typename TileTriangulation>
void write_tile_vrt_facets(const std::string& filename, const TileTriangulation& triangulation)
{
    write_vrt_header(filename, "wkbLineString");
    write_csv_facet(filename, triangulation);
}

template<typename TileTriangulation>
void write_tile_vrt_cells(const std::string& filename, const TileTriangulation& triangulation)
{
    write_vrt_header(filename, "wkbPolygon");
    write_csv_cell(filename, triangulation);
}

template<typename TileTriangulation>
void write_tile_vrt_tins(const std::string& filename, const TileTriangulation& triangulation)
{
    write_vrt_header(filename, "wkbTIN");
    write_csv_tin(filename, triangulation);
}

// VRT+CSV writers

template<typename TileContainer, typename Scheduler>
void write_vrt_verts(TileContainer& tc, Scheduler& sch, const std::string& dirname)
{
    boost::filesystem::path p(dirname);
    boost::filesystem::create_directories(p);
    sch.for_each(tc, [&dirname](typename TileContainer::Tile& tile) {
        std::string filename(dirname + "/" + std::to_string(tile.id()));
        write_tile_vrt_verts(filename, tile.triangulation());
        return 1;
    });
    write_vrt_header(dirname, "wkbPoint", "vertices", tc.ids_begin(), tc.ids_end());
}

template<typename TileContainer, typename Scheduler>
void write_vrt_facets(TileContainer& tc, Scheduler& sch, const std::string& dirname)
{
    boost::filesystem::path p(dirname);
    boost::filesystem::create_directories(p);
    sch.for_each(tc, [&dirname](typename TileContainer::Tile& tile) {
        std::string filename(dirname + "/" + std::to_string(tile.id()));
        write_tile_vrt_facets(filename, tile.triangulation());
        return 1;
    });
    write_vrt_header(dirname, "wkbLineString", "facets", tc.ids_begin(), tc.ids_end());
}

template<typename TileContainer, typename Scheduler>
void write_vrt_cells(TileContainer& tc, Scheduler& sch, const std::string& dirname)
{
    boost::filesystem::path p(dirname);
    boost::filesystem::create_directories(p);
    sch.for_each(tc, [&dirname](typename TileContainer::Tile& tile) {
        std::string filename(dirname + "/" + std::to_string(tile.id()));
        write_tile_vrt_cells(filename, tile.triangulation());
        return 1;
    });
    write_vrt_header(dirname, "wkbPolygon", "cells", tc.ids_begin(), tc.ids_end());
}

template<typename TileContainer>
void write_vrt_bboxes(const TileContainer& tc, const std::string& filename)
{
    write_vrt_header(filename, "wkbPolygon");
    write_csv_bboxes(filename, tc);
}

template<typename TileContainer, typename Scheduler>
void write_vrt_tins(TileContainer& tc, Scheduler& sch, const std::string& dirname)
{
    boost::filesystem::path p(dirname);
    boost::filesystem::create_directories(p);
    sch.for_each(tc, [&dirname](typename TileContainer::Tile& tile) {
        std::string filename(dirname + "/" + std::to_string(tile.id()));
        write_tile_vrt_tins(filename, tile.triangulation());
        return 1;
    });
    write_vrt_header(dirname, "wkbTIN", "tins", tc.ids_begin(), tc.ids_end());
}

}
}

#endif // CGAL_DDT_WRITE_VRT_H
