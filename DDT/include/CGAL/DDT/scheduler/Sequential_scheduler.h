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

#include <assert.h>
#include <CGAL/DDT/Tile.h>

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTSchedulerClasses
/// \cgalModels Scheduler
struct Sequential_scheduler
{
    /// constructor
    Sequential_scheduler(int max_concurrency = 0) { assert(max_concurrency==0 || max_concurrency==1); }

    inline int max_concurrency() const { return 1; }

    template<typename TileContainer,
             typename Transform,
             typename Reduce = std::plus<>,
             typename Tile = typename TileContainer::Tile,
             typename V = std::invoke_result_t<Reduce,
                                               std::invoke_result_t<Transform, Tile&>,
                                               std::invoke_result_t<Transform, Tile&> > >
    V for_each(TileContainer& tiles, Transform transform, Reduce reduce = {}, V init = {})
    {
        V value = init;
        for(auto& [id, tile] : tiles) {
            tile.locked = true;
            if (tiles.load(id, tile)) value = reduce(value, transform(tile));
            tile.locked = false;
        }
        return value;
    }

    template<typename TileContainer,
             typename MessagingContainer,
             typename Transform,
             typename Reduce = std::plus<>,
             typename Tile = typename TileContainer::Tile,
             typename Messaging = typename MessagingContainer::mapped_type,
             typename V = std::invoke_result_t<Reduce,
                                               std::invoke_result_t<Transform, Tile&, Messaging&>,
                                               std::invoke_result_t<Transform, Tile&, Messaging&> > >
    V for_each_zip(TileContainer& tiles, MessagingContainer& messagings, Transform transform, Reduce reduce = {}, V init = {})
    {
        V value = init;
        for(auto& [id, messaging] : messagings) {
            Tile& tile = tiles.emplace(id).first->second;
            tile.locked = true;
            if (tiles.load(id, tile)) value = reduce(value, transform(tile, messaging));
            messagings.send_points(id);
            tile.locked = false;
        }
        return value;
    }

    template<typename TileContainer,
         typename MessagingContainer,
         typename Transform,
         typename Reduce = std::plus<>,
         typename Tile = typename TileContainer::Tile,
         typename Messaging = typename MessagingContainer::mapped_type,
         typename V = std::invoke_result_t<Reduce,
                                           std::invoke_result_t<Transform, Tile&, Messaging&>,
                                           std::invoke_result_t<Transform, Tile&, Messaging&> > >
    V for_each_rec(TileContainer& tiles, MessagingContainer& messagings, Transform transform, Reduce reduce = {}, V init = {})
    {
        V value = init, v;
        do {
            v = for_each_zip(tiles, messagings, transform, reduce, init);
            value = reduce(value, v);
        } while (v != init);
        return value;
    }
};

}
}

#endif // CGAL_DDT_SCHEDULER_SEQUENTIAL_SCHEDULER_H
