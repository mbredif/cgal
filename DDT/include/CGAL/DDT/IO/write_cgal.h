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
void json_put_statistics(boost::property_tree::ptree& node, const Statistics& stats)
{
    node.put("finite_vertices", stats.number_of_finite_vertices);
    node.put("finite_facets", stats.number_of_finite_facets);
    node.put("finite_cells", stats.number_of_finite_cells);
    node.put("facets", stats.number_of_facets);
    node.put("cells", stats.number_of_cells);
}

template<typename TileTriangulation>
bool write_cgal_tile(const TileTriangulation& triangulation, const std::string& dirname)
{
    std::string cgal_name = dirname + "/" + std::to_string(triangulation.id() ) + ".txt";
    std::ofstream ofile(cgal_name, std::ios::out);
    if(!ofile.is_open())
    {
        std::cerr << "write_cgal_tile : File could not be opened" << std::endl;
        std::cerr << cgal_name << std::endl;
        return false;
    }
    ofile.precision(17);
    ofile << triangulation;
    return !ofile.fail();
}

template<typename DistributedTriangulation>
bool write_json_tiles(const DistributedTriangulation& tri, const std::string& dirname)
{
    boost::property_tree::ptree root_node;
    root_node.put("dimension", tri.maximal_dimension());
    json_put_statistics(root_node, tri.statistics());

    boost::property_tree::ptree tiles_node;
    for(const auto& [id, triangulation] : tri.tiles)
    {
        boost::property_tree::ptree tile_node;
        std::string sid = std::to_string(id);
        tile_node.put("tile", sid+".txt");
        std::ostringstream oss;
        oss << triangulation.bbox();
        tile_node.put("bbox", oss.str());
        json_put_statistics(tile_node, triangulation.statistics());
        tiles_node.add_child(sid, tile_node);
    }
    root_node.add_child("tiles", tiles_node);

    std::string json_name = dirname + "/tiles.json";
    std::ofstream ofile(json_name, std::ios::out);
    boost::property_tree::write_json(ofile, root_node);
    return !ofile.fail();
}

}
}

#endif // CGAL_DDT_WRITE_CGAL_H
