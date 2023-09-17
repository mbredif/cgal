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
#include <CGAL/DDT/Tile_point_set.h>
#include <CGAL/DDT/serializer/No_serializer.h>
#include <CGAL/DDT/point_set/No_tile_points.h>
#include <CGAL/DDT/point_set/count_random_points_in_tiles.h>

namespace CGAL {

/// \ingroup PkgDDTClasses
/// Distributed point set
template<typename PointSet_,
         typename TileIndexProperty_,
         typename Serializer = CGAL::DDT::No_serializer>
struct Distributed_point_set {
    using Point_set            = PointSet_;
    using TileIndexProperty    = TileIndexProperty_;
    using Tile_point_set       = CGAL::DDT::Tile_point_set<Point_set, TileIndexProperty>;
    using Traits               = CGAL::DDT::Point_set_traits<Point_set>;
    using Vertex_index         = typename Traits::const_iterator;
    using Point                = typename Traits::Point;
    using Tile_index           = typename TileIndexProperty::value_type;

    typedef std::map<Tile_index, Tile_point_set> Container;
    typedef typename Container::iterator iterator;
    typedef typename Container::value_type value_type;
    typedef typename Container::mapped_type mapped_type;
    typedef typename Container::reference reference;
    typedef typename Container::key_type key_type;
    typedef typename Container::const_iterator const_iterator;
    typedef typename Container::node_type node_type;

    template<typename P>
    void push_back(P&& value) {
        key_type key = value.first;
        std::move(value.second.begin(), value.second.end(), std::back_inserter(tiles[key]));
    }

    void clear() { tiles.clear(); }
    mapped_type& operator[](key_type key) { return tiles[key]; }
    iterator find(key_type key) { return tiles.find(key); }
    std::pair<iterator, iterator> equal_range(key_type k) { return tiles.equal_range(k); }
    std::pair<const_iterator, const_iterator> equal_range(key_type k) const { return tiles.equal_range(k); }
    iterator erase(iterator it) { return tiles.erase(it); }
    iterator erase(iterator first, iterator last) { return tiles.erase(first, last); }
    const_iterator begin  () const { return tiles.begin (); }
    const_iterator end    () const { return tiles.end   (); }
    iterator begin  () { return tiles.begin (); }
    iterator end    () { return tiles.end   (); }
    bool empty() const { return size_ == 0; }
    std::size_t size() const { return size_; }


    template<typename Point, typename Tile_index>
    void insert(const Point& point, Tile_index id, Tile_index tid)
    {
        auto it = tiles.emplace(std::piecewise_construct,
            std::forward_as_tuple(tid),
            std::forward_as_tuple(tid, tile_indices)).first;
        it->second.insert(point, id);
    }

    Distributed_point_set(TileIndexProperty tile_indices = {}) : tile_indices(tile_indices) {}

    template<typename Iterator, typename Partitioner>
    Distributed_point_set(Iterator it, std::size_t size, Partitioner& part) : tile_indices() {
        for(std::size_t i = 0; i < size; ++i, ++it) {
            Point p = *it;
            Tile_index id = part(p);
            insert(p, id, id);
        }
    }

    template<typename Iterator, typename Partitioner>
    Distributed_point_set(Iterator begin, Iterator end, Partitioner& part) : tile_indices() {
        for(Iterator it = begin; it != end; ++it) {
            Point p = *it;
            Tile_index id = part(p);
            insert(p, id, id);
        }
    }

    template <typename Iterator>
    Distributed_point_set(Tile_index id, Iterator begin, Iterator end) : tile_indices(id) {
        for(Iterator it = begin; it != end; ++it, ++id) {
            tiles.emplace(std::piecewise_construct,
                std::forward_as_tuple(id),
                std::forward_as_tuple(id, id, *it));
        }
    }


    TileIndexProperty tile_indices;
    Container tiles;
    std::size_t size_;
};

}

#endif // CGAL_DISTRIBUTED_POINT_SET_H
