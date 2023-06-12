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

#ifndef CGAL_DDT_MESSAGING_H
#define CGAL_DDT_MESSAGING_H

#include <vector>
#include <map>
#include <CGAL/DDT/tile_points/No_tile_points.h>

namespace CGAL {
namespace DDT {

template<class TileIndex, class Point, class TilePoints = No_tile_points>
struct Messaging {
    typedef TileIndex                    Tile_index;
    typedef std::vector<std::pair<Tile_index, Point>>  Points;
    typedef std::map<Tile_index, Points> Points_map;
    typedef TilePoints                   Tile_points;

    Messaging() : points_() {}

    const Points_map& points() const { return points_; }
    Points_map& points() { return points_; }

    const Points& extreme_points() const { return extreme_points_; }
    Points& extreme_points() { return extreme_points_; }

    void send_point(Tile_index id, Tile_index i, const Point& p) {
        points_[id].push_back({i,p});
    }

    template<typename TileTriangulation, typename VertexIndex>
    void send_vertex(Tile_index id, const TileTriangulation& t, VertexIndex v) {
        points_[id].emplace_back(t.vertex_id(v), t.point(v));
    }

    template<typename TileTriangulation, typename VertexIndex>
    std::size_t send_vertices(Tile_index id, const TileTriangulation& t, const std::set<VertexIndex>& vertices) {
        Points& p = points_[id];
        for(VertexIndex v : vertices)
            p.emplace_back(t.vertex_id(v), t.point(v));
        // debug
        //if(!vertices.empty()) std::cout << "\x1B[32m" << id() << "\t->\t" << std::to_string(id) << "\t:\t" << vertices.size()   << "\x1B[0m"<< std::endl;
        return vertices.size();
    }

    template<typename TileTriangulation, typename VertexIndex>
    std::size_t send_vertices_to_one_tile(const TileTriangulation& t, const std::map<Tile_index, std::set<VertexIndex>>& vertices) {
        std::size_t count = 0;
        for(auto& vi : vertices)
            count += send_vertices(vi.first, t, vi.second);
        return count;
    }

    template<typename TileTriangulation, typename VertexIndex>
    void send_vertices_to_all_tiles(const TileTriangulation& t, const std::vector<VertexIndex>& vertices) {
        for(VertexIndex v : vertices)
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

}
}

#endif // CGAL_DDT_MESSAGING_H
