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

/// \ingroup PkgDDTRef
/// \cgalModels Partitioner
/// For testing purposes only (correct but bad performance)
template<typename Traits, typename Generator = std::default_random_engine>
class random_partitioner
{
public:
    typedef typename Traits::Point Point;
    typedef typename Traits::Id    Id;

    random_partitioner(Id a, Id b, unsigned int seed = 0) : distribution(a,b), generator(seed)
    {
        if(seed == 0)
        {
            seed = std::chrono::system_clock::now().time_since_epoch().count();
            generator.seed(seed);
        }
    }

    random_partitioner(Id a, Id b, const Generator& g) : distribution(a,b), generator(g)
    {
    }

    inline Id operator()(const Point& p)
    {
        return distribution(generator);
    }

private:
    std::uniform_int_distribution<Id> distribution;
    Generator generator;
};

}
}

#endif // CGAL_DDT_PARITIONER_RANDOM_PARTITIONER_H
