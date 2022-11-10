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

#ifndef CGAL_DDT_WRITE_GEOJSON_H
#define CGAL_DDT_WRITE_GEOJSON_H

#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <set>

#include <boost/filesystem.hpp>
#undef BOOST_NO_CXX11_SCOPED_ENUMS
// #include "conf_header/conf.hpp"


// https://en.wikipedia.org/wiki/GeoJSON
namespace CGAL {
namespace DDT {

template<typename DDT, typename Iterator>
void write_geojson_vert_range(const DDT& ddt, Iterator begin, Iterator end, std::ostream & ofs, bool is_first = true)
{
    typedef typename Iterator::value_type Vertex_const_iterator;
    typedef typename Vertex_const_iterator::Traits Traits;
    int D = Traits::D;
    for(auto vit = begin; vit != end; ++vit)
    {
        if(ddt.is_infinite(vit)) continue;
        if(!is_first)
            ofs << "," << std::endl;
        is_first=false;
        ofs << "{" << std::endl;
        ofs << "\"type\": \"Feature\"," << std::endl;
        ofs << "\"geometry\": {" << std::endl;
        ofs << "\"type\": \"Point\"," << std::endl;
        ofs << "\"coordinates\": [";
        for(int d=0; d<D-1; ++d)
            ofs << ddt.point(vit)[d] << ",";
        ofs << ddt.point(vit)[D-1] << "]" << std::endl;;
        ofs << "}," << std::endl;
        ofs << "\"properties\": {" << std::endl;
        ofs << "\"fill\":" << (ddt.is_local(vit) ? "\"red\"" : "\"blue\"") <<  "," << std::endl;
        ofs << "\"tid\": " << int(ddt.tile_id(vit)) <<  "," << std::endl;
        ofs << "\"id\": " <<  int(ddt.main_id(vit))  << std::endl;
        ofs << "}" << std::endl;
        ofs << "}" << std::endl;
    }
}

template<typename DDT, typename Iterator>
void write_geojson_cell_range(const DDT& ddt, Iterator begin, Iterator end, std::ostream & ofs,bool is_first = true)
{

    typedef typename Iterator::value_type Cell_const_iterator;
    typedef typename Cell_const_iterator::Traits Traits;
    std::map<Cell_const_iterator, int> cmap;
    int nextid = 0;
    int D = Traits::D;
    for(auto iit = begin; iit != end; ++iit)
    {
        if(ddt.is_infinite(iit)) continue;
        if(!is_first)
            ofs << "," << std::endl;
        is_first=false;
        ofs << "{" << std::endl;
        ofs << "\"type\": \"Feature\"," << std::endl;
        ofs << "\"geometry\": {" << std::endl;
        ofs << "\"type\": \"Polygon\"," << std::endl;
        ofs << "\"coordinates\": [" << std::endl;
        int local = 0;
        ofs << "[[";
        for(int i=0; i<=D+1; ++i) // repeat first to close the polygon
        {
            auto v = ddt.vertex(iit, i % (D+1));
            if(i>0)
            {
                ofs << "],[";
                local += ddt.is_local(v);
            }
            auto p = ddt.point(v);
            for(int d=0; d<D-1; ++d) ofs << p[d] << ",";
            ofs << p[D-1];
        }
        ofs << "]]";
        ofs << "]";
        ofs << "}," << std::endl;
        ofs << "\"properties\": {" << std::endl;
        switch(local)
        {
        case 0 :
            ofs << "\"fill\":\"red\"," << std::endl;
            break;
        case 1 :
            ofs << "\"fill\":\"green\"," << std::endl;
            break;
        case 2 :
            ofs << "\"fill\":\"blue\"," << std::endl;
            break;
        }
        ofs << "\"stroke-width\":\"2\"," <<  std::endl;
        ofs << "\"local\": " << local << "," << std::endl;

        if(true)
        {

            if(!cmap.count(iit)) cmap[iit] = nextid++;
            ofs << "\"id\": " << cmap[iit] << "," << std::endl;
            for(int i = 0 ; i < D+1; i++)
            {
                auto n0 = ddt.main(ddt.neighbor(iit, i));
                int iid = -1;
                if(!cmap.count(n0)) cmap[n0] = nextid++;
                iid = cmap[n0];
                ofs << "\"neigbhor " << i << "\": " << iid << "," << std::endl;
            }
        }
        ofs << "\"prop1\": { \"this\": \"that\" }" << std::endl;
        ofs << "}" << std::endl;
        ofs << "}" << std::endl;
    }
}

template<typename DDT>
void write_geojson_tri(const DDT& ddt, std::ostream & ofs)
{

    ofs << "{" << std::endl;
    ofs << "\"type\": \"FeatureCollection\"," << std::endl;
    ofs << "\"features\": [" << std::endl;
    write_geojson_vert_range(ddt, ddt.vertices_begin(), ddt.vertices_end(), ofs,true);
    write_geojson_cell_range(ddt, ddt.cells_begin(), ddt.cells_end(), ofs,false);
    ofs << "]" << std::endl;
    ofs << "}" << std::endl;
}

template<typename tile>
void write_geojson_tile(const tile& tt, std::ostream & ofs)
{

    ofs << "{" << std::endl;
    ofs << "\"type\": \"FeatureCollection\"," << std::endl;
    ofs << "\"features\": [" << std::endl;
    write_geojson_vert_range(tt.vertices_begin(), tt.vertices_end(), ofs,true);
    write_geojson_cell_range(tt.cells_begin(), tt.cells_end(), ofs,false);
    ofs << "]" << std::endl;
    ofs << "}" << std::endl;
}

}
}

#endif // CGAL_DDT_WRITE_GEOJSON_H
