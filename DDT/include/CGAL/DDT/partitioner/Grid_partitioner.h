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

#ifndef CGAL_DDT_PARITIONER_GRID_PARTITIONER_H
#define CGAL_DDT_PARITIONER_GRID_PARTITIONER_H

#include <iterator>

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTPartitionerClasses
/// \cgalModels Partitioner
template<typename Traits>
class Grid_partitioner
{
public:
    typedef typename Traits::Point Point;
    typedef typename Traits::Bbox Bbox;
    typedef typename Traits::Id    Id;
    typedef typename std::vector<std::size_t>::const_iterator const_iterator;

    template<typename Iterator>
    Grid_partitioner(const Bbox& bbox, Iterator it, Iterator end, Id id0 = {}) : id0(id0)
    {
        int D = bbox.dimension();
        std::size_t n = 1;
        M = 1;
        N.resize(D);
        inv_step.resize(D);
        origin.resize(D);
        for(int i=0; i<D; ++i)
        {
            if (it!=end) n = *it++;
            N[i] = n;
            inv_step[i] = n/(bbox.max(i)-bbox.min(i));
            origin[i] = bbox.min(i);
            M *= n;
        }
    }

    Grid_partitioner(const Bbox& bbox, std::size_t n)
    {
        std::size_t D = bbox.dimension();
        M = 1;
        N.resize(D);
        inv_step.resize(D);
        origin.resize(D);
        for(int i=0; i<D; ++i)
        {
            N[i] = n;
            inv_step[i] = n/(bbox.max(i)-bbox.min(i));
            origin[i] = bbox.min(i);
            M *= n;
        }
    }

    /// @todo : use a predicate, may be approximate (p[i] is a double approximation)
    Id operator()(const Point& p) const
    {
        std::size_t D = N.size();
        Id id = id0;
        for(std::size_t i=0; i<D; ++i)
        {
            id = id*N[i] + (Id((p[i]-origin[i])*inv_step[i]) % N[i]);
            /// @todo : check compare_x/y/z/d with neighbors to check approximation validity
        }
        return id;
    }
    const_iterator size_begin() const { return N.begin(); }
    const_iterator size_end() const { return N.end(); }
    std::size_t size() const { return M; }

private:
    std::size_t M;
    std::vector<std::size_t> N;
    std::vector<double> inv_step;
    std::vector<double> origin;
    Id id0;
};

template<typename Traits>
std::ostream& operator<<(std::ostream& out, const Grid_partitioner<Traits>& partitioner) {
    out << "Grid_partitioner( ";
    std::copy(partitioner.size_begin(), partitioner.size_end(), std::ostream_iterator<std::size_t>(out, " "));
    return out << ")";
}

}
}

#endif // CGAL_DDT_PARITIONER_GRID_PARTITIONER_H
