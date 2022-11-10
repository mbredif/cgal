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

#ifndef CGAL_DDT_WRITE_DOT_H
#define CGAL_DDT_WRITE_DOT_H

#include <fstream>
#include <unordered_map>
#include <string>

namespace CGAL {
namespace DDT {

template<typename DDT>
void write_adjacency_graph_dot(const DDT& tri, const std::string& dot, bool oriented=false)
{
    typedef typename DDT::Id Id;
    std::unordered_multimap<Id,Id> edges;
    tri.get_adjacency_graph(edges);
    std::ofstream out(dot);
    out << (oriented?"digraph":"graph") << " tile_adjacency {\n";
    for(auto& p : edges)
        if(oriented || p.first < p.second)
            out << "\t"<<int(p.first) << (oriented?" -> ":" -- ") << int(p.second) << ";\n";
    out << "}";
}

}
}

#endif // CGAL_DDT_WRITE_DOT_H
