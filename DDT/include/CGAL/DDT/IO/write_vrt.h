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

// #include "conf_header/conf.hpp"

#include <boost/filesystem.hpp>
#undef BOOST_NO_CXX11_SCOPED_ENUMS

#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <set>

/// @todo tmp to workariound conf.hpp
#ifndef STYLE_SOURCE_DIR
#define STYLE_SOURCE_DIR ""
#endif

namespace ddt
{

// VRT header writers

inline void add_qgis_style(const std::string& filename,const std::string& stylename)
{
    boost::filesystem::path path(filename);
    boost::filesystem::path path_style_target = path.replace_extension(".qml");
    boost::filesystem::path path_style_source(std::string(STYLE_SOURCE_DIR) + stylename);
    boost::filesystem::copy_file(path_style_source, path_style_target, boost::filesystem::copy_option::overwrite_if_exists);
}

inline void write_vrt_header_vert(std::ofstream& csv, const std::string& filename)
{
    boost::filesystem::path path(filename);
    std::string stem = path.stem().string();
    std::ofstream f(filename);
    f <<"<OGRVRTDataSource>" << std::endl;
    f <<  "<OGRVRTLayer name=\"" << stem <<  "\">" << std::endl;
    f <<    "<SrcDataSource relativeToVRT=\"1\">" << stem << ".csv</SrcDataSource>" << std::endl;
    f <<    "<SrcLayer>" << stem <<  "</SrcLayer>" << std::endl;
    f <<    "<LayerSRS>IGNF:LAMB93</LayerSRS> " << std::endl;
    f <<    "<GeometryType>wkbPoint</GeometryType> " << std::endl;
    f <<    "<GeometryField encoding=\"WKT\" field=\"geom\"/> " << std::endl;
    f <<    "<Field name=\"tid\" type=\"Integer\"/>" << std::endl;
    f <<    "<Field name=\"id\" type=\"Integer\"/>" << std::endl;
    f <<  "</OGRVRTLayer>" << std::endl;
    f <<"</OGRVRTDataSource>" << std::endl;

    //    add_qgis_style(filename, std::string("vert1.qml"));
    csv.open(path.replace_extension("csv").string());
    csv << "geom,tid,id" << std::endl;
}

inline void write_vrt_header_facet(std::ofstream& csv, const std::string& filename)
{
    boost::filesystem::path path(filename);
    std::string stem = path.stem().string();
    std::ofstream f(filename);
    f <<"<OGRVRTDataSource>" << std::endl;
    f <<  "<OGRVRTLayer name=\"" << stem <<  "\">" << std::endl;
    f <<    "<SrcDataSource relativeToVRT=\"1\">" << stem << ".csv</SrcDataSource>" << std::endl;
    f <<    "<SrcLayer>" << stem <<  "</SrcLayer>" << std::endl;
    f <<    "<LayerSRS>IGNF:LAMB93</LayerSRS> " << std::endl;
    f <<    "<GeometryType>wkbLineString</GeometryType> " << std::endl;
    f <<    "<GeometryField encoding=\"WKT\" field=\"geom\"/> " << std::endl;
    f <<    "<Field name=\"tid\" type=\"Integer\"/>" << std::endl;
    f <<    "<Field name=\"local\" type=\"Integer\"/>" << std::endl;
    f <<  "</OGRVRTLayer>" << std::endl;
    f <<"</OGRVRTDataSource>" << std::endl;

    // add_qgis_style(filename,std::string("tri1.qml"));
    csv.open(path.replace_extension("csv").string());
    csv << "geom,tid,local" << std::endl;
}

inline void write_vrt_header_cell(std::ofstream& csv, const std::string& filename)
{
    boost::filesystem::path path(filename);
    std::string stem = path.stem().string();
    std::ofstream f(filename);
    f <<"<OGRVRTDataSource>" << std::endl;
    f <<  "<OGRVRTLayer name=\"" << stem <<  "\">" << std::endl;
    f <<    "<SrcDataSource relativeToVRT=\"1\">" << stem << ".csv</SrcDataSource>" << std::endl;
    f <<    "<SrcLayer>" << stem <<  "</SrcLayer>" << std::endl;
    f <<    "<LayerSRS>IGNF:LAMB93</LayerSRS> " << std::endl;
    f <<    "<GeometryType>wkbPolygon</GeometryType> " << std::endl;
    f <<    "<GeometryField encoding=\"WKT\" field=\"geom\"/> " << std::endl;
    f <<    "<Field name=\"tid\" type=\"Integer\"/>" << std::endl;
    f <<    "<Field name=\"local\" type=\"Integer\"/>" << std::endl;
    f <<    "<Field name=\"main\" type=\"Integer\"/>" << std::endl;
    f <<    "<Field name=\"cid\" type=\"Integer\"/>" << std::endl;
    f <<    "<Field name=\"cid0\" type=\"Integer\"/>" << std::endl;
    f <<    "<Field name=\"cid1\" type=\"Integer\"/>" << std::endl;
    f <<    "<Field name=\"cid2\" type=\"Integer\"/>" << std::endl;
    f <<  "</OGRVRTLayer>" << std::endl;
    f <<"</OGRVRTDataSource>" << std::endl;

    //    add_qgis_style(filename,std::string("tri1.qml"));
    csv.open(path.replace_extension("csv").string());
    csv << "geom,tid,local,main,cid,cid0,cid1,cid2" << std::endl;
}

inline void write_vrt_header_tin(std::ofstream& csv, const std::string& filename)
{
    boost::filesystem::path path(filename);
    std::string stem = path.stem().string();
    std::ofstream f(filename);
    f <<"<OGRVRTDataSource>" << std::endl;
    f <<  "<OGRVRTLayer name=\"" << stem <<  "\">" << std::endl;
    f <<    "<SrcDataSource relativeToVRT=\"1\">" << stem << ".csv</SrcDataSource>" << std::endl;
    f <<    "<SrcLayer>" << stem <<  "</SrcLayer>" << std::endl;
    f <<    "<LayerSRS>IGNF:LAMB93</LayerSRS> " << std::endl;
    f <<    "<GeometryType>wkbTIN</GeometryType> " << std::endl;
    f <<    "<GeometryField encoding=\"WKT\" field=\"geom\"/> " << std::endl;
    f <<    "<Field name=\"tid\" type=\"Integer\"/>" << std::endl;
    f <<    "<Field name=\"id\" type=\"Integer\"/>" << std::endl;
    f <<  "</OGRVRTLayer>" << std::endl;
    f <<"</OGRVRTDataSource>" << std::endl;

    // add_qgis_style(filename, std::string("tin1.qml"));
    csv.open(path.replace_extension("csv").string());
    csv << "geom,tid,id" << std::endl;
}

inline void write_vrt_header_bbox(std::ofstream& csv, const std::string& filename)
{
    boost::filesystem::path path(filename);
    std::string stem = path.stem().string();
    std::ofstream f(filename);
    f <<"<OGRVRTDataSource>" << std::endl;
    f <<  "<OGRVRTLayer name=\"" << stem <<  "\">" << std::endl;
    f <<    "<SrcDataSource relativeToVRT=\"1\">" << stem << ".csv</SrcDataSource>" << std::endl;
    f <<    "<SrcLayer>" << stem <<  "</SrcLayer>" << std::endl;
    f <<    "<LayerSRS>IGNF:LAMB93</LayerSRS> " << std::endl;
    f <<    "<GeometryType>wkbPolygon</GeometryType> " << std::endl;
    f <<    "<GeometryField encoding=\"WKT\" field=\"geom\"/> " << std::endl;
    f <<    "<Field name=\"tid\" type=\"Integer\"/>" << std::endl;
    f <<    "<Field name=\"bboxid\" type=\"Integer\"/>" << std::endl;
    f <<  "</OGRVRTLayer>" << std::endl;
    f <<"</OGRVRTDataSource>" << std::endl;

    //    add_qgis_style(filename, std::string("bbox1.qml"));
    csv.open(path.replace_extension("csv").string());
    csv << "geom,tid,bboxid" << std::endl;
}
// CSV tile writers (Tile)

template<typename Tile>
void write_csv_vert(const Tile& tile, std::ostream& csv, bool main_only=false)
{
    int D = tile.maximal_dimension();
    for(auto vit = tile.vertices_begin(); vit != tile.vertices_end(); ++vit)
    {
        if(tile.vertex_is_infinite(vit)) continue;
        if(main_only && !tile.vertex_is_main(vit)) continue;
        csv << "POINT( ";
        for(int d=0; d<D; ++d)
            csv << tile.point(vit)[d] << " ";
        csv << ")," << int(tile.id()) << "," << int(tile.id(vit)) << "\n";
    }
}

template<typename Tile>
void write_csv_facet(const Tile& tile, std::ostream& csv, bool main_only=false)
{
    int D = tile.maximal_dimension();
    for(auto fit = tile.facets_begin(); fit != tile.facets_end(); ++fit)
    {
        if(tile.facet_is_infinite(fit)) continue;
        if(main_only && !tile.facet_is_main(fit)) continue;
        auto cit = tile.full_cell(fit);
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
        csv << ")\"," << int(tile.id()) << "," << local << "\n";
    }
}

template<typename Tile>
void write_csv_cell(const Tile& tile, std::ostream& csv, bool main_only=false)
{
    int D = tile.maximal_dimension();
    for(auto cit = tile.cells_begin(); cit != tile.cells_end(); ++cit)
    {
        if(tile.cell_is_infinite(cit)) continue;
        if(main_only && !tile.cell_is_main(cit))continue;
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
        csv << "))\"," << int(tile.id()) << "," << int(local) << "," << int(tile.cell_is_main(cit)) << "\n";
    }
}





template<typename Tile>
void write_tile_vrt_cells(const Tile& tile, const std::string& vrt_name, bool main_only=false)
{
    boost::filesystem::path path(vrt_name);

    std::ofstream csv;
    write_vrt_header_cell(csv,vrt_name);
    // add_qgis_style(vrt_name,std::string("tri1.qml"));
    write_csv_cell(tile, csv,main_only);
}



template<typename Tile>
void write_csv_tin(const Tile& tile, std::ostream& csv, bool main_only=false)
{
    int D = tile.maximal_dimension();
    csv << "\"TIN (";
    bool first = true;
    for(auto cit = tile.cells_begin(); cit != tile.cells_end(); ++cit)
    {
        if(tile.cell_is_infinite(cit)) continue;
        if(main_only && !tile.cell_is_main(cit))continue;
        if(!first) csv << ", ";
        first = false;
        csv << "((";
        typename Tile::Vertex_const_handle v;
        for(int i=0; i<=D; ++i)
        {
            v = tile.vertex(cit,i);
            for(int d=0; d<D; ++d)
                csv << tile.point(v)[d] << " ";
            csv /*<< int(tile.id(v))*/ << ",";
        }
        v = tile.vertex(cit,0);
        for(int d=0; d<D; ++d) // repeat first to close the polygon
            csv << tile.point(v)[d] << " ";
        csv /*<< int(tile.id(v))*/ << "))";
    }
    csv << ")\"," << int(tile.id()) << "\n";
}

template<typename Tile>
void write_csv_bbox(const Tile& tile, std::ostream& csv)
{
    for(auto& pair : tile.bbox())
    {
        auto bboxid = pair.first;
        auto bbox   = pair.second;
        csv << "\"POLYGON((";
        csv << bbox.min(0) << " "<< bbox.min(1) << ", ";
        csv << bbox.max(0) << " "<< bbox.min(1) << ", ";
        csv << bbox.max(0) << " "<< bbox.max(1) << ", ";
        csv << bbox.min(0) << " "<< bbox.max(1) << ", ";
        csv << bbox.min(0) << " "<< bbox.min(1);
        csv << "))\"," << int(tile.id()) << "," << int(bboxid) << "\n";
    }
}

template<typename Tile>
void write_csv_bbox_vert(const Tile& tile, std::ostream& csv)
{
    int D = tile.maximal_dimension();
    std::vector<typename Tile::Vertex_const_handle> points;
    tile.get_bbox_points(points);
    for(auto it : points)
    {
        csv << "POINT( ";
        for(int d=0; d<D; ++d)
            csv << tile.point(it)[d] << " ";
        csv << ")," << int(tile.id()) << "," << int(tile.id(it)) << "\n";
    }
}

// VRT+CSV writers (iterators)

template<typename DDT, typename Iterator>
void write_vrt_vert_range(DDT& ddt, Iterator begin, Iterator end, const std::string& filename)
{
    std::ofstream csv;
    write_vrt_header_vert(csv, filename);
    typedef typename Iterator::value_type Vertex_const_iterator;
    typedef typename Vertex_const_iterator::Traits Traits;
    int D = Traits::D;
    for(auto vit = begin; vit != end; ++vit)
    {
        if(ddt.is_infinite(vit)) continue;
        csv << "POINT( ";
        for(int d=0; d<D; ++d)
            csv << ddt.point(vit)[d] << " ";
        csv << ")," << int(ddt.tile_id(vit)) << "," << int(ddt.main_id(vit)) << "\n";
    }
}

template<typename DDT, typename Iterator>
void write_vrt_facet_range(DDT& ddt, Iterator begin, Iterator end, const std::string& filename)
{
    std::ofstream csv;
    write_vrt_header_facet(csv, filename);
    typedef typename Iterator::value_type Facet_const_iterator;
    typedef typename Facet_const_iterator::Traits Traits;
    int D = Traits::D;
    for(auto fit = begin; fit != end; ++fit)
    {
        if(fit->is_infinite()) continue;
        auto cit = fit->full_cell();
        int idx = fit->index_of_covertex();
        csv << "\"LINESTRING(";
        int local = 0;
        int j = 0;
        for(int i=0; i<=D; ++i)
        {
            if(i == idx) continue;
            auto v = ddt.vertex(cit, i);
            local += ddt.is_local(v);
            for(int d=0; d<D; ++d)
                csv << ddt.point(v)[d] << " ";
            if (++j < D) csv << ",";
        }
        csv << ")\"," << int(fit->tile()->id()) << "," << local << "\n";
    }
}

template<typename DDT, typename Iterator>
void write_vrt_cell_range(DDT& ddt, Iterator begin, Iterator end, const std::string& filename)
{
    std::ofstream csv;
    write_vrt_header_cell(csv, filename);
    typedef typename Iterator::value_type Cell_const_iterator;
    typedef typename Cell_const_iterator::Traits Traits;
    std::map<Cell_const_iterator, int> cmap;
    int nextid = 0;
    int D = Traits::D;
    for(auto iit = begin; iit != end; ++iit)
    {
        if(iit->is_infinite()) continue;
        csv << "\"POLYGON((";
        int local = 0;
        for(int i=0; i<=D+1; ++i) // repeat first to close the polygon
        {
            auto v = ddt.vertex(*iit, i % (D+1));
            if(i>0)
            {
                csv << ",";
                local += ddt.is_local(v);
            }
            auto p = ddt.point(v);
            for(int d=0; d<D; ++d) csv << p[d] << " ";
        }
        csv << "))\"," << int(iit->tile()->id()) << "," << int(local) << "," << int(iit->main_id());
        auto n0 = iit->neighbor(0)->main();
        auto n1 = iit->neighbor(1)->main();
        auto n2 = iit->neighbor(2)->main();
        if(!cmap.count(*iit)) cmap[*iit] = nextid++;
        if(!cmap.count(n0)) cmap[n0] = nextid++;
        if(!cmap.count(n1)) cmap[n1] = nextid++;
        if(!cmap.count(n2)) cmap[n2] = nextid++;
        csv << "," << cmap[*iit] << "," << cmap[n0] << "," << cmap[n1] << "," << cmap[n2];
        csv << "," << 0 << "," << 0 << "," << 0 << "," << 0;
        csv << "\n";
    }
}

// VRT+CSV writers (DDT)

template<typename DDT>
void write_vrt_vert(DDT& ddt, const std::string& filename)
{
    write_vrt_vert_range(ddt, ddt.vertices_begin(), ddt.vertices_end(), filename);
}

template<typename DDT>
void write_vrt_facet(DDT& ddt, const std::string& filename)
{
    write_vrt_facet_range(ddt, ddt.facets_begin(), ddt.facets_end(), filename);
}

template<typename DDT>
void write_vrt_cell(DDT& ddt, const std::string& filename)
{
    write_vrt_cell_range(ddt, ddt.cells_begin(), ddt.cells_end(), filename);
}

template<typename DDT>
void write_vrt_tin(DDT& tri, const std::string& filename)
{
    std::ofstream csv;
    write_vrt_header_tin(csv, filename);
    for(auto tile = tri.tiles_begin(); tile != tri.tiles_end(); ++tile)
        write_csv_tin(*tile, csv);
}

template<typename DDT>
void write_vrt_bbox(DDT& tri, const std::string& filename)
{
    std::ofstream csv;
    write_vrt_header_bbox(csv, filename);
    for(auto tile = tri.tiles_begin(); tile != tri.tiles_end(); ++tile)
        write_csv_bbox(*tile, csv);
}

template<typename DDT>
void write_vrt_bbox_vert(DDT& tri, const std::string& filename)
{
    std::ofstream csv;
    write_vrt_header_vert(csv, filename);
    for(auto tile = tri.tiles_begin(); tile != tri.tiles_end(); ++tile)
        write_csv_bbox_vert(*tile, csv);
}

// VRT+CSV writers (DDT tiles)

template<typename DDT>
void write_vrt_verts(DDT& tri, const std::string& dirname)
{
    boost::filesystem::path p(dirname);
    if(boost::filesystem::exists(p.parent_path()))
    {
        for(auto tile = tri.tiles_begin(); tile != tri.tiles_end(); ++tile)
        {
            std::string filename(dirname + std::string("/tile_verts") + std::to_string(tile->id()) + ".vrt");
            std::ofstream csv;
            write_vrt_header_vert(csv, filename);
            write_csv_vert(*tile, csv, true);
        }
    }
    else
    {
        std::cerr << "[ERROR]" << dirname << " does not exist, create it before writing" << std::endl;
    }
}

template<typename DDT>
void write_vrt_facets(DDT& tri, const std::string& dirname)
{
    boost::filesystem::path p(dirname);
    if(boost::filesystem::exists(p.parent_path()))
    {
        for(auto tile = tri.tiles_begin(); tile != tri.tiles_end(); ++tile)
        {
            std::string filename(dirname  + std::string("/tile_facets_") + std::to_string(tile->id()) + ".vrt");
            std::ofstream csv;
            write_vrt_header_facet(csv, filename);
            write_csv_facet(*tile, csv, true);
        }
    }
    else
    {
        std::cerr << "[ERROR]" << dirname << " does not exist, create it before writing" << std::endl;
    }
}

template<typename DDT>
void write_vrt_cells(DDT& tri, const std::string& dirname)
{
    boost::filesystem::path p(dirname);
    if(boost::filesystem::exists(p.parent_path()))
    {
        for(auto tile = tri.tiles_begin(); tile != tri.tiles_end(); ++tile)
        {
            std::string filename(dirname + std::string("/tile_cell_") +  std::to_string(tile->id()) + ".vrt");
            std::ofstream csv;
            write_vrt_header_cell(csv, filename);
            write_csv_cell(*tile, csv, true);
        }
    }
    else
    {
        std::cerr << "[ERROR]" << dirname << " does not exist, create it before writing" << std::endl;
    }
}

}

#endif // CGAL_DDT_WRITE_VRT_H
