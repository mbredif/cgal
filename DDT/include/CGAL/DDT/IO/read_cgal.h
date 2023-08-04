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

#ifndef CGAL_DDT_READ_CGAL_H
#define CGAL_DDT_READ_CGAL_H

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/list.hpp>

#include <iostream>
#include <string>
#include <fstream>

namespace CGAL {
namespace DDT {

template<typename Statistics>
void json_get_statistics(const boost::property_tree::ptree& node, Statistics& stats)
{
    stats.number_of_finite_vertices = node.get<std::size_t>("finite_vertices");
    stats.number_of_finite_facets = node.get<std::size_t>("finite_facets");
    stats.number_of_finite_cells = node.get<std::size_t>("finite_cells");
    stats.number_of_facets = node.get<std::size_t>("facets");
    stats.number_of_cells = node.get<std::size_t>("cells");
    stats.valid = true;
}

template<typename TileTriangulation>
bool read_cgal_tile(TileTriangulation& triangulation, const std::string& dirname)
{
    std::string cgal_name = dirname + "/" + std::to_string(triangulation.id() ) + ".txt";
    std::ifstream ifile(cgal_name,  std::ios::in );
    if(!ifile.is_open())
    {
        std::cerr << "read_cgal_tile : File could not be opened" << std::endl;
        std::cerr << cgal_name << std::endl;
        return false;
    }

    ifile >> triangulation;
    return !ifile.fail();
}

template<typename DistributedTriangulation>
bool read_json_tiles(DistributedTriangulation& tri, const std::string& dirname)
{
    std::string json_name = dirname + "/tiles.json";
    std::ifstream ifile(json_name, std::ifstream::in);
    boost::property_tree::ptree root_node;
    boost::property_tree::read_json(ifile, root_node);
    if (ifile.fail()) return false;

    typedef typename DistributedTriangulation::Tile_index Tile_index;
    typedef typename DistributedTriangulation::Tile_triangulation Tile_triangulation;

    int dimension = root_node.get<int>("dimension");
    tri.maximal_dimension() = dimension;
    json_get_statistics(root_node, tri.statistics());
    boost::property_tree::ptree tiles_node = root_node.get_child("tiles");
    for(const auto& [sid, node] : tiles_node)
    {
        Tile_index id = std::stoi(sid);
        Tile_triangulation& triangulation = tri.tiles.try_emplace(id, id, dimension).first->second;
        std::istringstream iss(node.get_child("bbox").data());
        iss >> triangulation.bbox();
        json_get_statistics(node, triangulation.statistics());
    }
    return true;
}

}
}

#endif // CGAL_DDT_READ_CGAL_H
