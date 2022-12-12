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

template<typename Tile>
std::istream& read_json(Tile & tile,std::istream&  ifile)
{

    typedef typename Tile::Id Id;
    boost::property_tree::ptree root_node;
    boost::property_tree::read_json(ifile, root_node);
    Id id =  root_node.get<Id>("id");
    assert(id == tile.id());
    return ifile;
}

template<typename Tile>
int read_cgal_tile(Tile& tile, const std::string& dirname)
{

    std::string filename = dirname + "/" + std::to_string(tile.id() ) + ".bin";
    std::string json_name = dirname + "/" + std::to_string(tile.id() ) + ".json";
    std::ifstream ifile_tri(filename,  std::ios::in );

    if (!ifile_tri.is_open())
    {
        std::cerr << "stream not opened : " << filename << std::endl;
        return 1;
    }

    ifile_tri >> tile.triangulation().triangulation();
    ifile_tri.close();

    std::ifstream ifile_json(json_name, std::ifstream::in);
    read_json(tile,ifile_json);
    ifile_json.close();

    return 0;
}

template<typename TileContainer>
int read_cgal(TileContainer& tc, const std::string& dirname)
{
    typedef typename TileContainer::Id Id;
    boost::property_tree::ptree root_node;
    boost::property_tree::ptree tiles_node;
    boost::property_tree::ptree bboxes_node;
    std::string json_name = dirname + "/tiles.json";
    std::ifstream ifile_json(json_name, std::ifstream::in);
    boost::property_tree::read_json(ifile_json, root_node);

    tiles_node = root_node.get_child("tiles");
    bboxes_node = root_node.get_child("bboxes");
    for (auto its : tiles_node)
    {
        Id tid = std::stoi(its.first);
        auto& tile = tc[tid];
        tc.load(tile);
        std::istringstream iss(bboxes_node.find(its.first)->second.data());
        iss >> tile.bbox();
        read_cgal_tile(tile,dirname);
    }
    tc.finalize();
    std::cout << tc.is_valid() << std::endl;
    return 0;
}

}
}

#endif // CGAL_DDT_READ_CGAL_H
