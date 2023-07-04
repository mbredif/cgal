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
void read_json_statistics(boost::property_tree::ptree& node, Statistics& stats)
{
    stats.number_of_finite_vertices = node.get<std::size_t>("finite_vertices");
    stats.number_of_finite_facets = node.get<std::size_t>("finite_facets");
    stats.number_of_finite_cells = node.get<std::size_t>("finite_cells");
    stats.number_of_facets = node.get<std::size_t>("facets");
    stats.number_of_cells = node.get<std::size_t>("cells");
}

template<typename Tile_triangulation>
std::istream& read_json(Tile_triangulation& tri,std::istream&  ifile)
{
    boost::property_tree::ptree root_node;
    boost::property_tree::read_json(ifile, root_node);
    tri.id() =  root_node.get<typename Tile_triangulation::Tile_index>("id");
    read_json_statistics(root_node, tri.statistics());
    return ifile;
}

template<typename TileTriangulation>
int read_cgal_tile(TileTriangulation& triangulation, const std::string& dirname)
{

    std::string filename = dirname + "/" + std::to_string(triangulation.id() ) + ".bin";
    std::string json_name = dirname + "/" + std::to_string(triangulation.id() ) + ".json";
    std::ifstream ifile_tri(filename,  std::ios::in );

    if (!ifile_tri.is_open())
    {
        std::cerr << "stream not opened : " << filename << std::endl;
        return 1;
    }

    ifile_tri >> triangulation;
    ifile_tri.close();

    std::ifstream ifile_json(json_name, std::ifstream::in);
    read_json(triangulation,ifile_json);
    ifile_json.close();

    return 0;
}

template<typename DistributedTriangulation>
int read_cgal(DistributedTriangulation& tri, const std::string& dirname)
{
    typedef typename DistributedTriangulation::Tile_index Tile_index;
    typedef typename DistributedTriangulation::Tile_triangulation Tile_triangulation;
    boost::property_tree::ptree root_node;
    boost::property_tree::ptree tiles_node;
    boost::property_tree::ptree bboxes_node;
    std::string json_name = dirname + "/tiles.json";
    std::ifstream ifile_json(json_name, std::ifstream::in);
    boost::property_tree::read_json(ifile_json, root_node);

    int dimension = root_node.get<int>("dimension");
    tiles_node = root_node.get_child("tiles");
    bboxes_node = root_node.get_child("bboxes");
    read_json_statistics(root_node, tri.statistics());
    for (auto its : tiles_node)
    {
        Tile_index tid = std::stoi(its.first);
        Tile_triangulation& triangulation = tri.tiles.emplace(tid, std::move(Tile_triangulation(tid, tri.maximal_dimension()))).first->second;
        std::istringstream iss(bboxes_node.find(its.first)->second.data());
        iss >> triangulation.bbox();
        read_cgal_tile(triangulation, dirname);
    }
    return 0;
}

}
}

#endif // CGAL_DDT_READ_CGAL_H
