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

#ifndef CGAL_DDT_TILE_POINT_SET_H
#define CGAL_DDT_TILE_POINT_SET_H

#include <CGAL/DDT/point_set/Point_set_traits.h>

namespace CGAL {
namespace DDT {

// \tparam T is a model of the PointSet concept
// The Tile_point_set stores a local Point set.
template<
    class T,
    class TileIndexProperty>
class Tile_point_set
{
public:
    typedef T                                        Point_set;
    typedef Point_set_traits<T>                      Traits;
    typedef TileIndexProperty                        Tile_index_property;
    typedef typename Tile_index_property::value_type Tile_index;
    typedef typename Traits::Point                   Point;
    typedef typename Traits::Point_const_reference   Point_const_reference;
    typedef typename Traits::iterator                iterator;
    typedef typename Traits::const_iterator          const_iterator;

    /// constructor
    template <typename... Args>
    Tile_point_set(Tile_index id = {}, Tile_index_property index_map = {}, Args&&... args)
        : id_(id),
          ps_(std::forward<Args>(args)...),
          tile_indices(index_map),
          local_size_(std::size(ps_))
    {}

    inline Tile_index id() const { return id_; }
    inline Tile_index& id() { return id_; }

    inline int dimension() const { return Traits::dimension(ps_); }
    inline const_iterator begin() const { return std::begin(ps_); }
    inline const_iterator end  () const { return std::end  (ps_); }
    inline bool empty() const { return std::empty(ps_); }

    inline std::size_t size() const { return std::size(ps_); }
    inline std::size_t local_size() const { return local_size_; }

    inline Tile_index point_id(const_iterator v) const {
        return get(tile_indices, std::make_pair(std::ref(ps_), v));
    }

    inline Point_const_reference point(const_iterator v) const {
        return Traits::point(ps_, v);
    }

    inline std::pair<iterator, bool> insert(Point_const_reference p, Tile_index pid, const_iterator v = {}) {
        auto inserted = Traits::insert(ps_, p, v);
        if(inserted.second) {
            if (pid == id()) ++local_size_;
            if constexpr (std::is_convertible_v<typename Tile_index_property::category, boost::writable_property_map_tag>)
                put(tile_indices, std::make_pair(std::ref(ps_), inserted.first), pid);
            CGAL_assertion(get(tile_indices, std::make_pair(std::cref(ps_), inserted.first)) == pid);
        }

        return inserted;
    }

    inline void remove(iterator v) {
        if(get(tile_indices, std::make_pair(std::ref(ps_), v)) == id()) --local_size;
        Traits::remove(ps_, v);
    }

    template <typename PointSet, typename IndexMap, typename OutputIterator>
    int insert(const PointSet& received, IndexMap received_indices, OutputIterator out)
    {
        iterator v;
        for(auto& r : received)
        {
            Point_const_reference p = Point_set_traits<PointSet>::point(received, r);
            Tile_index id = get(received_indices, r);
            auto inserted = insert(p, id, v);
            v = inserted.first;
            if (inserted.second) *out++ = v;
        }
    }

    Point_set& point_set() { return ps_; }
    const Point_set& point_set() const { return ps_; }
    const Tile_index_property& indices() const { return tile_indices; }

private:
    Tile_index id_;
    Point_set ps_;
    Tile_index_property tile_indices;
    std::size_t local_size_;
};

template<class T, class Pmap>
std::ostream& operator<<(std::ostream& out, const Tile_point_set<T, Pmap>& t)
{
    return CGAL::DDT::Point_set_traits<T>::write(out, t.point_set());
}

template<class T, class Pmap>
std::istream& operator>>(std::istream& in, Tile_point_set<T, Pmap>& t)
{
    return CGAL::DDT::Point_set_traits<T>::read(in, t.point_set());
}

template<class T, class Pmap>
std::ostream& write_summary(std::ostream& out, const Tile_point_set<T, Pmap>& t)
{
    return out << t.size();
}

}
}

#endif // CGAL_DDT_TILE_TRIANGULATION_H
