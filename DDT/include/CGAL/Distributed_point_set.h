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
#include <CGAL/DDT/serializer/No_serializer.h>
#include <CGAL/DDT/Tile_point_set.h>
#include <CGAL/DDT/partitioner/count_random_points_in_tiles.h>

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
    using Vertex_index         = typename Traits::Vertex_index;
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

    Distributed_point_set(TileIndexProperty tile_indices = {}) : tile_indices(tile_indices) {}

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

/*
    template <typename Iterator>
    Distributed_point_set(Iterator begin, Iterator end, Tile_index id = Tile_index()) {
        for(Iterator it = begin; it != end; ++it, ++id) {
            // tiles[id].insert(*it); // TODO use a tile container with a LAS reader/serializer
        }
    }

    template<typename PointIndexRange>
    Distributed_point_set(const PointIndexRange& range) {
        for (auto& p : range)
            tiles[p.first].emplace_back(p.first,p.second);
    }

    template<typename PointRange, typename Partitioner>
    Distributed_point_set(const PointRange& points, Partitioner& part) {
        for(const auto& p : points)  {
            typename Partitioner::Tile_index id = part(p);
            tiles[id].emplace_back(id,p);
        }
    }
    */

    template<typename Iterator, typename Partitioner>
    Distributed_point_set(Iterator it, std::size_t size, Partitioner& part) : tile_indices() {
        for(std::size_t i = 0; i < size; ++i, ++it) {
            Point_2 p = *it;
            Tile_index id = part(p);
            auto it = tiles.emplace(std::piecewise_construct,
                std::forward_as_tuple(id),
                std::forward_as_tuple(id, tile_indices)).first;
            it->second.insert(p, id);
        }
    }

    template<typename RandomPoint, typename Partitioner>
    Distributed_point_set(const CGAL::DDT::Random_point_set<RandomPoint>& ps, Partitioner& part) : tile_indices(part) {
        typedef CGAL::DDT::Random_point_set<RandomPoint> Random_point_set;
        typedef typename RandomPoint::Bbox Bbox;
        std::vector<std::pair<Tile_index, std::size_t>> counts;
        CGAL::DDT::count_random_points_in_tiles(ps, part, std::back_inserter(counts));
        for(auto [id, count]: counts) {
            Bbox bbox = part.bbox(id);
            RandomPoint generator(bbox);
            Random_point_set ps(count, generator);
            auto it = tiles.emplace(std::piecewise_construct,
                std::forward_as_tuple(id),
                std::forward_as_tuple(id, tile_indices)).first;
            it->second.point_set() = ps;
        }
    }

    TileIndexProperty tile_indices;
    Container tiles;
    std::size_t size_;
};

}

#endif // CGAL_DISTRIBUTED_POINT_SET_H
