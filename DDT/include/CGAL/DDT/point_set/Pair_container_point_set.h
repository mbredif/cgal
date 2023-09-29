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

#ifndef CGAL_DDT_PAIR_CONTAINTER_POINT_SET_H
#define CGAL_DDT_PAIR_CONTAINTER_POINT_SET_H

#include <CGAL/IO/io.h>
#include <CGAL/DDT/point_set/Point_set_traits.h>
#include <CGAL/DDT/point_set/mpl.h>
#include <CGAL/DDT/property_map/Internal_property_map.h>
#include <CGAL/DDT/property_map/Pair_container_property_map.h>

namespace CGAL {
namespace DDT {

/// specialization for Containers of (key,point) pairs
/// \todo meant to be doc?
template <typename PointSet>
struct Point_set_traits<PointSet, std::enable_if_t<mpl::is_pair_container_v<PointSet>>>
{
    typedef typename PointSet::value_type::first_type  Tile_index;
    typedef typename PointSet::value_type::second_type Point;
    typedef typename Kernel_traits<Point>::Point_const_reference Point_const_reference;
    typedef typename PointSet::iterator                iterator;
    typedef typename PointSet::const_iterator          const_iterator;


};

template <typename PointSet>
std::enable_if_t<mpl::is_pair_container_v<PointSet>,
typename Point_set_traits<PointSet>::Point_const_reference>
point(const PointSet& ps, typename Point_set_traits<PointSet>::const_iterator v) {
    return v->second;
}

template <typename PointSet, typename Tile_index>
std::enable_if_t<mpl::is_pair_container_v<PointSet>,
std::pair<typename Point_set_traits<PointSet>::iterator, bool>>
insert(PointSet& ps, typename Point_set_traits<PointSet>::Point_const_reference p,
    Tile_index i, typename Point_set_traits<PointSet>::const_iterator hint)
{
    return std::make_pair(ps.emplace(ps.end(), i, p), true);
}

template <typename Container>
struct Internal_property_map<Container, std::enable_if_t<mpl::is_pair_container_v<Container>>>
: public Pair_container_property_map<Container> {};

}

template <typename PointSet>
std::enable_if_t<DDT::mpl::is_pair_container_v<PointSet>, std::ostream&>
operator<<(std::ostream& out, const PointSet& ps) {
    out << ps.size() << " ";
    for(auto it = ps.begin(); it != ps.end(); ++it)
        out << it->first << " " << IO::serialize(it->second) << " ";
    return out;
}

template <typename PointSet>
std::enable_if_t<DDT::mpl::is_pair_container_v<PointSet>, std::istream&>
operator>>(std::istream& in, PointSet& ps) {
    std::size_t size;
    in >> size;
    for(std::size_t i = 0; i<size; ++i) {
        typename PointSet::value_type p;
        in >> p.first >> p.second;
        ps.emplace_back(p);
    }
    return in;
}


}

#endif // CGAL_DDT_PAIR_CONTAINTER_POINT_SET_H

