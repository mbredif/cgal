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
    tri.insert(first, last, inserted);
    if (inserted.empty()) return out;

    // get the relevant neighbor points
    std::map<Tile_index, std::set<Vertex_index>> vertices;
    tri.get_finite_neighbors(inserted, vertices);

    // send them to the relevant neighboring tiles
    for(auto& vi : vertices)
    {
        PointSet points(vi.first, indices);
        for(auto v : vi.second)
            points.insert_point(tri.triangulation_point(v), tri.vertex_id(v));
        if(!points.empty())
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
    std::vector<Vertex_index> inserted;
    for(InputIterator it = begin; it != end; ++it) {
        auto& ps = it->second;
        for(auto v = ps.begin(); v != ps.end(); ++v)
            inserted.push_back(tri.insert(point(ps.point_set(), v), ps.point_id(v)).first);
    }
    std::map<Tile_index, std::set<Vertex_index>> vertices;
    tri.get_finite_neighbors(inserted, vertices);
    for(auto& vi : vertices)
    {
        PointSet points(vi.first, indices);
        for(auto v : vi.second)
            points.insert_point(tri.triangulation_point(v), tri.vertex_id(v));
        if (!points.empty())
            *out++ = { vi.first, std::move(points) };
    }
    return out;
}

template<typename PointSet2, typename PointIndices2, typename TileIndex>
struct Star_splayer {
    Star_splayer(PointIndices2 pointset_indices2, TileIndex root) : pointset_indices2(pointset_indices2), root(root) {}

    TileIndex root;
    PointIndices2 pointset_indices2;

    template<typename PointSetIterator, typename TileTriangulation, typename OutputIterator>
    OutputIterator operator() (PointSetIterator first, PointSetIterator last, TileTriangulation& tri, OutputIterator out) {
        typedef typename TileTriangulation::Vertex_index Vertex_index;
        if (tri.id() == root) {
           return splay_root_triangulation<PointSet2>(tri, first, last, pointset_indices2, out);

        } else {
            bool empty = tri.number_of_vertices() == 0;
            out = splay_tile<PointSet2>(tri, first, last, pointset_indices2, out);
            if(empty) {
                // send the extreme points along each axis to the root tile to initialize the star splaying
                std::vector<Vertex_index> vertices;
                tri.get_axis_extreme_points(vertices);
                PointSet2 points(root, pointset_indices2);
                for(auto v : vertices)
                    points.insert_point(tri.triangulation_point(v), tri.vertex_id(v));
                if (!points.empty())
                    *out++ = { root, std::move(points) };
            }
            return out;
        }
    }
};

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
    typedef typename PointsetContainer2::value_type PointSetContainerValue2;
    typedef typename PointSetContainerValue2::second_type PointSet2;
    Star_splayer<PointSet2, PointIndices2, TileIndex> star_splayer(pointset_indices2, root);
    sch.ranges_for_each(points1, triangulations, points2, star_splayer, dim, triangulation_indices);
}

} // namespace impl

}
}

#endif // CGAL_DDT_INSERT_H
