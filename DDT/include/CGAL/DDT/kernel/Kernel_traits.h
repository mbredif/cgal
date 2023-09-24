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
    typedef P Point;
    typedef const Point&                               Point_const_reference;
    typedef void Bbox;
    static int D;

    template<typename InputIterator>
    Point make_point(InputIterator begin, InputIterator end);

    Point make_point(int dim);

    Bbox make_bbox(int dim);
};

template<typename Point_const_reference>
bool less_coordinate(Point_const_reference p, Point_const_reference q, int i);

template<typename Point_const_reference>
double approximate_cartesian_coordinate(Point_const_reference p, int i);

template<typename Point_const_reference>
typename Kernel_traits<Point_const_reference>::Bbox make_bbox(Point_const_reference p);

template<typename Point_const_reference>
typename Kernel_traits<Point_const_reference>::Bbox make_bbox(Point_const_reference p, Point_const_reference q);

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

