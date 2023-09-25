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

#ifndef CGAL_DDT_KERNEL_TRAITS_H
#define CGAL_DDT_KERNEL_TRAITS_H

namespace CGAL {
namespace DDT {

template<typename P>
struct Kernel_traits {
    typedef P            Point;
    typedef Point const& Point_const_reference;
    typedef void         Bbox;
    static int           D;
};

template<typename Point>
inline void assign(Point& p, typename Kernel_traits<Point>::Point_const_reference q) {
    p = q;
}

template<typename Point_reference, typename InputIterator>
inline void assign(Point_reference p, InputIterator begin, InputIterator end) {
    std::copy(begin, end, p);
}

template<typename Point_const_reference>
double approximate_cartesian_coordinate(Point_const_reference p, int i) {
    return double(p[i]);
}

template<typename Point_const_reference>
bool less_coordinate(Point_const_reference p, Point_const_reference q, int i) {
    return p[i] < q[i];
}

template<typename Domain>
double measure(const Domain& d) {
    return d.measure();
}

template<typename Domain>
double intersection_measure(const Domain& x, const Domain& y) {
    return x.intersection_measure(y);
}

}
}

#endif // CGAL_DDT_KERNEL_TRAITS_H

