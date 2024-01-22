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

#ifndef CGAL_DDT_POINT_SET_TRAITS_H
#define CGAL_DDT_POINT_SET_TRAITS_H

#include <CGAL/DDT/kernel/Kernel_traits.h>

namespace CGAL {
namespace DDT {

/// general case, for Containers of points
/// \todo meant to be doc? MB: maybe not. Maybe the doc should be moved to the PointSet concept
template <typename PointSet, typename = void>
struct Point_set_traits {
    typedef typename PointSet::value_type      value_type;
    typedef typename Kernel_traits<value_type>::Point_const_reference const_reference;
    typedef typename PointSet::const_iterator  iterator;
    typedef typename PointSet::const_iterator  const_iterator;
};

template<typename T, typename = void> struct is_point_set : public std::false_type {};
template<typename T>
struct is_point_set<T, typename Point_set_traits<T>::Point> : public std::true_type {};
template<class T> inline constexpr bool is_point_set_v = is_point_set<T>::value;

template <typename T>
std::enable_if<is_point_set_v<T>,void> clear(const T& ps) {
    ps.clear();
}

/*
template <typename PointSet, typename ConstIterator>
typename Point_set_traits<PointSet>::Point_const_reference
point(const PointSet& ps, ConstIterator v);

template <typename PointSet, typename PointConstReference, typename Tile_index, typename ConstIterator>
std::pair<typename Point_set_traits<PointSet>::iterator, bool>
insert(PointSet& ps, PointConstReference p, Tile_index i, ConstIterator hint);

template <typename PointSet>
std::ostream& write(std::ostream& out, const PointSet& ps);

template <typename PointSet>
std::istream& read(std::istream& in, PointSet& ps);
*/

}
}

#endif // CGAL_DDT_POINT_SET_TRAITS_H

