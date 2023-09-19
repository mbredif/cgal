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
    typedef typename Traits::iterator                iterator;
    typedef typename Traits::const_iterator          const_iterator;

    /// constructor
    template <typename... Args>
    Tile_point_set(Tile_index id = {}, Tile_index_property index_map = {}, Args&&... args)
        : id_(id),
          ps_(std::forward<Args>(args)...),
          tile_indices(index_map),
          local_size_(Traits::size(ps_))
    {}

    inline Tile_index id() const { return id_; }
    inline Tile_index& id() { return id_; }

    inline int dimension() const { return Traits::dimension(ps_); }
    inline const_iterator begin() const { return std::begin(ps_); }
    inline const_iterator end  () const { return std::end  (ps_); }
    inline bool empty() const { return std::empty(ps_); }

    inline std::size_t size() const { return Traits::size(ps_); }
    inline std::size_t local_size() const { return local_size_; }

    inline Tile_index point_id(const_iterator v) const {
        return get(tile_indices, std::make_pair(std::ref(ps_), v));
    }

    inline Point point(const_iterator v) const {
        return Traits::point(ps_, v);
    }

    inline std::pair<iterator, bool> insert(const Point& p, Tile_index pid, const_iterator v = {}) {
        auto inserted = Traits::insert(ps_, p, v);
        if(inserted.second) {
            if (pid == id()) ++local_size_;
            if constexpr (std::is_convertible_v<typename Tile_index_property::category, boost::writable_property_map_tag>) {
                std::pair<Point_set&,iterator> key(std::ref(ps_), inserted.first);
                put(tile_indices, key, pid);
            } else
                CGAL_assertion(get(tile_indices, std::make_pair(std::ref(ps_), inserted.first)) == pid);
        }

        return inserted;
    }

    inline void remove(iterator v) {
        if(get(tile_indices, std::make_pair(std::ref(ps_), v)) == id()) --local_size;
        Traits::remove(ps_, v);
    }

    inline void spatial_sort(std::vector<std::size_t>& indices, const std::vector<Point>& points) const { Traits::spatial_sort(ps_, indices, points); }

    /// \name Tile_triangulation locality tests
    /// @{
    /// A finite vertex is local if its tile id matches the id of the tile triangulation (tile.vertex_id(vertex) == tile.id()), otherwise, it is foreign
    /// Simplices may be local, mixed or foreign if respectively all, some or none of their finite incident vertices are local.

    /// checks if a finite vertex is local : tile.vertex_id(vertex) == tile.id()
    inline bool vertex_is_local(const_iterator v) const { return vertex_id(v) == id(); }

    /// checks if a finite vertex is foreign : tile.vertex_id(vertex) != tile.id()
    inline bool vertex_is_foreign(const_iterator v) const { return !vertex_is_local(v); }

    /// collects at most 2*D vertices which points define the bounding box of the local tile vertices
    template <class VertexContainer>
    void get_axis_extreme_points(VertexContainer& out) const
    {
        std::vector<const_iterator> vertices;
        int D = dimension();
        vertices.reserve(2*D);
        const_iterator v = begin();
        // first local point
        for(; v != end(); ++v)
        {
            if (vertex_is_local(v))
            {
                for(int i=0; i<2*D; ++i) vertices[i] = v;
                break;
            }
        }
        if(v == end()) return; // no local points
        // other local points
        for(; v != end(); ++v)
        {
            if (vertex_is_local(v))
            {
                const Point& p = point(v);
                for(int i=0; i<D; ++i)
                {
                    if(Traits::less_coordinate(p, point(vertices[i  ]), i)) vertices[i  ] = v;
                    if(Traits::less_coordinate(point(vertices[i+D]), p, i)) vertices[i+D] = v;
                }
            }
        }
        // remove duplicates (O(D^2) implementation, should we bother ?)
        for(int i=0; i<2*D; ++i)
        {
            int j = 0;
            for(; j<i; ++j) if(vertices[j]==vertices[i]) break;
            if(i==j) out.push_back(vertices[i]);
        }
    }

    template <typename PointSet, typename IndexMap, typename OutputIterator>
    int insert(const PointSet& received, IndexMap received_indices, OutputIterator out)
    {
        // retrieve the input points and ids in separate vectors
        // compute the axis-extreme points on the way
        std::vector<Point> points;
        std::vector<Tile_index> ids;
        std::vector<std::size_t> indices;
        std::size_t index=0;
        points.reserve(received.size());
        iterator v;
        for(auto& r : received)
        {
            const Point& p = Point_set_traits<PointSet>::point(received, r);
            Tile_index id = get(received_indices, r);
            auto inserted = insert(p, id, v);
            v = inserted.first;
            if (inserted.second) *out++ = v;
        }
    }

    bool are_vertices_equal(const_iterator v, const Point_set& ps, const_iterator psv) const
    {
        return Traits::are_vertices_equal(ps_, v, ps.ps_, psv);
    }

    const_iterator locate_vertex(const Point& p, const_iterator hint = const_iterator()) const
    {
        return Traits::locate_vertex(ps_, p, hint);
    }

    const_iterator relocate_vertex(const Tile_point_set& t, const_iterator v, const_iterator hint = {}) const
    {
        return locate_vertex(t.point(v), hint);
    }

    Point_set& point_set() { return ps_; }
    const Point_set& point_set() const { return ps_; }

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
