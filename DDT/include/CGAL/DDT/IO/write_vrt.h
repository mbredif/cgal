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
bool write_union_vrt_header(const std::string& dirname, const std::string& prefix, const std::string& suffix,
    const std::string& type, const std::string& union_name, DistributedTriangulation& tri, bool local)
{
    std::ofstream f(dirname + "/" + prefix + suffix + ".vrt");
    f <<"<OGRVRTDataSource>" << std::endl;
    f <<"<OGRVRTUnionLayer name=\""<< union_name << "\">" << std::endl;
    f <<"<SourceLayerFieldName>tile</SourceLayerFieldName>" << std::endl;
    for(const auto& [id, tile] : tri.tiles) {
        std::string name = std::to_string(id);
        f <<  "<OGRVRTLayer name=\"" << name <<  "\">" << std::endl;
        f <<    "<SrcDataSource relativeToVRT=\"1\">" << prefix << "/" << name << ".csv</SrcDataSource>" << std::endl;
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

bool write_vrt_header(std::ostream& vrt, const std::string& stem, const std::string& type, bool local)
{
    vrt << "<OGRVRTDataSource>" << std::endl;
    vrt <<  "<OGRVRTLayer name=\"" << stem <<  "\">" << std::endl;
    vrt <<    "<SrcDataSource relativeToVRT=\"1\">" << stem << ".csv</SrcDataSource>" << std::endl;
    vrt <<    "<SrcLayer>" << stem <<  "</SrcLayer>" << std::endl;
    vrt <<    "<LayerSRS>IGNF:LAMB93</LayerSRS> " << std::endl;
    vrt <<    "<GeometryType>" << type << "</GeometryType>" << std::endl;
    vrt <<    "<GeometryField encoding=\"WKT\" field=\"geom\"/>" << std::endl;
    vrt <<    "<Field name=\"id\" type=\"Integer\"/>" << std::endl;
    if (local) vrt <<    "<Field name=\"local\" type=\"Integer\"/>" << std::endl;
    vrt <<  "</OGRVRTLayer>" << std::endl;
    vrt <<"</OGRVRTDataSource>" << std::endl;
    return !vrt.fail();
}

// CSV tile writers

template<typename TileTriangulation>
bool write_csv_vert(std::ostream& csv, const TileTriangulation& triangulation)
{
    typedef typename TileTriangulation::Vertex_index          Vertex_index;
    typedef typename TileTriangulation::Point_const_reference Point_const_reference;
    csv << "geom,id" << std::endl;
    int D = triangulation.maximal_dimension();
    for(Vertex_index v = triangulation.vertices_begin(); v != triangulation.vertices_end(); ++v)
    {
        if(triangulation.vertex_is_infinite(v)) continue;
        csv << "POINT( ";
        Point_const_reference p = triangulation.point(v);
        for(int d=0; d<D; ++d)
            csv << approximate_cartesian_coordinate(p,d) << " ";
        csv << ")," << std::to_string(triangulation.vertex_id(v)) << "\n";
    }
    return !csv.fail();
}

template<typename TileTriangulation>
bool write_csv_facet(std::ostream& csv, const TileTriangulation& triangulation)
{
    typedef typename TileTriangulation::Vertex_index          Vertex_index;
    typedef typename TileTriangulation::Facet_index           Facet_index;
    typedef typename TileTriangulation::Cell_index            Cell_index;
    typedef typename TileTriangulation::Point_const_reference Point_const_reference;
    csv << "geom,id,local" << std::endl;
    int D = triangulation.maximal_dimension();
    for(Facet_index f = triangulation.facets_begin(); f != triangulation.facets_end(); ++f)
    {
        if(triangulation.facet_is_infinite(f)) continue;
        Cell_index c = triangulation.cell_of_facet(f);
        int idx = triangulation.index_of_covertex(f);
        csv << "\"LINESTRING(";
        int local = 0;
        int j = 0;
        for(int i=0; i<=D; ++i)
        {
            if(i == idx) continue;
            Vertex_index v = triangulation.vertex(c,i);
            local += triangulation.vertex_is_local(v);
            Point_const_reference p = triangulation.point(v);
            for(int d=0; d<D; ++d)
                csv << approximate_cartesian_coordinate(p, d) << " ";
            if (++j < D) csv << ",";
        }
        csv << ")\"," << std::to_string(triangulation.facet_id(f)) << "," << local << "\n";
    }
    return !csv.fail();
}

template<typename TileTriangulation>
bool write_csv_cell(std::ostream& csv, const TileTriangulation& triangulation)
{
    typedef typename TileTriangulation::Vertex_index          Vertex_index;
    typedef typename TileTriangulation::Cell_index            Cell_index;
    typedef typename TileTriangulation::Point_const_reference Point_const_reference;
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
            Point_const_reference p = triangulation.point(v);
            for(int d=0; d<D; ++d)
                csv << approximate_cartesian_coordinate(p, d) << " ";
            csv << ",";
        }
        Vertex_index v = triangulation.vertex(c,0);
        Point_const_reference p = triangulation.point(v);
        for(int d=0; d<D; ++d) // repeat first to close the polygon
            csv << approximate_cartesian_coordinate(p, d) << " ";
        csv << "))\"," << std::to_string(triangulation.cell_id(c)) << "," << local << "\n";
    }
    return !csv.fail();
}

template<typename TileTriangulation>
bool write_csv_tin(std::ostream& csv, const TileTriangulation& triangulation)
{
    typedef typename TileTriangulation::Vertex_index          Vertex_index;
    typedef typename TileTriangulation::Cell_index            Cell_index;
    typedef typename TileTriangulation::Point_const_reference Point_const_reference;
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
            Point_const_reference p = triangulation.point(v);
            for(int d=0; d<D; ++d)
                csv << approximate_cartesian_coordinate(p, d) << " ";
            csv << ",";
        }
        v = triangulation.vertex(c,0);
        Point_const_reference p = triangulation.point(v);
        for(int d=0; d<D; ++d) // repeat first to close the polygon
            csv << approximate_cartesian_coordinate(p,d) << " ";
        csv << "))";
    }
    if (!first) csv << ")\"," << std::to_string(triangulation.id()) << "\n";
    return !csv.fail();
}

// VRT+CSV tile writers

template<typename TileTriangulation>
bool write_tile_vrt_verts(const std::string& dirname, const TileTriangulation& triangulation)
{
    std::string stem = std::to_string(triangulation.id());
    std::ofstream csv(dirname + "/" + stem + ".csv");
    std::ofstream vrt(dirname + "/" + stem + ".vrt");
    return write_vrt_header(vrt, stem, "wkbPoint", false) && write_csv_vert(csv, triangulation);
}

template<typename TileTriangulation>
bool write_tile_vrt_facets(const std::string& dirname, const TileTriangulation& triangulation)
{
    std::string stem = std::to_string(triangulation.id());
    std::ofstream csv(dirname + "/" + stem + ".csv");
    std::ofstream vrt(dirname + "/" + stem + ".vrt");
    return write_vrt_header(vrt, std::to_string(triangulation.id()), "wkbLineString", true) && write_csv_facet(csv, triangulation);
}

template<typename TileTriangulation>
bool write_tile_vrt_cells(const std::string& dirname, const TileTriangulation& triangulation)
{
    std::string stem = std::to_string(triangulation.id());
    std::ofstream csv(dirname + "/" + stem + ".csv");
    std::ofstream vrt(dirname + "/" + stem + ".vrt");
    return write_vrt_header(vrt, std::to_string(triangulation.id()), "wkbPolygon", true) && write_csv_cell(csv, triangulation);
}

template<typename TileTriangulation>
bool write_tile_vrt_tins(const std::string& dirname, const TileTriangulation& triangulation)
{
    std::string stem = std::to_string(triangulation.id());
    std::ofstream csv(dirname + "/" + stem + ".csv");
    std::ofstream vrt(dirname + "/" + stem + ".vrt");
    return write_vrt_header(vrt, std::to_string(triangulation.id()), "wkbTIN", false) && write_csv_tin(csv, triangulation);
}

}
}

#endif // CGAL_DDT_WRITE_VRT_H
