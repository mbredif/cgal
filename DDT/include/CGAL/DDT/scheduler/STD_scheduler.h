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

#ifndef CGAL_DDT_SCHEDULER_STD_SCHEDULER_H
#define CGAL_DDT_SCHEDULER_STD_SCHEDULER_H

#if __cplusplus >= 201703L
#include <execution>
#define CGAL_DDT_SCHEDULER_STD_SCHEDULER_PAR policy,
#else
#define CGAL_DDT_SCHEDULER_STD_SCHEDULER_PAR
#endif
#include <CGAL/DDT/Tile.h>

namespace CGAL {
namespace DDT {

// \ingroup PkgDDTSchedulerClasses
// \cgalModels Scheduler
#if __cplusplus >= 201703L
template<typename ExecutionPolicy
>
#endif
struct STD_scheduler
{

    STD_scheduler(int max_concurrency = 0) {}

    inline int max_concurrency() const { return 0; }

    template<typename TileContainer,
         typename Transform,
         typename Reduce = std::plus<>,
         typename Tile = typename TileContainer::Tile,
         typename V = std::invoke_result_t<Reduce,
                                           std::invoke_result_t<Transform, Tile&>,
                                           std::invoke_result_t<Transform, Tile&> > >
    V for_each(TileContainer& tc, Transform transform, Reduce reduce = {}, V init = {})
    {
        return std::transform_reduce(CGAL_DDT_SCHEDULER_STD_SCHEDULER_PAR
                                     tc.begin(), tc.end(), init, reduce, [&tc, &transform, &init](Tile& tile){
            tile.locked = true;
            V value = init;
            if (tc.load(tile)) value = transform(tile);
            tile.locked = false;
            return value;
        } );
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
    V for_each_zip(TileContainer& tc, MessagingContainer& messagings, Transform transform, Reduce reduce = {}, V init = {})
    {
        typedef typename MessagingContainer::value_type value_type;
        return std::transform_reduce(CGAL_DDT_SCHEDULER_STD_SCHEDULER_PAR
                                     messagings.begin(), messagings.end(), init, reduce, [&tc, &messagings, &transform, &init](value_type& messaging){
            Tile& tile = tc[messaging.first];
            tile.locked = true;
            V value = init;
            if (tc.load(tile)) value = transform(tile, messaging.second);
            messagings.send_points(messaging.first);
            tile.locked = false;
            return value;
        } );
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
    V for_each_rec(TileContainer& tc, MessagingContainer& messagings, Transform transform, Reduce reduce = {}, V init = {})
    {
        V value = init, v;
        do {
            v = for_each_zip(tc, messagings, transform, reduce, init);
            value = reduce(value, v);
        } while (v != init);
        return value;
    }
#if __cplusplus >= 201703L
    ExecutionPolicy policy;
#endif
};

#if __cplusplus >= 201703L
using STD_scheduler_par = CGAL::DDT::STD_scheduler<std::execution::parallel_policy>;
using STD_scheduler_seq = CGAL::DDT::STD_scheduler<std::execution::sequenced_policy>;
#else
using STD_scheduler_seq = CGAL::DDT::STD_scheduler;
#endif

}
}

#endif // CGAL_DDT_SCHEDULER_SEQUENTIAL_SCHEDULER_H
