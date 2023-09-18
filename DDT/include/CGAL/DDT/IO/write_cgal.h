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

namespace ddt
{

template<typename Tile>
std::ostream & write_json(Tile & tile,std::string filename,std::ostream & ofile)
{
    boost::property_tree::ptree root_node;
    boost::property_tree::ptree bbox_node;
    root_node.put("filename", filename);
    root_node.put("id", tile.id());
    root_node.put("nbmc", tile.number_of_main_cells());
    root_node.put("nbmv", tile.number_of_main_vertices());
    root_node.put("nbmf", tile.number_of_main_facets());

    auto & bbox = tile.bbox();
    for(auto iter = bbox.begin(); iter != bbox.end(); ++iter)
    {
        std::stringstream ss;
        ss << iter->second;
        bbox_node.put(std::to_string(iter->first),ss.str());
    }

    root_node.add_child("bbox", bbox_node);
    boost::property_tree::write_json(ofile, root_node);
    return ofile;
}

template<typename Tile>
int write_cgal_tile(const Tile& tile, std::string dirname)
{
    std::string filename = dirname + "/" + std::to_string(tile.id() ) + ".bin";
    std::string json_name = dirname + "/" + std::to_string(tile.id() ) + ".json";
    std::ofstream ofile_tri(filename, std::ios::out);
    std::ofstream ofile_json(json_name, std::ios::out);
    if(!ofile_tri.is_open())
    {
        std::cerr << "dump_tri_binary : File could not be opened" << std::endl;
        std::cerr << filename << std::endl;
        return 1;
    }

    //tile.write_cgal(ofile_tri);
    ofile_tri << tile;
    ofile_tri.close();
    write_json(tile,filename,ofile_json);
    ofile_json.close();
    return 0;
}



template <typename list_edges>
int dump_edge_binary(const std::string& filename, list_edges & edge)
{

    std::ofstream ofile(filename, std::ios::binary | std::ios::out);
    if(!ofile.is_open())
    {
        std::cerr << "dump_edge_binary : File could not be opened" << std::endl;
        std::cerr << filename << std::endl;
        return 1;
    }
    uint nbe = edge.size();
    ofile.write(reinterpret_cast<char *>(&nbe), sizeof(nbe));
    for(auto it = edge.begin(); it != edge.end(); ++it)
    {
        int id1 = it->first;
        int id2 = it->second;
        ofile.write(reinterpret_cast<char *>(&id1), sizeof(id1));
        ofile.write(reinterpret_cast<char *>(&id2), sizeof(id2));
    }
    return 0;
}

template<typename Tile>
int write_iedge_tile(const Tile& tile, std::string dirname)
{
    tile.write_maps();
    return 0;
}

template<typename DDT>
int write_cgal(const DDT& tri, const std::string& dirname)
{
    boost::property_tree::ptree root_node;
    boost::property_tree::ptree tiles_node;

    for(auto  tile = tri.tiles_begin(); tile != tri.tiles_end(); ++tile)
    {
        std::string fpath = std::to_string(tile->id())  + ".bin";
        tiles_node.put(std::to_string(tile->id()),fpath);
    }
    root_node.add_child("tiles", tiles_node);

    std::string json_name = dirname + "/tiles.json";
    std::ofstream ofile(json_name, std::ios::out);
    boost::property_tree::write_json(ofile, root_node);
    ofile.close();


    int i = 0;
    for(auto tile = tri.tiles_begin(); tile != tri.tiles_end(); ++tile)
        i += write_cgal_tile(*tile, dirname);
    return i;
}

template<typename DDT>
int write_iedge(const DDT& tri, const std::string& dirname)
{
    int i = 0;
    for(auto tile = tri.tiles_begin(); tile != tri.tiles_end(); ++tile)
        i += write_iedge_tile(*tile, dirname);
    return i;
}

}

#endif // CGAL_DDT_WRITE_CGAL_H
