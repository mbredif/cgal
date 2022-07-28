#ifndef DDT_WRITE_GEOJSON_HPP
#define DDT_WRITE_GEOJSON_HPP

#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <set>

#include <boost/filesystem.hpp>
#undef BOOST_NO_CXX11_SCOPED_ENUMS
#include "conf_header/conf.hpp"


// https://en.wikipedia.org/wiki/GeoJSON
namespace ddt
{

template<typename Iterator>
void write_geojson_vert_range(Iterator begin, Iterator end, std::ostream & ofs, bool is_first = true)
{
    typedef typename Iterator::value_type Vertex_const_iterator;
    typedef typename Vertex_const_iterator::Traits Traits;
    int D = Traits::D;
    for(auto vit = begin; vit != end; ++vit)
    {
        if(vit->is_infinite()) continue;
        if(!is_first)
            ofs << "," << std::endl;
        is_first=false;
        ofs << "{" << std::endl;
        ofs << "\"type\": \"Feature\"," << std::endl;
        ofs << "\"geometry\": {" << std::endl;
        ofs << "\"type\": \"Point\"," << std::endl;
        ofs << "\"coordinates\": [";
        for(int d=0; d<D-1; ++d)
            ofs << vit->point()[d] << ",";
        ofs << vit->point()[D-1] << "]" << std::endl;;
        ofs << "}," << std::endl;
        ofs << "\"properties\": {" << std::endl;
        ofs << "\"fill\":" << (vit->is_local() ? "\"red\"" : "\"blue\"") <<  "," << std::endl;
        ofs << "\"tid\": " << int(vit->tile()->id()) <<  "," << std::endl;
        ofs << "\"id\": " <<  int(vit->main_id())  << std::endl;
        ofs << "}" << std::endl;
        ofs << "}" << std::endl;
    }
}

template<typename Iterator>
void write_geojson_cell_range(Iterator begin, Iterator end, std::ostream & ofs,bool is_first = true)
{

    typedef typename Iterator::value_type Cell_const_iterator;
    typedef typename Cell_const_iterator::Traits Traits;
    std::map<Cell_const_iterator, int> cmap;
    int nextid = 0;
    int D = Traits::D;
    for(auto iit = begin; iit != end; ++iit)
    {
        if(iit->is_infinite()) continue;
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
            auto v = iit->vertex(i % (D+1));
            if(i>0)
            {
                ofs << "],[";
                local += v->is_local();
            }
            auto p = v->point();
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

            if(!cmap.count(*iit)) cmap[*iit] = nextid++;
            ofs << "\"id\": " << cmap[*iit] << "," << std::endl;
            for(int i = 0 ; i < D+1; i++)
            {
                auto n0 = iit->neighbor(i)->main();
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
    write_geojson_vert_range(ddt.vertices_begin(), ddt.vertices_end(), ofs,true);
    write_geojson_cell_range(ddt.cells_begin(), ddt.cells_end(), ofs,false);
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

#endif // DDT_WRITE_GEOJSON_HPP
