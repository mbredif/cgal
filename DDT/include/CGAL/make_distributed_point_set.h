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

#ifndef CGAL_MAKE_DISTRIBUTED_POINT_SET_H
#define CGAL_MAKE_DISTRIBUTED_POINT_SET_H

#include <CGAL/Distributed_point_set.h>
#include <CGAL/DDT/partitioner/Grid_partitioner.h>
#include <CGAL/DDT/point_set/Random_points_in_bbox.h>
#include <CGAL/DDT/property_map/Partitioner_property_map.h>
#include <CGAL/assertions.h>

namespace CGAL {
namespace DDT {

namespace Impl {
template<typename TileIndex, typename Point, typename OutputIterator>
OutputIterator count_random_points_in_tiles(
    const Random_point_set<Uniform_point_in_bbox<Point>>& points,
    const Grid_partitioner<TileIndex,Point>& part,
    OutputIterator out)
{
    typedef Grid_partitioner<TileIndex,Point> Partitioner;
    typedef typename Partitioner::Bbox        Bbox;
    Bbox part_bbox = part.bbox();
    Bbox points_bbox = points.generator().bbox();
    CGAL_assertion( part_bbox == points_bbox );
    std::vector<std::size_t> counts(part.size(), 0);
    std::mt19937 gen(points.seed());
    std::size_t n_tiles = part.size();
    std::size_t n_points = points.size();
    for(TileIndex id = part.begin(); id < part.end(); ++id, --n_tiles) {
        std::size_t n = 0;
        if (n_points > 0) {
            std::binomial_distribution<std::size_t> distrib(n_points, 1./n_tiles);
            n = distrib(gen);
        }

        n_points -= n;
        *out++ = {id, n};
    }

    return out;
}
}
}

template<typename TileIndex, typename Point>
auto make_distributed_point_set(
    const DDT::Random_point_set<DDT::Uniform_point_in_bbox<Point>>& points,
    const DDT::Grid_partitioner<TileIndex,Point>& partitioner)
{
    typedef DDT::Random_point_set<DDT::Uniform_point_in_bbox<Point>> Point_set;
    typedef DDT::Grid_partitioner<TileIndex, Point>                  Partitioner;
    typedef DDT::Partitioner_property_map<Point_set, Partitioner>    PropertyMap;
    typedef Distributed_point_set<Point_set, PropertyMap>            Distributed_point_set;

    Distributed_point_set dpoints(partitioner);
    std::vector<std::pair<TileIndex, std::size_t>> counts;
    DDT::Impl::count_random_points_in_tiles(points, partitioner, std::back_inserter(counts));
    for(auto c : counts)
    {
        TileIndex id = c.first;
        unsigned int seed = points.seed() + std::hash<TileIndex>{}(id);
        dpoints.try_emplace(id, c.second, partitioner.bbox(id), seed);
    }
    return std::move(dpoints);
}

}

#endif // CGAL_MAKE_DISTRIBUTED_POINT_SET_H
