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

/// \ingroup PkgDDTPartitionerClasses
/// \cgalModels Partitioner
/// For testing purposes only (correct but bad performance)
template<typename Traits, typename Generator = std::default_random_engine>
class Random_partitioner
{
public:
    typedef typename Traits::Point Point;
    typedef typename Traits::Tile_index    Tile_index;

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

    inline Tile_index operator()(const Point& p)
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
