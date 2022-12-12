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

#include "Tile_triangulation.h"

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTClasses
/// \tparam T is a model of the TriangulationTraits concept
/// \tparam Selector is a template for a model of the Selector concept (defaults to Median_selector)
/// The Tile stores a local Delaunay triangulation.
/// The main id of a simplex is defined by the selector
template<class T>
class Tile
{
public:
    typedef T                                         Traits;
    typedef typename Traits::Id                       Id;
    typedef typename Traits::Bbox                     Bbox;
    typedef typename Traits::Point                    Point;
    typedef typename Traits::Vertex_const_handle      Vertex_const_handle;
    typedef std::pair<Id,Point>                       Point_id;
    typedef std::vector<Point_id>                     Points;
    typedef std::map<Id, Points>                      Points_map;
    typedef CGAL::DDT::Tile_triangulation<T> Tile_triangulation;

    Tile(Id id, Traits t) :
        id_(id),
        traits(t),
        triangulation_(id, t),
        bbox_(t.dimension()),
        points_(),
        in_mem(false),
        locked(false)
    {}

    inline void set_id(Id i) { id_ = i; }
    inline Id id() const { return id_; }
    const Bbox& bbox() const { return bbox_; }
    Bbox& bbox() { return bbox_; }

    const Points_map& points() const { return points_; }
    Points_map& points() { return points_; }

    const Points& extreme_points() const { return extreme_points_; }
    Points& extreme_points() { return extreme_points_; }

    const Tile_triangulation& triangulation() const { return triangulation_; }
    Tile_triangulation& triangulation() { return triangulation_; }


    void send_point(Id id, Id i, const Point& p) {
        points_[id].push_back({i,p});
    }

    void send_vertex(Id id, Vertex_const_handle v) {
        points_[id].emplace_back(triangulation_.vertex_id(v), triangulation_.point(v));
    }

    std::size_t send_vertices(Id id, const std::set<Vertex_const_handle>& vertices) {
        Points& p = points_[id];
        for(Vertex_const_handle v : vertices)
            p.emplace_back(triangulation_.vertex_id(v), triangulation_.point(v));
        // debug
        //if(!vertices.empty()) std::cout << "\x1B[32m" << id() << "\t->\t" << size_t(id) << "\t:\t" << vertices.size()   << "\x1B[0m"<< std::endl;
        return vertices.size();
    }

    std::size_t send_vertices_to_one_tile(const std::map<Id, std::set<Vertex_const_handle>>& vertices) {
        std::size_t count = 0;
        for(auto& vi : vertices)
            count += send_vertices(vi.first, vi.second);
        return count;
    }

    void send_vertices_to_all_tiles(const std::vector<Vertex_const_handle>& vertices) {
        for(Vertex_const_handle v : vertices)
            if (!triangulation_.vertex_is_infinite(v))
                extreme_points_.emplace_back(triangulation_.vertex_id(v), triangulation_.point(v));
        // debug
        //if(!vertices.empty()) std::cout << "\x1B[33m" << id() << "\t->\t*\t:\t" << vertices.size()   << "\x1B[0m" << std::endl;
    }

    void receive_points(Points& received) {
        received.swap(points_[id()]);

        // debug
        //if(!received.empty()) std::cout << "\x1B[31m" << id() << "\t<-\t*\t:\t" << received.size()   << "\x1B[0m" << std::endl;

    }

    /// lock the tile for exclusive use (no unloading, no concurrent processing)
    bool locked;
    /// is the triangulation in memory ?
    bool in_mem;

    inline size_t number_of_main_facets  () const { return number_of_main_facets_;   }
    inline size_t number_of_main_cells   () const { return number_of_main_cells_;    }
    inline size_t number_of_main_finite_vertices() const { return number_of_main_finite_vertices_; }
    inline size_t number_of_main_finite_facets  () const { return number_of_main_finite_facets_;   }
    inline size_t number_of_main_finite_cells   () const { return number_of_main_finite_cells_;    }

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
        size_t number_of_main_finite_vertices = 0;
        size_t number_of_main_finite_facets = 0;
        size_t number_of_main_finite_cells = 0;
        size_t number_of_main_facets = 0;
        size_t number_of_main_cells = 0;
        if (in_mem) {
            if(!triangulation_.is_valid(verbose, level))
            {
                std::cerr << "DT Tile " << int(id()) << " is invalid" << std::endl;
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
    Traits traits;
    Id id_;
    Tile_triangulation triangulation_;
    Bbox bbox_;
    Points_map points_;
    Points extreme_points_;

    size_t number_of_main_finite_vertices_;
    size_t number_of_main_finite_facets_;
    size_t number_of_main_finite_cells_;
    size_t number_of_main_facets_;
    size_t number_of_main_cells_;
};


//if (r.first == id())
//    bbox_ += r.second;

}
}

#endif // CGAL_DDT_TILE_H
