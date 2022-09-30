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

#include <CGAL/DDT/bbox.h>

namespace ddt
{

template<typename Traits>
class grid_partitioner
{
public:
    typedef typename Traits::Point Point;
    typedef typename Traits::Id    Id;
    enum { D = Traits::D };

    template<typename Iterator>
    grid_partitioner(const Bbox<D, double>& bbox, Iterator it, Iterator end)
    {
        M = 1;
        int n = 1;
        for(int i=0; i<D; ++i)
        {
            if (it!=end) n = *it++;
            N[i] = n;
            M *= n;
            inv_step[i] = n/(bbox.max(i)-bbox.min(i));
            origin[i] = bbox.min(i);
        }
    }

    grid_partitioner(const Bbox<D, double>& bbox, int n)
    {
        M = 1;
        for(int i=0; i<D; ++i)
        {
            N[i] = n;
            M *= n;
            inv_step[i] = n/(bbox.max(i)-bbox.min(i));
            origin[i] = bbox.min(i);
        }
    }

    /// @todo : use a predicate, may be approximate (p[i] is a double approximation)
    Id operator()(const Point& p) const
    {
        int id = 0;
        for(int i=0; i<D; ++i)
        {
            id = id*N[i] + (int((p[i]-origin[i])*inv_step[i]) % N[i]);
            /// @todo : check compare_x/y/z/d with neighbors to check approximation validity
        }
        return id;
    }
    const int *begin() const { return N; }
    const int *end() const { return N+D; }
    int size() const { return M; }

private:
    int N[D], M;
    double inv_step[D];
    double origin[D];
};

}

#endif // CGAL_DDT_PARITIONER_GRID_PARTITIONER_H
