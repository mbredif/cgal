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

private:
    Points_map points_;
    std::vector<Tile_points> input_points_;
    Points extreme_points_;
};

template<class Tile_index, typename Messaging>
struct Messaging_container {
    typedef std::map<Tile_index, Messaging> Container;
    typedef typename Container::iterator iterator;
    typedef typename Container::value_type value_type;
    typedef typename Container::mapped_type mapped_type;
    typedef typename Container::key_type key_type;
    typedef typename Messaging::Points Points;

    mapped_type& operator[](key_type key) { return messagings[key]; }
    iterator begin  () { return messagings.begin (); }
    iterator end    () { return messagings.end   (); }

    void send_points(Tile_index id) {
        Messaging& msg = messagings[id];
        for(auto& p : msg.points()) {
            if(p.first != id) {
                auto& points = messagings[p.first].points()[p.first];
                points.insert(points.end(), p.second.begin(), p.second.end());
                p.second.clear();
            }
        }
        for(auto& messaging : messagings) {
            Points& p = messaging.second.points()[messaging.first];
            p.insert(p.end(), msg.extreme_points().begin(), msg.extreme_points().end());
        }
        extreme_points_.insert(extreme_points_.end(), msg.extreme_points().begin(), msg.extreme_points().end());
        msg.extreme_points().clear();
    }

    private:
    std::map<Tile_index, Messaging> messagings;
    Points extreme_points_;
};


/// \ingroup PkgDDTClasses
/// \tparam T is a model of the TriangulationTraits concept
/// The Tile stores a local Delaunay triangulation.
template<class Triangulation, class TileIndexProperty>
class Tile
{
public:
    typedef Triangulation_traits<Triangulation>       Traits;
    typedef typename TileIndexProperty::value_type    Tile_index;
    typedef typename Traits::Bbox                     Bbox;
    typedef CGAL::DDT::Tile_triangulation<Triangulation, TileIndexProperty>          Tile_triangulation;

    Tile(Tile_index id, int dimension) :
        id_(id),
        triangulation_(id, dimension),
        bbox_(Traits::bbox(dimension)),
        in_mem(false),
        locked(false)
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

    bool is_valid(bool verbose = false, int level = 0) const
    {
        return triangulation_.is_valid(verbose, level);
    }

private:
    Tile_index id_;
    Tile_triangulation triangulation_;
    Bbox bbox_;
};

}
}

#endif // CGAL_DDT_TILE_H
