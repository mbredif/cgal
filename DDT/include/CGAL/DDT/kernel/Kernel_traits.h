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
    typedef void Bbox;
    static int D;

    template<typename InputIterator>
    Point make_point(InputIterator begin, InputIterator end);

    Point make_point(int dim);

    Bbox make_bbox(int dim);
};

template<typename Point>
bool less_coordinate(const Point& p, const Point& q, int i);

template<typename Point>
double approximate_cartesian_coordinate(const Point& p, int i);

template<typename Point>
typename Kernel_traits<Point>::Bbox make_bbox(const Point& p);

template<typename Point>
typename Kernel_traits<Point>::Bbox make_bbox(const Point& p, const Point& q);

}
}

#endif // CGAL_DDT_KERNEL_TRAITS_H

