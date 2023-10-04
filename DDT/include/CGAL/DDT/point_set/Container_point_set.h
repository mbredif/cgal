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

#ifndef CGAL_DDT_CONTAINER_POINT_SET_H
#define CGAL_DDT_CONTAINER_POINT_SET_H

#include <CGAL/IO/io.h>
#include <CGAL/DDT/point_set/Point_set_traits.h>
#include <CGAL/DDT/point_set/mpl.h>

namespace CGAL {
namespace DDT {

template <typename PointSet>
std::enable_if_t<mpl::is_container_v<PointSet> && !mpl::is_pair_container_v<PointSet>,
typename Point_set_traits<PointSet>::const_reference>
point(const PointSet& ps, typename Point_set_traits<PointSet>::const_iterator v) {
    return *v;
}

template <typename PointSet, typename Tile_index>
std::enable_if_t<mpl::is_container_v<PointSet> && !mpl::is_pair_container_v<PointSet>,
std::pair<typename Point_set_traits<PointSet>::iterator, bool>>
insert(PointSet& ps, typename Point_set_traits<PointSet>::const_reference p, Tile_index /*i*/,
    typename Point_set_traits<PointSet>::const_iterator hint)
{
    return std::make_pair(ps.emplace(ps.end(), p), true);
}

} // namespace DDT

template <typename PointSet>
std::enable_if_t<mpl::is_container_v<PointSet> && !mpl::is_pair_container_v<PointSet>, std::ostream&>
operator<<(std::ostream& out, const PointSet& ps) {
    out << ps.size() << " ";
    for(auto it = ps.begin(); it != ps.end(); ++it)
        out << IO::serialize(*it) << " ";
    return out;
}

template <typename PointSet>
std::enable_if_t<mpl::is_container_v<PointSet> && !mpl::is_pair_container_v<PointSet>, std::istream&>
operator>>(std::istream& in, PointSet& ps) {
    std::size_t size;
    in >> size;
    for(std::size_t i = 0; i<size; ++i) {
        typename PointSet::value_type p;
        in >> p;
        ps.emplace_back(p);
    }
    return in;
}

} // namespace CGAL

#endif // CGAL_DDT_CONTAINER_POINT_SET_H

