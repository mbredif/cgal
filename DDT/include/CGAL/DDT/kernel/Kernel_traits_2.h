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

    /// Point const reference type
    typedef Point const& Point_const_reference;

    /// Model of Point_set with embedded TileIndex
    template<typename TileIndex> using Point_set_with_id = std::vector<std::pair<TileIndex, Point>>;
};

template<typename K, typename InputIterator>
inline void assign(CGAL::Point_2<K>& p, InputIterator begin, InputIterator end) {
    InputIterator x = begin++;
    InputIterator y = begin;
    CGAL_assertion(++begin == end);
    p = { *x, *y };
}

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

inline void assign(CGAL::Bbox_2& b, int dim) {
    CGAL_assertion(dim == 2);
    b = {};
}

template<typename K>
inline void assign(CGAL::Bbox_2& b, const CGAL::Point_2<K>& p) {
    b = { p.x(), p.y(), p.x(), p.y() };
}

template<typename K>
inline void assign(CGAL::Bbox_2& b, const CGAL::Point_2<K>& p, const CGAL::Point_2<K>& q) {
    b = { p.x(), p.y(), q.x(), q.y() };
}

template<typename InputIterator0, typename InputIterator1>
inline void assign(CGAL::Bbox_2& b, InputIterator0 begin0, InputIterator0 end0, InputIterator1 begin1, InputIterator1 end1) {
    InputIterator0 x0 = begin0++;
    InputIterator0 y0 = begin0;
    InputIterator1 x1 = begin1++;
    InputIterator1 y1 = begin1;
    CGAL_assertion(++begin0 == end0 && ++begin1 == end1);
    b = { *x0, *y0, *x1, *y1 };
}

double measure(const CGAL::Bbox_2& b) {
    double m = b.x_span();
    if (m <= 0) return 0;
    m*=b.y_span();
    if (m <= 0) return 0;
    return m;
}

double intersection_measure(const CGAL::Bbox_2& x, const CGAL::Bbox_2& y) {
    double result = 1;
    for(int i=0; i<2; ++i) {
        result *= std::min(x.max(i), y.max(i)) -
                  std::max(x.min(i), y.min(i));
        if (result <= 0) return 0;
    }
    return result;
}

}
}

#endif // CGAL_DDT_KERNEL_TRAITS_2_H

