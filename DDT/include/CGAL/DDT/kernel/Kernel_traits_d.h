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
#include <CGAL/DDT/kernel/Bbox.h>
#include <CGAL/Kernel_d/Point_d.h>

namespace CGAL {
namespace DDT {

namespace Impl {
    template < typename D >
    struct Dim_value {
      static constexpr int value = D::value;
      Dim_value(int dim) { CGAL_assertion(dim == value); }
      inline constexpr int dimension() const { return value; }
    };

    template <>
    struct Dim_value <Dynamic_dimension_tag> {
        static constexpr int value = 0;
        int dimension_;
        Dim_value(int dim) : dimension_(dim) { CGAL_assertion(dim >= 2); }
        inline int dimension() const { return dimension_; }
    };
}

template<typename K>
struct Kernel_traits<CGAL::Wrap::Point_d<K>> {
    typedef K Kernel;

    /// Point type
    typedef typename K::Point_d Point;

    typedef const Point& Point_const_reference;

    static constexpr int D = Impl::Dim_value<typename Point::Ambient_dimension>::value;

    /// Bbox type
    typedef CGAL::DDT::Bbox<D, double>                      Bbox;

    template<typename InputIterator>
    static inline Point point(InputIterator begin, InputIterator end) {
        int dim = std::distance(begin, end);
        CGAL_assertion(D == 0 || D == dim);
        return Point(dim, begin, end);
    }

    static inline Point point(int dim) {
        CGAL_assertion(D == 0 || D == dim);
        return Point(dim);
    }

    static inline Bbox bbox(int dim) {
      CGAL_assertion(D == 0 || dim == D);
      return Bbox(dim);
    }

};

template<typename K>
inline bool less_coordinate(const CGAL::Wrap::Point_d<K>& p, const CGAL::Wrap::Point_d<K>& q, int i) {
  CGAL_assertion(p.dimension() == q.dimension());
    return p[i] < q[i];
}

template<typename K>
inline double approximate_cartesian_coordinate(const CGAL::Wrap::Point_d<K>& p, int i)
{
    CGAL_assertion(0 <= i && i < p.dimension());
    return CGAL::to_double(p[i]);
}

template<typename K>
auto make_bbox(const CGAL::Wrap::Point_d<K>& p) {
    typedef typename Kernel_traits<CGAL::Wrap::Point_d<K>>::Bbox  Bbox;
    int dim = p.dimension();
    Bbox b(dim);
    int i = 0;
    for(int i = 0; i < dim; ++i) {
        std::pair<double, double> interval = CGAL::to_interval(p[i]);
        b.min(i) = interval.first;
        b.max(i) = interval.second;
    }
    return b;
}

template<typename K>
auto make_bbox(const CGAL::Wrap::Point_d<K>& p, const CGAL::Wrap::Point_d<K>& q) {
    typedef typename Kernel_traits<CGAL::Wrap::Point_d<K>>::Bbox  Bbox;
    CGAL_assertion(p.dimension() == q.dimension());
    int dim = p.dimension();
    Bbox bb(dim);
    int i = 0;
    for(int i = 0; i < dim; ++i) {
        std::pair<double, double> a = CGAL::to_interval(p[i]);
        std::pair<double, double> b = CGAL::to_interval(q[i]);
        bb.min(i) = std::min(a.first, b.first);
        bb.max(i) = std::max(a.second, b.second);
    }
    return bb;
}

}
}

#endif // CGAL_DDT_KERNEL_TRAITS_2_H

