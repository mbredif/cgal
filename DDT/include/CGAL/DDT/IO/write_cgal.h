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
bool write_cgal_tile(std::ostream& os, const TileTriangulation& triangulation)
{
    os.precision(17);
    os << triangulation;
    return !os.fail();
}

template<typename DistributedTriangulation>
bool write_json_tiles(std::ostream& os, const DistributedTriangulation& tri)
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
        json_put_statistics(tile_node, triangulation.statistics());
        tiles_node.add_child(sid, tile_node);
    }
    root_node.add_child("tiles", tiles_node);
    boost::property_tree::write_json(os, root_node);
    return !os.fail();
}

}
}

#endif // CGAL_DDT_WRITE_CGAL_H
