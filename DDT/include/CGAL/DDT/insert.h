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
template<typename TileTriangulation, typename InputIterator, typename OutputIterator>
OutputIterator splay_tile(TileTriangulation& tri, InputIterator first, InputIterator last, OutputIterator out)
{
    typedef typename TileTriangulation::Tile_index   Tile_index;
    typedef typename TileTriangulation::Vertex_index Vertex_index;
    typedef typename InputIterator::value_type::second_type  PointSet;
    if (first == last) return out;

    // insert them into the current tile triangulation and get the new foreign points
    std::set<Vertex_index> inserted;
    for(InputIterator it = first; it != last; ++it) {
        // TODO: For now, all points are inserted, disregarding memory constraints
        tri.insert(it->second, inserted, true);
        it->second.clear();
        // if some remain, reschedule them !
        if(!it->second.empty())
            *out++ = { it->first, std::move(it->second) };
    }
    if (inserted.empty()) return out;

    // get the relevant neighbor points
    std::map<Tile_index, std::set<Vertex_index>> vertices;
    tri.get_finite_neighbors(inserted, vertices);

    // send them to the relevant neighboring tiles
    for(auto& vi : vertices)
    {
        PointSet points;
        for(auto v : vi.second)
            points.emplace_back(tri.vertex_id(v), tri.point(v));
        *out++ = { vi.first, std::move(points) };
    }
    return out;
}

template<typename TriangulationContainer, typename PointSetContainer, typename OutputIterator, typename Scheduler>
OutputIterator
insert_and_get_axis_extreme_points(TriangulationContainer& tiles, PointSetContainer& point_sets, OutputIterator out, Scheduler& sch, int dim)
{
    typedef typename TriangulationContainer::mapped_type TileTriangulation;
    typedef typename PointSetContainer::value_type PointSetContainerValue;
    return sch.template ranges_transform<PointSetContainerValue>(point_sets, tiles, [](auto first, auto last, TileTriangulation& tri, auto out_)
    {
        typedef typename TileTriangulation::Vertex_index Vertex_index;
        out_ = splay_tile(tri, first, last, out_);
        // send the extreme points along each axis to all tiles to initialize the star splaying
        std::vector<Vertex_index> vertices;
        tri.get_axis_extreme_points(vertices);
        for(auto v : vertices)
            first->second.emplace_back(tri.vertex_id(v), tri.point(v));
        return out_;
    }, out, dim);
}

template<typename TriangulationContainer, typename PointSetContainer, typename Scheduler>
void splay_stars(TriangulationContainer& tiles, PointSetContainer& point_sets, Scheduler& sch, int dim)
{
    typedef typename TriangulationContainer::mapped_type TileTriangulation;
    sch.ranges_for_each(point_sets, tiles,
        [](auto first, auto last, TileTriangulation& tri, auto out) { return splay_tile(tri, first, last, out); },
        dim);
}

template<typename TileTriangulation, typename PointSetContainer, typename OutputIterator>
OutputIterator splay_root_triangulation(TileTriangulation& tri, PointSetContainer& input_points, OutputIterator out)
{
    typedef typename PointSetContainer::key_type     TileIndex;
    typedef typename PointSetContainer::mapped_type  PointSet;
    typedef typename TileTriangulation::Vertex_index Vertex_index;
    std::vector<Vertex_index> inserted;
    for(auto& ps: input_points)
        for(auto& p: ps.second)
            inserted.push_back(tri.insert(p.second, p.first).first);
    std::map<TileIndex, std::set<Vertex_index>> vertices;
    tri.get_finite_neighbors(inserted, vertices);
    for(auto& vi : vertices)
    {
        PointSet points;
        for(auto v : vi.second)
            points.emplace_back(tri.vertex_id(v), tri.point(v));
        *out++ = { vi.first, std::move(points) };
    }
    return out;
}

} // namespace impl

}
}

#endif // CGAL_DDT_INSERT_H
