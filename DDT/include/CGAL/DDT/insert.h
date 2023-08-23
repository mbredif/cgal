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

#ifndef CGAL_DDT_INSERT_H
#define CGAL_DDT_INSERT_H

#include <vector>
#include <set>
#include <map>

#if __cplusplus >= 202002L
# include <ranges> // c++20 for std::views::counted
#endif

namespace CGAL {
namespace DDT {
namespace impl {

// inserts as many points as possible from received into tri
// on exit, received contains the uninserted points
template<typename Index, typename TileTriangulation, typename PointSet, typename OutputIterator>
std::pair<std::size_t, OutputIterator> splay_tile(Index id, TileTriangulation& tri, PointSet& received, OutputIterator out)
{
    typedef typename TileTriangulation::Tile_index   Tile_index;
    typedef typename TileTriangulation::Vertex_index Vertex_index;
    if (received.empty()) return { 0, out };

    // insert them into the current tile triangulation and get the new foreign points
    std::set<Vertex_index> inserted;
    tri.insert(received, inserted, true);
    // TODO: For now, all points are inserted, disregarding memory constraints
    received.clear();
    if (inserted.empty()) return { 0, out };

    // get the relevant neighbor points
    std::map<Tile_index, std::set<Vertex_index>> vertices;
    tri.get_finite_neighbors(inserted, vertices);

    // send them to the relevant neighboring tiles
    for(auto& vi : vertices)
    {
        PointSet res;
        for(auto v : vi.second)
            res.emplace_back(tri.vertex_id(v), tri.point(v));
        *out++ = { vi.first, res };
    }
    if(!received.empty()) {
        *out++ = { id, std::move(received) };
    }

    return { inserted.size(), out };
}

template<typename TileContainer, typename PointSetContainer, typename OutputIterator, typename Scheduler>
std::pair<std::size_t, OutputIterator>
insert_and_get_axis_extreme_points(TileContainer& tiles, PointSetContainer& point_sets, OutputIterator out, Scheduler& sch, int dim)
{
    typedef typename TileContainer::key_type         Index;
    typedef typename TileContainer::mapped_type      Triangulation;
    typedef typename PointSetContainer::mapped_type  PointSet;
    return sch.join_transform_reduce(tiles, point_sets, out, 0, std::plus<>(), [](Index id, Triangulation& tri, PointSet& point_set, OutputIterator out)
    {
        typedef typename Triangulation::Vertex_index Vertex_index;
        auto res = splay_tile(id, tri, point_set, out);
        // send the extreme points along each axis to all tiles to initialize the star splaying
        std::vector<Vertex_index> vertices;
        tri.get_axis_extreme_points(vertices);
        for(auto v : vertices)
            point_set.emplace_back(tri.vertex_id(v), tri.point(v));
        return res;
    }, dim);
}

template<typename TileContainer, typename PointSetContainer, typename Scheduler>
std::size_t splay_stars(TileContainer& tiles, PointSetContainer& point_sets, Scheduler& sch, int dim)
{
    typedef typename TileContainer::key_type             Index;
    typedef typename TileContainer::mapped_type          Triangulation;
    typedef typename PointSetContainer::mapped_type      PointSet;
    typedef std::back_insert_iterator<PointSetContainer> OutputIterator;
    return sch.join_transform_reduce_loop(tiles, point_sets,
        std::back_inserter(point_sets), 0, std::plus<>(), &splay_tile<Index, Triangulation, PointSet, OutputIterator>, dim);
}

template<typename TileTriangulation, typename PointSetContainer>
std::size_t splay_root_triangulation(TileTriangulation& tri, PointSetContainer& input_points, PointSetContainer& output_points)
{
    typedef typename PointSetContainer::key_type     Tile_index;
    typedef typename PointSetContainer::mapped_type  PointSet;
    typedef typename TileTriangulation::Vertex_index Vertex_index;
    std::vector<Vertex_index> inserted;
    for(auto& ps: input_points)
        for(auto& p: ps.second)
            inserted.push_back(tri.insert(p.second, p.first).first);
    std::map<Tile_index, std::set<Vertex_index>> vertices;
    tri.get_finite_neighbors(inserted, vertices);
    for(auto& vi : vertices)
    {
        PointSet points;
        for(auto v : vi.second)
            points.emplace_back(tri.vertex_id(v), tri.point(v));
        output_points.push_back(std::make_pair(vi.first, std::move(points)));
    }
    return inserted.size();
}

} // namespace impl

}
}

#endif // CGAL_DDT_INSERT_H
