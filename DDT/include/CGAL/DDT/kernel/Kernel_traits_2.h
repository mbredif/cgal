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

#ifndef CGAL_DDT_KERNEL_TRAITS_2_H
#define CGAL_DDT_KERNEL_TRAITS_2_H

#include <CGAL/DDT/kernel/Kernel_traits.h>
#include <CGAL/Bbox_2.h>
#include <CGAL/Point_2.h>

namespace CGAL {
namespace DDT {

template<typename K>
struct Kernel_traits<CGAL::Point_2<K>> {
    static constexpr int D = 2;

    typedef K Kernel;

    /// Bbox type
    typedef CGAL::Bbox_2 Bbox;

    /// Point type
    typedef CGAL::Point_2<K> Point;

    template<typename InputIterator>
    static inline Point point(InputIterator begin, InputIterator end) {
        CGAL_assertion(std::distance(begin, end) == 2);
        return Point(*begin, *(begin+1));
    }

    static inline Point point(int dim) {
        CGAL_assertion(dim == 2);
        return Point();
    }

    static inline Bbox bbox(int dim) {
        CGAL_assertion(dim == 2);
        return Bbox();
    }
};

template<typename K>
inline bool less_coordinate(const CGAL::Point_2<K>& p, const CGAL::Point_2<K>& q, int i) {
    CGAL_assertion(0 <= i && i < 2);
    return p[i] < q[i];
}

template<typename K>
inline double approximate_cartesian_coordinate(const CGAL::Point_2<K>& p, int i)
{
    CGAL_assertion(0 <= i && i < 2);
    return CGAL::to_double(p[i]);
}

template<typename K>
inline CGAL::Bbox_2 make_bbox(const CGAL::Point_2<K>& p) {
    return CGAL::Bbox_2(p.x(), p.y(), p.x(), p.y());
}

template<typename K>
inline CGAL::Bbox_2 make_bbox(const CGAL::Point_2<K>& p, const CGAL::Point_2<K>& q) {
    return CGAL::Bbox_2(p.x(), p.y(), q.x(), q.y());
}


}
}

#endif // CGAL_DDT_KERNEL_TRAITS_2_H

