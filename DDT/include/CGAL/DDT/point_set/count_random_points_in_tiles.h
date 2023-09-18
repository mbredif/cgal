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

#ifndef CGAL_DDT_COUNT_RANDOM_POINTS_IN_TILES_H
#define CGAL_DDT_COUNT_RANDOM_POINTS_IN_TILES_H

#include <CGAL/DDT/partitioner/Grid_partitioner.h>
#include <CGAL/DDT/point_set/Random_points_in_bbox.h>
#include <CGAL/assertions.h>

namespace CGAL {
namespace DDT {


template<typename RandomPoints, typename Partitioner, typename OutputIterator>
OutputIterator count_random_points_in_tiles(RandomPoints& points, const Partitioner& part, OutputIterator out);

template<typename Point, typename TileIndex, typename Triangulation, typename OutputIterator>
OutputIterator count_random_points_in_tiles(
    const Random_point_set<Uniform_point_in_bbox<Point>>& points,
    const Grid_partitioner<TileIndex,Triangulation>& part,
    OutputIterator out)
{
    typedef Grid_partitioner<TileIndex,Triangulation> Partitioner;
    typedef typename Partitioner::Bbox                Bbox;
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

#endif // CGAL_DDT_COUNT_RANDOM_POINTS_IN_TILES_H
