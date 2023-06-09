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

#ifndef CGAL_DDT_TILE_H
#define CGAL_DDT_TILE_H

#include <CGAL/DDT/Tile_triangulation.h>
#include <CGAL/DDT/tile_points/No_tile_points.h>

namespace CGAL {
namespace DDT {

namespace Impl {
template<class Triangulation, class TileIndexProperty, class TilePoints = No_tile_points>
struct Messaging {
    typedef Triangulation_traits<Triangulation>       Traits;
    typedef typename TileIndexProperty::value_type    Tile_index;
    typedef CGAL::DDT::Tile_triangulation<Triangulation, TileIndexProperty>          Tile_triangulation;
    typedef typename Traits::Vertex_index             Vertex_index;
    typedef typename Traits::Point             Point;
    typedef std::pair<Tile_index,Point>               Point_id;
    typedef std::vector<Point_id>                     Points;
    typedef TilePoints                                Tile_points;
    typedef std::map<Tile_index, Points>              Points_map;


    Messaging() : points_() {}

    const Points_map& points() const { return points_; }
    Points_map& points() { return points_; }

    const Points& extreme_points() const { return extreme_points_; }
    Points& extreme_points() { return extreme_points_; }

    Points_map points_;
    std::vector<Tile_points> input_points_;
    Points extreme_points_;

    void send_point(Tile_index id, Tile_index i, const Point& p) {
        points_[id].push_back({i,p});
    }

    void send_vertex(Tile_index id, const Tile_triangulation& t, Vertex_index v) {
        points_[id].emplace_back(t.vertex_id(v), t.point(v));
    }

    std::size_t send_vertices(Tile_index id, const Tile_triangulation& t, const std::set<Vertex_index>& vertices) {
        Points& p = points_[id];
        for(Vertex_index v : vertices)
            p.emplace_back(t.vertex_id(v), t.point(v));
        // debug
        //if(!vertices.empty()) std::cout << "\x1B[32m" << id() << "\t->\t" << std::to_string(id) << "\t:\t" << vertices.size()   << "\x1B[0m"<< std::endl;
        return vertices.size();
    }

    std::size_t send_vertices_to_one_tile(const Tile_triangulation& t, const std::map<Tile_index, std::set<Vertex_index>>& vertices) {
        std::size_t count = 0;
        for(auto& vi : vertices)
            count += send_vertices(vi.first, t, vi.second);
        return count;
    }

    void send_vertices_to_all_tiles(const Tile_triangulation& t, const std::vector<Vertex_index>& vertices) {
        for(Vertex_index v : vertices)
            if (!t.vertex_is_infinite(v))
                extreme_points_.emplace_back(t.vertex_id(v), t.point(v));
        // debug
        //if(!vertices.empty()) std::cout << "\x1B[33m" << id() << "\t->\t*\t:\t" << vertices.size()   << "\x1B[0m" << std::endl;
    }

    void receive_points(Tile_index i, Points& received) {
        received.swap(points_[i]);
        std::vector<Point> points_read;
        for(auto& ip : input_points_)
            ip.read(std::back_inserter(points_read));
        for(auto& p : points_read)
            received.emplace_back(i, p);
        input_points_.clear();

        // debug
        //if(!received.empty()) std::cout << "\x1B[31m" << id() << "\t<-\t*\t:\t" << received.size()   << "\x1B[0m" << std::endl;

    }


    template<typename... Args>
    std::size_t insert(Args... args) {
        input_points_.emplace_back(args...);
        return input_points_.back().size();
    }

};
}

/// \ingroup PkgDDTClasses
/// \tparam T is a model of the TriangulationTraits concept
/// \tparam TilePoints is a model of the TilePoints concept
/// The Tile stores a local Delaunay triangulation.
template<class Triangulation, class TileIndexProperty, class TilePoints = No_tile_points >
class Tile
{
public:
    typedef Triangulation_traits<Triangulation>       Traits;
    typedef typename TileIndexProperty::value_type    Tile_index;
    typedef typename Traits::Bbox                     Bbox;
    typedef typename Traits::Point                    Point;
    typedef CGAL::DDT::Tile_triangulation<Triangulation, TileIndexProperty>          Tile_triangulation;
    typedef Impl::Messaging<Triangulation, TileIndexProperty, TilePoints> Messaging;

    Tile(Tile_index id, int dimension) :
        id_(id),
        triangulation_(id, dimension),
        bbox_(Traits::bbox(dimension)),
        in_mem(false),
        locked(false),
        messaging()
    {
    }

    inline Tile_index id() const { return id_; }
    const Bbox& bbox() const { return bbox_; }
    Bbox& bbox() { return bbox_; }

    const Tile_triangulation& triangulation() const { return triangulation_; }
    Tile_triangulation& triangulation() { return triangulation_; }


    /// lock the tile for exclusive use (no unloading, no concurrent processing)
    bool locked;
    /// is the triangulation in memory ?
    bool in_mem;

    inline std::size_t number_of_main_facets  () const { return number_of_main_facets_;   }
    inline std::size_t number_of_main_cells   () const { return number_of_main_cells_;    }
    inline std::size_t number_of_main_finite_vertices() const { return number_of_main_finite_vertices_; }
    inline std::size_t number_of_main_finite_facets  () const { return number_of_main_finite_facets_;   }
    inline std::size_t number_of_main_finite_cells   () const { return number_of_main_finite_cells_;    }

    void finalize()
    {
        if (in_mem) {
            triangulation_.finalize();
            number_of_main_finite_vertices_ = triangulation_.number_of_main_finite_vertices();
            number_of_main_finite_facets_ = triangulation_.number_of_main_finite_facets();
            number_of_main_finite_cells_ = triangulation_.number_of_main_finite_cells();
            number_of_main_facets_ = triangulation_.number_of_main_facets();
            number_of_main_cells_ = triangulation_.number_of_main_cells();
        }
    }
    bool is_valid(bool verbose = false, int level = 0) const
    {
        std::size_t number_of_main_finite_vertices = 0;
        std::size_t number_of_main_finite_facets = 0;
        std::size_t number_of_main_finite_cells = 0;
        std::size_t number_of_main_facets = 0;
        std::size_t number_of_main_cells = 0;
        if (in_mem) {
            if(!triangulation_.is_valid(verbose, level))
            {
                std::cerr << "DT Tile " << std::to_string(id()) << " is invalid" << std::endl;
                //assert(! "CGAL tile not valid" );
                return false;
            }
            number_of_main_finite_vertices += triangulation_.number_of_main_finite_vertices();
            number_of_main_finite_facets += triangulation_.number_of_main_finite_facets();
            number_of_main_finite_cells += triangulation_.number_of_main_finite_cells();
            number_of_main_facets += triangulation_.number_of_main_facets();
            number_of_main_cells += triangulation_.number_of_main_cells();
        }
        if (number_of_main_finite_vertices != number_of_main_finite_vertices_) { std::cerr << "incorrect number_of_finite_vertices" << std::endl; return false; }
        if (number_of_main_finite_facets != number_of_main_finite_facets_) { std::cerr << "incorrect number_of_finite_facets" << std::endl; return false; }
        if (number_of_main_finite_cells != number_of_main_finite_cells_) { std::cerr << "incorrect number_of_finite_cells" << std::endl; return false; }
        if (number_of_main_facets != number_of_main_facets_) { std::cerr << "incorrect number_of_facets" << std::endl; return false; }
        if (number_of_main_cells != number_of_main_cells_) { std::cerr << "incorrect number_of_cells" << std::endl; return false; }
        return true;
    }
private:
    Tile_index id_;
    Tile_triangulation triangulation_;
    Bbox bbox_;

    std::size_t number_of_main_finite_vertices_;
    std::size_t number_of_main_finite_facets_;
    std::size_t number_of_main_finite_cells_;
    std::size_t number_of_main_facets_;
    std::size_t number_of_main_cells_;

public:
    Messaging messaging;
};

}
}

#endif // CGAL_DDT_TILE_H
