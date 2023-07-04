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

#ifndef CGAL_DDT_WRITE_CGAL_H
#define CGAL_DDT_WRITE_CGAL_H

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <string>
#include <fstream>

namespace CGAL {
namespace DDT {

template<typename Statistics>
void write_json_statistics(boost::property_tree::ptree& node, const Statistics& stats)
{
    node.put("finite_vertices", stats.number_of_finite_vertices);
    node.put("finite_facets", stats.number_of_finite_facets);
    node.put("finite_cells", stats.number_of_finite_cells);
    node.put("facets", stats.number_of_facets);
    node.put("cells", stats.number_of_cells);
}

template<typename TileTriangulation>
std::ostream & write_json(TileTriangulation & triangulation,std::string filename,std::ostream & ofile)
{
    boost::property_tree::ptree root_node;
    root_node.put("filename", filename);
    root_node.put("id", std::to_string(triangulation.id()));
    write_json_statistics(root_node, triangulation.statistics());
    boost::property_tree::write_json(ofile, root_node);
    return ofile;
}

template<typename TileTriangulation>
bool write_cgal_tile(const TileTriangulation& triangulation, std::string dirname)
{
    std::string filename = dirname + "/" + std::to_string(triangulation.id() ) + ".bin";
    std::string json_name = dirname + "/" + std::to_string(triangulation.id() ) + ".json";
    std::ofstream ofile_tri(filename, std::ios::out);
    std::ofstream ofile_json(json_name, std::ios::out);
    if(!ofile_tri.is_open())
    {
        std::cerr << "write_cgal_tile : File could not be opened" << std::endl;
        std::cerr << filename << std::endl;
        return false;
    }

    ofile_tri.precision(17);
    ofile_tri << triangulation;
    ofile_tri.close();
    write_json(triangulation, filename, ofile_json);
    ofile_json.close();
    return true;
}

template<typename DistributedTriangulation>
int write_cgal(const DistributedTriangulation& tri, const std::string& dirname)
{
    boost::property_tree::ptree root_node;
    boost::property_tree::ptree tiles_node;
    boost::property_tree::ptree bboxes_node;
    typedef typename DistributedTriangulation::Tile_triangulation Tile_triangulation;

    int i = 0;
    for(auto& [id, tile] : tri.tiles)
    {
        const Tile_triangulation& triangulation = tile;
        std::string sid = std::to_string(id);
        std::string fpath = sid + ".bin";
        std::ostringstream ss;
        ss << triangulation.bbox();
        tiles_node.put (sid, fpath);
        bboxes_node.put(sid, ss.str());
        i += !write_cgal_tile(triangulation, dirname);
    }
    root_node.put("dimension", tri.maximal_dimension());
    root_node.add_child("tiles", tiles_node);
    root_node.add_child("bboxes", bboxes_node);
    write_json_statistics(root_node, tri.statistics());
    std::string json_name = dirname + "/tiles.json";
    std::ofstream ofile(json_name, std::ios::out);
    boost::property_tree::write_json(ofile, root_node);
    ofile.close();

    return i;
}

}
}

#endif // CGAL_DDT_WRITE_CGAL_H
