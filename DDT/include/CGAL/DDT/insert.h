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
        tri.insert(it->second, inserted, true);
        it->second.clear();
        // TODO: For now, all points are inserted, disregarding memory constraints
        // if some remain, reschedule them : *out++ = { it->first, std::move(it->second) };
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


template<typename InputIterator, typename TileTriangulation, typename OutputIterator>
OutputIterator splay_root_triangulation(TileTriangulation& tri, InputIterator begin, InputIterator end, OutputIterator out)
{
    typedef typename TileTriangulation::Tile_index   TileIndex;
    typedef typename TileTriangulation::Vertex_index Vertex_index;
    std::vector<Vertex_index> inserted;
    for(InputIterator it = begin; it != end; ++it) {
        for(auto& p: it->second)
            inserted.push_back(tri.insert(p.second, p.first).first);
        it->second.clear();
    }
    std::map<TileIndex, std::set<Vertex_index>> vertices;
    tri.get_finite_neighbors(inserted, vertices);
    for(auto& vi : vertices)
    {
        typename InputIterator::value_type::second_type points;
        for(auto v : vi.second)
            points.emplace_back(tri.vertex_id(v), tri.point(v));
        *out++ = { vi.first, std::move(points) };
    }
    return out;
}

template<typename TriangulationContainer, typename PointSetContainer, typename OutputIterator, typename TileIndex, typename Scheduler>
OutputIterator
insert_and_get_axis_extreme_points(TriangulationContainer& tiles, PointSetContainer& point_sets, OutputIterator out, TileIndex root, Scheduler& sch, int dim)
{
    typedef typename TriangulationContainer::mapped_type TileTriangulation;
    typedef typename PointSetContainer::value_type PointSetContainerValue;
    return sch.template ranges_transform<PointSetContainerValue>(point_sets, tiles, [&root](auto first, auto last, TileTriangulation& tri, auto out_)
    {
        typedef typename TileTriangulation::Vertex_index Vertex_index;
        typedef typename PointSetContainerValue::second_type PointSet;
        out_ = splay_tile(tri, first, last, out_);
        // send the extreme points along each axis to all tiles to initialize the star splaying
        std::vector<Vertex_index> vertices;
        tri.get_axis_extreme_points(vertices);
        PointSet ps;
        for(auto v : vertices)
            ps.emplace_back(tri.vertex_id(v), tri.point(v));
        *out_++ = { root, std::move(ps) };
        return out_;
    }, out, dim);
}

template<typename TriangulationContainer, typename PointSetContainer, typename TileIndex, typename Scheduler>
void splay_stars(TriangulationContainer& tiles, PointSetContainer& points, Scheduler& sch, TileIndex root, int dim)
{
    typedef typename TriangulationContainer::mapped_type TileTriangulation;
    sch.ranges_for_each(points, tiles,
        [&root](auto first, auto last, TileTriangulation& tri, auto out) {
            if (tri.id() == root) {
                return splay_root_triangulation(tri, first, last, out);
            } else {
                return splay_tile(tri, first, last, out);
            }
        },
        dim);
}

} // namespace impl

}
}

#endif // CGAL_DDT_INSERT_H
