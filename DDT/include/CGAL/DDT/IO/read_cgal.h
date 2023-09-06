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
bool read_cgal_tile(std::istream& is, TileTriangulation& triangulation)
{
    is >> triangulation;
    return !is.fail();
}

template<typename DistributedTriangulation>
bool read_cgal_json(std::istream& is, DistributedTriangulation& tri)
{
    boost::property_tree::ptree root_node;
    boost::property_tree::read_json(is, root_node);
    if (is.fail()) return false;

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
        json_get_statistics(node, triangulation.statistics());
    }
    return true;
}

}
}

#endif // CGAL_DDT_READ_CGAL_H
