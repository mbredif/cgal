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

template<typename Tile>
std::ostream & write_json(Tile & tile,std::string filename,std::ostream & ofile)
{
    boost::property_tree::ptree root_node;
    root_node.put("filename", filename);
    root_node.put("id", std::to_string(tile.id()));
    root_node.put("nbmc", tile.triangulation().number_of_main_finite_cells());
    root_node.put("nbmv", tile.triangulation().number_of_main_finite_vertices());
    root_node.put("nbmf", tile.triangulation().number_of_main_finite_facets());
    boost::property_tree::write_json(ofile, root_node);
    return ofile;
}

template<typename Tile>
bool write_cgal_tile(const Tile& tile, std::string dirname)
{
    std::string filename = dirname + "/" + std::to_string(tile.id() ) + ".bin";
    std::string json_name = dirname + "/" + std::to_string(tile.id() ) + ".json";
    std::ofstream ofile_tri(filename, std::ios::out);
    std::ofstream ofile_json(json_name, std::ios::out);
    if(!ofile_tri.is_open())
    {
        std::cerr << "write_cgal_tile : File could not be opened" << std::endl;
        std::cerr << filename << std::endl;
        return false;
    }

    ofile_tri.precision(17);
    ofile_tri << tile.triangulation();
    ofile_tri.close();
    write_json(tile,filename,ofile_json);
    ofile_json.close();
    return true;
}

template<typename DistributedTriangulation>
int write_cgal(const DistributedTriangulation& tri, const std::string& dirname)
{
    boost::property_tree::ptree root_node;
    boost::property_tree::ptree tiles_node;
    boost::property_tree::ptree bboxes_node;

    int i = 0;
    for(auto& tile : tri.tiles)
    {
        std::string id = std::to_string(tile.id());
        std::string fpath = id + ".bin";
        std::ostringstream ss;
        ss << tile.bbox();
        tiles_node.put (id, fpath);
        bboxes_node.put(id, ss.str());
        i += !write_cgal_tile(tile, dirname);
    }
    root_node.add_child("tiles", tiles_node);
    root_node.add_child("bboxes", bboxes_node);
    std::string json_name = dirname + "/tiles.json";
    std::ofstream ofile(json_name, std::ios::out);
    boost::property_tree::write_json(ofile, root_node);
    ofile.close();

    return i;
}

}
}

#endif // CGAL_DDT_WRITE_CGAL_H
