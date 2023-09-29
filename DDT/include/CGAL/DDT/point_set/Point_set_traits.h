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
/// \todo meant to be doc?
template <typename PointSet, typename = void>
struct Point_set_traits;

template <typename PointSet>
void clear(const PointSet& ps) {
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

