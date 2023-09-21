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

template<typename PointGenerator, typename Partitioner, typename OutputIterator>
OutputIterator count_random_points_in_tiles(
    const Random_point_set<PointGenerator>& points,
    const Partitioner& part,
    OutputIterator out)
{
    typedef Random_point_set<PointGenerator> PointSet;
    typedef typename PointGenerator::Domain  Domain;
    typedef typename Partitioner::Tile_index Tile_index;

    Domain domain = points.generator().domain();
    std::vector<std::size_t> counts(part.size(), 0);
    std::mt19937 gen(points.seed());
    std::size_t n_points = points.size();
    double M = measure(domain);
    bool contained = (M == intersection_measure(domain, part.domain()));
    for(Tile_index id = part.begin(); id < part.end() &&  0 < n_points; ++id) {
        double m = intersection_measure(domain, part.domain(id));
        std::binomial_distribution<std::size_t> distrib(n_points, m/M);
        std::size_t n = distrib(gen);
        // ensure the requested amount of points is reported if the partition domain contains the generator domain
        // just in case the last proba!=1 due to floating point approximations
        if(contained && id+1 == part.end()) n = n_points;
        M -= m;
        n_points -= n;
        if (n>0) *out++ = {id, n};
    }
    return out;
}
}
}

/// \ingroup PkgDDTFunctions
/// makes a distributed point set from point set uniformly generated in a bounding box and a grid partitioner
template<typename PointGenerator, typename Partitioner>
Distributed_point_set<
    DDT::Random_point_set<PointGenerator>,
    DDT::Partitioner_property_map<DDT::Random_point_set<PointGenerator>, Partitioner>
>
make_distributed_point_set(
    const DDT::Random_point_set<PointGenerator>& points,
    const Partitioner& partitioner)
{
    typedef DDT::Random_point_set<PointGenerator> Point_set;
    typedef DDT::Partitioner_property_map<Point_set, Partitioner> PropertyMap;
    typedef Distributed_point_set<Point_set, PropertyMap>         Distributed_point_set;
    typedef typename Partitioner::Tile_index                      Tile_index;

    Distributed_point_set dpoints(partitioner);
    std::vector<std::pair<Tile_index, std::size_t>> counts;
    DDT::Impl::count_random_points_in_tiles(points, partitioner, std::back_inserter(counts));
    std::hash<Tile_index> hash;
    for(auto c : counts)
    {
        if (c.second == 0) continue; // skip empty tiles
        Tile_index id = c.first;
        unsigned int seed = points.seed() + hash(id);
        dpoints.try_emplace(id, c.second, partitioner.bbox(id), seed);
    }
    return std::move(dpoints);
}

}

#endif // CGAL_MAKE_DISTRIBUTED_POINT_SET_H
