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

    template<typename Iterator>
    Grid_partitioner(const Bbox& bbox, Iterator it, Iterator end)
    {
        int D = bbox.dimension();
        int n = 1;
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

    Grid_partitioner(const Bbox& bbox, int n)
    {
        int D = bbox.dimension();
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
        int D = N.size();
        int id = 0;
        for(int i=0; i<D; ++i)
        {
            id = id*N[i] + (int((p[i]-origin[i])*inv_step[i]) % N[i]);
            /// @todo : check compare_x/y/z/d with neighbors to check approximation validity
        }
        return id;
    }
    typename std::vector<Id>::const_iterator begin() const { return N.begin(); }
    typename std::vector<Id>::const_iterator end() const { return N.end(); }
    size_t size() const { return M; }

private:
    size_t M;
    std::vector<Id> N;
    std::vector<double> inv_step;
    std::vector<double> origin;
};

}
}

#endif // CGAL_DDT_PARITIONER_GRID_PARTITIONER_H
