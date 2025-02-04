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
#include <CGAL/DDT/Tile_container.h>

namespace CGAL {

/// \ingroup PkgDDTRef
/// The Distributed_point_set class wraps a Container to expose a point set interface.
/// \tparam PointSet model of `PointSet`, which stores local point sets.
/// \tparam TileIndexProperty model of `VertexPropertyMap`, which provides access to the tile index of the points.
/// \tparam Serializer model of `Serializer`. If not provided, serialization is disabled and computations are performed in core using `CGAL::DDT::No_serializer`
/// \todo add more doc in brief and members? API to be discussed. MB: Serializer is not used yet. For now the distributed point set can only be made scalable in memory when the
/// distributed point set is an immutable input, by using an out-of-memory PointSet model such as LAS_point_set or Random_point_set.
template<typename PointSet_,
         typename TileIndexProperty_,
         typename Serializer = CGAL::DDT::No_serializer>
struct Distributed_point_set {
    using Point_set            = PointSet_;
    using TileIndexProperty    = TileIndexProperty_;
    using Tile_point_set       = CGAL::DDT::Tile_point_set<Point_set, TileIndexProperty>;
    using Traits               = CGAL::DDT::Point_set_traits<Point_set>;
    using Vertex_index         = typename Traits::const_iterator;
    using Point                = typename Traits::value_type;
    using Point_const_reference= typename Traits::const_reference;
    using Tile_index           = typename TileIndexProperty::value_type;

    using AssociativeContainer = std::map<Tile_index, Tile_point_set>; // unordered_map is not suitable as its iterators may get invalidated by try_emplace
    using Container = std::conditional_t<
        std::is_same_v<Serializer, CGAL::DDT::No_serializer>,              // No serialization ?
        AssociativeContainer ,                                             // y: tiles are kept in memory
        CGAL::DDT::Tile_container<AssociativeContainer, Serializer> >;     // n: using serialization and a tile container

    typedef typename Container::iterator iterator;
    typedef typename Container::value_type value_type;
    typedef typename Container::mapped_type mapped_type;
    typedef typename Container::reference reference;
    typedef typename Container::key_type key_type;
    typedef typename Container::const_iterator const_iterator;
    typedef typename Container::node_type node_type;

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

    template<typename... Args>
    std::pair<iterator, bool> try_emplace(const key_type& k, TileIndexProperty indices = {}, Args&&... args)
    {
        return tiles.emplace(std::piecewise_construct,
            std::forward_as_tuple(k),
            std::forward_as_tuple(k, indices, std::forward<Args>(args)...));
    }

    template<typename... Args>
    Distributed_point_set(Args&&... args)
    :   tiles(std::forward<Args>(args)...),
        size_(0)
    {}

    template<typename Point_const_reference, typename Tile_index>
    void insert(Point_const_reference point, Tile_index id, Tile_index tid, TileIndexProperty indices = {})
    {
        try_emplace(tid, indices).first->second.insert_point(point, id);
    }

    template<typename PointIterator, typename Partitioner>
    void insert(PointIterator it, std::size_t size, Partitioner& part, TileIndexProperty indices = {}) {
        for(std::size_t i = 0; i < size; ++i, ++it) {
            Point_const_reference p = *it;
            Tile_index id = part(p);
            insert(p, id, id, indices);
        }
    }

    template<typename PointIterator, typename Partitioner>
    void insert(PointIterator begin, PointIterator end, Partitioner& part, TileIndexProperty indices = {}) {
        for(PointIterator it = begin; it != end; ++it) {
            Point_const_reference p = *it;
            Tile_index id = part(p);
            insert(p, id, id, indices);
        }
    }

    Container tiles;
private:
    std::size_t size_;
};

}

#endif // CGAL_DISTRIBUTED_POINT_SET_H
