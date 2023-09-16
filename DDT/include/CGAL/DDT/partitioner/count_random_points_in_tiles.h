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
#include <CGAL/DDT/Tile_point_set.h>
#include <iterator>

namespace CGAL {
namespace DDT {


template<typename RandomPoints, typename Partitioner, typename OutputIterator>
OutputIterator count_random_points_in_tiles(RandomPoints& points, const Partitioner& part, OutputIterator out);

template<typename RandomPoint, typename TileIndex, typename Triangulation, typename OutputIterator>
OutputIterator count_random_points_in_tiles(
    const Random_point_set<RandomPoint>& points,
    const Grid_partitioner<TileIndex,Triangulation>& part,
    OutputIterator out)
{
    typedef Grid_partitioner<TileIndex,Triangulation> Partitioner;
    typedef typename Partitioner::Point               Point;
    typedef typename Partitioner::Bbox                Bbox;
    Bbox part_bbox = part.bbox();
    Bbox points_bbox = points.bbox();
    CGAL_assertion( part_bbox ==  points_bbox );
    for(TileIndex i = part.begin(); i < part.end(); ++i) {
        *out++ = {i, points.size()/part.size()};
    }
    return out;
}

}
}

#endif // CGAL_DDT_COUNT_RANDOM_POINTS_IN_TILES_H
