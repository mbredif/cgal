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


namespace ddt
{

template<typename Tile>
std::istream& read_json(Tile & tile,std::istream&  ifile)
{

    typedef typename Tile::Id Id;
    boost::property_tree::ptree root_node;
    boost::property_tree::read_json(ifile, root_node);
    auto & bbox = tile.bbox();
    // int id =  root_node.get<Id>("id");
    // tile.set_id(id);
    for (auto its : root_node.get_child("bbox"))
    {
        int iid = std::stoi(its.first);
        Id id = iid;
        std::stringstream ss (its.second.data());
        ss >> bbox[id];
    }
    return ifile;
}

template<typename Tile>
int read_cgal_tile(Tile& tile, const std::string& dirname)
{

    std::string filename = dirname + "/" + std::to_string(tile.id() ) + ".bin";
    std::string json_name = dirname + "/" + std::to_string(tile.id() ) + ".json";
    std::ifstream ifile_tri(filename,  std::ios::in);

    if (!ifile_tri.is_open())
    {
        std::cerr << "stream not open" << std::endl;
        return 1;
    }

    tile.read_class(ifile_tri);
    ifile_tri.close();

    std::ifstream ifile_json(json_name, std::ifstream::in);
    read_json(tile,ifile_json);
    ifile_json.close();

    return 0;
}

template<typename DDT>
int read_cgal(DDT& tri, const std::string& dirname)
{
    typedef typename DDT::Id Id;
    boost::property_tree::ptree root_node;
    std::string json_name = dirname + "/tiles.json";
    std::ifstream ifile_json(json_name, std::ifstream::in);
    boost::property_tree::read_json(ifile_json, root_node);
    for (auto its : root_node.get_child("tiles"))
    {
        Id tid = std::stoi(its.first);
        tri.init(tid);
        auto tile  = tri.get_tile(tid);
        read_cgal_tile(*tile,dirname);
    }

    return 0;
}

}

#endif // CGAL_DDT_READ_CGAL_H
