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

#include <CGAL/IO/io.h>
#include <CGAL/DDT/kernel/Kernel_traits.h>
#include <CGAL/DDT/property_map/Default_tile_index_map.h>

namespace CGAL {
namespace DDT {

/// general case, for Containers of points
/// \todo meant to be doc?
template <typename PointSet, typename = void>
struct Point_set_traits
{
    typedef typename PointSet::value_type              Point;
    typedef typename Kernel_traits<Point>::Point_const_reference Point_const_reference;
    typedef typename PointSet::iterator                iterator;
    typedef typename PointSet::const_iterator          const_iterator;

    static Point_const_reference point(const PointSet& ps, const_iterator v) {
        return *v;
    }
    static void clear(PointSet& ps) { ps.clear(); }
    static std::pair<iterator, bool> insert(PointSet& ps, Point_const_reference p, const_iterator hint = {})
    {
        return std::make_pair(ps.emplace(ps.end(), p), true);
    }

    static inline std::ostream& write(std::ostream& out, const PointSet& ps) {
        out << ps.size() << " ";
        for(auto it = ps.begin(); it != ps.end(); ++it)
            out << IO::serialize(*it) << " ";
        return out;
    }

    static inline std::istream& read(std::istream& in, PointSet& ps) {
        std::size_t size;
        in >> size;
        for(std::size_t i = 0; i<size; ++i) {
            typename PointSet::value_type p;
            in >> p;
            ps.emplace_back(p);
        }
        return in;
    }
};

/// specialization for Containers of (key,point) pairs
/// \todo meant to be doc?
template <typename PointSet>
struct Point_set_traits<PointSet, std::enable_if_t<std::is_default_constructible_v<typename PointSet::value_type::first_type>>>
{
    typedef typename PointSet::value_type::first_type  Tile_index;
    typedef typename PointSet::value_type::second_type Point;
    typedef typename Kernel_traits<Point>::Point_const_reference Point_const_reference;
    typedef typename PointSet::iterator                iterator;
    typedef typename PointSet::const_iterator          const_iterator;

    static Point_const_reference point(const PointSet& ps, const_iterator v) {
        return v->second;
    }

    static void clear(PointSet& ps) { ps.clear(); }

    static std::pair<iterator, bool> insert(PointSet& ps, Point_const_reference p, const_iterator hint = {})
    {
        Tile_index i = {};
        return std::make_pair(ps.emplace(ps.end(), i, p), true);
    }

    static inline std::ostream& write(std::ostream& out, const PointSet& ps) {
        out << ps.size() << " ";
        for(auto it = ps.begin(); it != ps.end(); ++it)
            out << it->first << " " << IO::serialize(it->second) << " ";
        return out;
    }

    static inline std::istream& read(std::istream& in, PointSet& ps) {
        std::size_t size;
        in >> size;
        for(std::size_t i = 0; i<size; ++i) {
            typename PointSet::value_type p;
            in >> p.first >> p.second;
            ps.emplace_back(p);
        }
        return in;
    }
};

template <typename Tile>
struct Default_tile_index_map<typename Tile::value_type::first_type, Tile>
{

/// \cond SKIP_IN_MANUAL
    typedef typename Tile::value_type::first_type  value_type;
    typedef value_type&                            reference;
    typedef boost::read_write_property_map_tag     category;
    typedef Default_tile_index_map<value_type, Tile>   Self;
    typedef typename Tile::iterator                iterator;
    typedef typename Tile::const_iterator          const_iterator;
    typedef std::pair<const Tile&, const_iterator> const_key_type;
    typedef std::pair<Tile&, iterator>             key_type;


    friend value_type get(Self pmap, const const_key_type& key) { return key.second->first; }
    friend void put(Self pmap, const key_type& key, value_type v) { key.second->first = v; }
/// \endcond
};

}
}

#endif // CGAL_DDT_POINT_SET_TRAITS_H

