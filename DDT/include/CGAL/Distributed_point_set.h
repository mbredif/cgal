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

#ifndef CGAL_DISTRIBUTED_POINT_SET_H
#define CGAL_DISTRIBUTED_POINT_SET_H

#include <map>
#include <CGAL/DDT/tile_points/No_tile_points.h>

namespace CGAL {

/// \ingroup PkgDDTClasses
/// Distributed point set
template<typename TileIndex, typename Point_, typename PointSerializer = CGAL::DDT::No_point_serializer>
struct Distributed_point_set {
    typedef Point_ Point;
    typedef TileIndex Tile_index;

    typedef std::vector<std::pair<Tile_index,Point>> Point_set;
    typedef std::map<Tile_index, Point_set> Container;
    typedef typename Container::iterator iterator;
    typedef typename Container::value_type value_type;
    typedef typename Container::mapped_type mapped_type;
    typedef typename Container::reference reference;
    typedef typename Container::key_type key_type;
    typedef typename Container::const_iterator const_iterator;
    typedef typename Container::node_type node_type;

    Distributed_point_set() {}

    template<typename P>
    void push_back(P&& value) {
        key_type key = value.first;
        std::move(value.second.begin(), value.second.end(), std::back_inserter(point_sets[key]));
    }

    void clear() { point_sets.clear(); }
    mapped_type& operator[](key_type key) { return point_sets[key]; }
    iterator find(key_type key) { return point_sets.find(key); }
    std::pair<iterator, iterator> equal_range(key_type k) { return point_sets.equal_range(k); }
    std::pair<const_iterator, const_iterator> equal_range(key_type k) const { return point_sets.equal_range(k); }
    iterator erase(iterator it) { return point_sets.erase(it); }
    iterator erase(iterator first, iterator last) { return point_sets.erase(first, last); }
    const_iterator begin  () const { return point_sets.begin (); }
    const_iterator end    () const { return point_sets.end   (); }
    iterator begin  () { return point_sets.begin (); }
    iterator end    () { return point_sets.end   (); }
    bool empty() const { return point_sets.empty(); }

    template <typename Iterator>
    Distributed_point_set(Iterator begin, Iterator end, Tile_index id = Tile_index()) {
        for(Iterator it = begin; it != end; ++it, ++id) {
            // point_sets[id].insert(*it); // TODO use a tile container with a LAS reader/serializer
        }
    }

    template<typename PointIndexRange>
    Distributed_point_set(const PointIndexRange& range) {
        for (auto& p : range)
            point_sets[p.first].emplace_back(p.first,p.second);
    }

    template<typename PointRange, typename Partitioner>
    Distributed_point_set(const PointRange& points, Partitioner& part) {
        for(const auto& p : points)  {
            typename Partitioner::Tile_index id = part(p);
            point_sets[id].emplace_back(id,p);
        }
    }

    template<typename Iterator, typename Partitioner>
    Distributed_point_set(Iterator it, int count, Partitioner& part) {
        for(; count; --count, ++it) {
            auto p(*it);
            Tile_index id = part(p);
            point_sets[id].emplace_back(id,p);
        }
    }

  private:
    Container point_sets;
};

}

#endif // CGAL_DISTRIBUTED_POINT_SET_H
