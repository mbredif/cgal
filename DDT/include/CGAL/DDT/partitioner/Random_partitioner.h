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

#ifndef CGAL_DDT_PARITIONER_RANDOM_PARTITIONER_H
#define CGAL_DDT_PARITIONER_RANDOM_PARTITIONER_H

#include <chrono>
#include <random>

namespace CGAL {
namespace DDT {

// \ingroup PkgDDTPartitionerClasses
// Affects points randomly to the range of tile indices [a, b].
// For testing purposes only (correct but bad performance)
// \cgalModels{Partitioner}
template<typename TileIndex, typename Triangulation, typename Generator = std::default_random_engine>
class Random_partitioner
{
    typedef CGAL::DDT::Triangulation_traits<Triangulation> Traits;
public:
    typedef TileIndex Tile_index;
    typedef typename Traits::Point Point;

    Random_partitioner(Tile_index a, Tile_index b, unsigned int seed = 0) : distribution(a,b), generator(seed)
    {
        if(seed == 0)
        {
            seed = std::chrono::system_clock::now().time_since_epoch().count();
            generator.seed(seed);
        }
    }

    Random_partitioner(Tile_index a, Tile_index b, const Generator& g) : distribution(a,b), generator(g)
    {
    }

    inline Tile_index operator()(Point_const_reference p)
    {
        return distribution(generator);
    }

    inline Tile_index a() const { return distribution.a(); }
    inline Tile_index b() const { return distribution.b(); }
    inline std::size_t size() const { return 1+b()-a(); }

private:
    std::uniform_int_distribution<Tile_index> distribution;
    Generator generator;
};

template<typename Traits>
std::ostream& operator<<(std::ostream& out, const Random_partitioner<Traits>& partitioner) {
    return out << "Random_partitioner( [ " << partitioner.a() << " , " << partitioner.b() << " ] )";
}

}
}

#endif // CGAL_DDT_PARITIONER_RANDOM_PARTITIONER_H
