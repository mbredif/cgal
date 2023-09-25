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

    static constexpr int D = Impl::Dim_value<typename Point::Ambient_dimension>::value;

    /// Bbox type
    typedef CGAL::DDT::Bbox<D, double>                      Bbox;

    /// Point const reference type
    typedef Point const& Point_const_reference;

    /// Model of Point_set with embedded TileIndex
    template<typename TileIndex> using Point_set_with_id = std::vector<std::pair<TileIndex, Point>>;
};

template<typename K, typename InputIterator>
inline void assign(CGAL::Wrap::Point_d<K>& p, InputIterator begin, InputIterator end) {
    int dim = std::distance(begin, end);
    CGAL_assertion(Kernel_traits<CGAL::Wrap::Point_d<K>>::D == 0 || Kernel_traits<CGAL::Wrap::Point_d<K>>::D == dim);
    p = CGAL::Wrap::Point_d<K>(dim, begin, end);
}

template<typename K>
inline bool less_coordinate(const CGAL::Wrap::Point_d<K>& p, const CGAL::Wrap::Point_d<K>& q, int i) {
    CGAL_assertion(p.dimension() == q.dimension());
    CGAL_assertion(0 <= i && i < p.dimension());
    return p[i] < q[i];
}

template<typename K>
inline double approximate_cartesian_coordinate(const CGAL::Wrap::Point_d<K>& p, int i)
{
    CGAL_assertion(0 <= i && i < p.dimension());
    return CGAL::to_double(p[i]);
}

template<unsigned int D, typename T, typename K>
inline void assign(CGAL::DDT::Bbox<D, T>& b, const CGAL::Wrap::Point_d<K>& p) {
    int dim = p.dimension();
    CGAL_assertion(D==0 || D == dim);
    b = { dim };
    int i = 0;
    for(int i = 0; i < dim; ++i) {
        std::pair<double, double> interval = CGAL::to_interval(p[i]);
        b.min(i) = interval.first;
        b.max(i) = interval.second;
    }
}

template<unsigned int D, typename T, typename K>
inline void assign(CGAL::DDT::Bbox<D, T>& bb, const CGAL::Wrap::Point_d<K>& p, const CGAL::Wrap::Point_d<K>& q) {
    int dim = p.dimension();
    CGAL_assertion(dim == q.dimension());
    CGAL_assertion(D==0 || D == dim);
    bb = { dim };
    int i = 0;
    for(int i = 0; i < dim; ++i) {
        std::pair<double, double> a = CGAL::to_interval(p[i]);
        std::pair<double, double> b = CGAL::to_interval(q[i]);
        bb.min(i) = std::min(a.first, b.first);
        bb.max(i) = std::max(a.second, b.second);
    }
}

}
}

#endif // CGAL_DDT_KERNEL_TRAITS_2_H

