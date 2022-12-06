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

#ifndef CGAL_DDT_SCHEDULER_SEQUENTIAL_SCHEDULER_H
#define CGAL_DDT_SCHEDULER_SEQUENTIAL_SCHEDULER_H

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTSchedulerClasses
/// \cgalModels Scheduler
template<typename T>
struct Sequential_scheduler
{
    typedef T Tile;
    typedef typename Tile::Id Id;

    /// constructor
    Sequential_scheduler(int max_concurrency = 0) { assert(max_concurrency==0 || max_concurrency==1); }

    inline int max_concurrency() const { return 1; }

    template<typename TileContainer, typename UnaryOp, typename V = int, typename BinaryOp = std::plus<>>
    V for_each(TileContainer& tc, UnaryOp op1, BinaryOp op2 = {}, V init = {})
    {
        V value = init;
        for(auto it = tc.tile_ids_begin(); it != tc.tile_ids_end(); ++it) {
            auto tile = tc.load(*it);
            value = op2(value, op1(tc, *tile));
            tc.unload(tile);
        }
        return value;
    }

    template<typename TileContainer, typename UnaryOp, typename V = int, typename BinaryOp = std::plus<>>
    V for_each_rec(TileContainer& tc, UnaryOp op1, BinaryOp op2 = {}, V init = {})
    {
        V value = init, v;
        do {
            v = for_each(tc, op1, op2, init);
            value = op2(value, v);
        } while (v != init);
        return value;
    }
};

}
}

#endif // CGAL_DDT_SCHEDULER_SEQUENTIAL_SCHEDULER_H
