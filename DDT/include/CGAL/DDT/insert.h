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
template<typename PointSet, typename TileTriangulation, typename InputIterator, typename PointIndices, typename OutputIterator>
OutputIterator splay_tile(TileTriangulation& tri, InputIterator first, InputIterator last, PointIndices& indices, OutputIterator out)
{
    typedef typename TileTriangulation::Tile_index   Tile_index;
    typedef typename TileTriangulation::Vertex_index Vertex_index;
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
        PointSet points(vi.first, indices);
        for(auto v : vi.second)
            points.insert(tri.point(v), tri.vertex_id(v));
        *out++ = { vi.first, std::move(points) };
    }
    return out;
}


template<typename PointSet, typename PointIndices, typename TileTriangulation, typename InputIterator, typename OutputIterator>
OutputIterator splay_root_triangulation(TileTriangulation& tri, InputIterator begin, InputIterator end, PointIndices indices, OutputIterator out)
{
    typedef typename std::iterator_traits<InputIterator>::value_type value_type;
    typedef typename value_type::second_type                         Pointset;
    typedef typename TileTriangulation::Tile_index                   Tile_index;
    typedef typename TileTriangulation::Vertex_index                 Vertex_index;
    typedef typename Pointset::Vertex_index                          Point_index;
    typedef typename Pointset::Point                                 Point;
    std::vector<Vertex_index> inserted;
    for(InputIterator it = begin; it != end; ++it) {
        auto& ps = it->second;
        for(auto v = ps.begin(); v != ps.end(); ++v) {
            Tile_index id = ps.point_id(v);
            const Point& p = ps.point(v);
            inserted.push_back(tri.insert(p, id).first);
        }
        ps.clear();
    }
    std::map<Tile_index, std::set<Vertex_index>> vertices;
    tri.get_finite_neighbors(inserted, vertices);
    for(auto& vi : vertices)
    {
        PointSet points(vi.first, indices);
        for(auto v : vi.second)
            points.insert(tri.point(v), tri.vertex_id(v));
        *out++ = { vi.first, std::move(points) };
    }
    return out;
}

template<
    typename TriangulationContainer, typename TriangulationIndices,
    typename PointsetContainer1,
    typename PointsetContainer2, typename PointIndices2,
    typename TileIndex, typename Scheduler>
void splay_stars(
    TriangulationContainer& triangulations, TriangulationIndices triangulation_indices,
    PointsetContainer1& points1,
    PointsetContainer2& points2, PointIndices2 pointset_indices2,
    Scheduler& sch, TileIndex root, int dim)
{
    typedef typename TriangulationContainer::mapped_type TileTriangulation;
    typedef typename TileTriangulation::Vertex_index Vertex_index;
    typedef typename PointsetContainer2::value_type PointSetContainerValue2;
    typedef typename PointSetContainerValue2::second_type PointSet2;
    sch.ranges_for_each(points1, triangulations, points2,
        [&root, &pointset_indices2](auto first, auto last, TileTriangulation& tri, auto out) {
            if (tri.id() == root) {
               return splay_root_triangulation<PointSet2>(tri, first, last, pointset_indices2, out);

            } else {
                bool empty = tri.number_of_vertices() == 0;
                out = splay_tile<PointSet2>(tri, first, last, pointset_indices2, out);
                if(empty) {
                    // send the extreme points along each axis to the root tile to initialize the star splaying
                    std::vector<Vertex_index> vertices;
                    tri.get_axis_extreme_points(vertices);
                    PointSet2 ps(root, pointset_indices2);
                    for(auto v : vertices)
                        ps.insert(tri.point(v), tri.vertex_id(v));
                    *out++ = { root, std::move(ps) };
                }
                return out;
            }
        },
        dim, triangulation_indices);
}

} // namespace impl

}
}

#endif // CGAL_DDT_INSERT_H
