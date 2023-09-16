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


namespace CGAL {
namespace DDT {

template<typename RandomPoint>
struct Random_point_set {
    typedef typename RandomPoint::Bbox   Bbox;
    typedef typename RandomPoint::Point  value_type;
    typedef Random_point_set             const_iterator;
    Random_point_set(std::size_t size = 0, RandomPoint generator = {}) : generator(generator), size_(size) {}

    const value_type& operator*() const { return *generator; }
    bool operator==(const const_iterator& it) const {
        return size_ == it.size_;
    }
    bool operator!=(const const_iterator& rhs) const { return !(*this == rhs); }
    const_iterator operator++() { --size_; ++generator; return *this; }

    const const_iterator& begin() const { return *this; }
    const_iterator end  () const { return const_iterator(0    , generator); }
    std::size_t size() const { return size_; }

    const Bbox& bbox() const { return generator.bbox; }

    RandomPoint generator;
    std::size_t size_;
};

/// general case, for Containers of points
template <typename PointSet, typename = void>
struct Point_set_traits
{
    typedef typename PointSet::value_type           Point;
    typedef typename PointSet::iterator       Vertex_index;

    static std::size_t size(const PointSet& ps) { return ps.size(); }

    static const Point& point(const PointSet& ps, Vertex_index v) {
        return *ps;
    }
    static void clear(PointSet& ps) { ps.clear(); }
    static std::pair<Vertex_index, bool> insert(PointSet& ps, const Point& p, Vertex_index hint = Vertex_index())
    {
        return std::make_pair(ps.emplace(ps.end(), p), true);
    }
};
/// specialization for Containers of (key,point) pairs
template <typename PointSet>
struct Point_set_traits<PointSet, std::enable_if_t<std::is_default_constructible_v<typename PointSet::value_type::first_type>>>
{
    typedef typename PointSet::value_type::first_type  Tile_index;
    typedef typename PointSet::value_type::second_type Point;
    typedef typename PointSet::iterator          Vertex_index;

    static const Point& point(const PointSet& ps, Vertex_index v) {
        return v->second;
    }
    static void clear(PointSet& ps) { ps.clear(); }
    static std::size_t size(const PointSet& ps) { return ps.size(); }

    static std::pair<Vertex_index, bool> insert(PointSet& ps, const Point& p, Vertex_index hint = Vertex_index())
    {
        Tile_index i;
        return std::make_pair(ps.emplace(ps.end(), i, p), true);
    }
};

/// specialization for Random_point_sets
template<typename RandomPoint>
struct Point_set_traits<Random_point_set<RandomPoint>>
{
    typedef Random_point_set<RandomPoint>     PointSet;
    typedef typename PointSet::value_type     Point;
    typedef typename PointSet::const_iterator Vertex_index;

    static std::size_t size(const PointSet& ps) { return ps.size(); }
    static const Point& point(const PointSet& ps, Vertex_index v) {
        return *v;
    }
    static void clear(PointSet& ps) {}
};

/// \ingroup PkgPropertyMapRef
/// Property map that evaluates a partitioner on the vertex point.
///
/// \cgalModels `ReadablePropertyMap`
template <typename T>
struct First_property_map
{

/// \cond SKIP_IN_MANUAL
  typedef First_property_map<T>                    Self;
  typedef Point_set_traits<T>                      Traits;
  typedef typename Traits::Vertex_index            Vertex_index;
  typedef std::pair<const T&, Vertex_index>        key_type;
  typedef typename T::value_type::first_type       value_type;
  typedef value_type&                              reference;
  typedef boost::read_write_property_map_tag       category;

  friend value_type get(const Self&, const key_type& k) { return k.second->first; }
  friend void put(const Self&, key_type& k, value_type v) { k.second->first = v; }

/// \endcond
};

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
    typedef typename Traits::Vertex_index            Vertex_index;

    /// constructor
    Tile_point_set(Tile_index id, Tile_index_property index_map)
        : id_(id),
          ps_(),
          tile_indices(index_map),
          local_size_(0)
    {}

    inline Tile_index id() const { return id_; }
    inline Tile_index& id() { return id_; }

    inline int dimension() const { return Traits::dimension(ps_); }
    inline Vertex_index  begin() { return std::begin(ps_); }
    inline Vertex_index  end  () { return std::end  (ps_); }

    inline std::size_t size() const { return Traits::size(ps_); }
    inline std::size_t local_size() const { return local_size_; }

    inline Tile_index point_id(Vertex_index v) const {
        return get(tile_indices, std::make_pair(std::ref(ps_), v));
    }

    inline Point point(Vertex_index v) const {
        return Traits::point(ps_, v);
    }

    inline void clear() { Traits::clear(ps_); }
    inline std::pair<Vertex_index, bool> insert(const Point& p, Tile_index pid, Vertex_index v = {}) {
        auto inserted = Traits::insert(ps_, p, v);
        if(inserted.second) {
            if (pid == id()) ++local_size_;
            if constexpr (std::is_convertible_v<typename Tile_index_property::category, boost::writable_property_map_tag>) {
                std::pair<const Point_set&,Vertex_index> key(std::cref(ps_), inserted.first);
                put(tile_indices, key, pid);
            } else
                CGAL_assertion(get(tile_indices, std::make_pair(std::ref(ps_), inserted.first)) == pid);
        }

        return inserted;
    }

    inline void remove(Vertex_index v) {
        if(get(tile_indices, std::make_pair(std::ref(ps_), v)) == id()) --local_size;
        Traits::remove(ps_, v);
    }

    inline void spatial_sort(std::vector<std::size_t>& indices, const std::vector<Point>& points) const { Traits::spatial_sort(ps_, indices, points); }

    /// \name Tile_triangulation locality tests
    /// @{
    /// A finite vertex is local if its tile id matches the id of the tile triangulation (tile.vertex_id(vertex) == tile.id()), otherwise, it is foreign
    /// Simplices may be local, mixed or foreign if respectively all, some or none of their finite incident vertices are local.

    /// checks if a finite vertex is local : tile.vertex_id(vertex) == tile.id()
    inline bool vertex_is_local(Vertex_index v) const { return vertex_id(v) == id(); }

    /// checks if a finite vertex is foreign : tile.vertex_id(vertex) != tile.id()
    inline bool vertex_is_foreign(Vertex_index v) const { return !vertex_is_local(v); }

    /// collects at most 2*D vertices which points define the bounding box of the local tile vertices
    template <class VertexContainer>
    void get_axis_extreme_points(VertexContainer& out) const
    {
        std::vector<Vertex_index> vertices;
        int D = dimension();
        vertices.reserve(2*D);
        Vertex_index v = begin();
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
        Vertex_index v;
        for(auto& r : received)
        {
            const Point& p = Point_set_traits<PointSet>::point(received, r);
            Tile_index id = get(received_indices, r);
            auto inserted = insert(p, id, v);
            v = inserted.first;
            if (inserted.second) *out++ = v;
        }
    }

    bool are_vertices_equal(Vertex_index v, const Point_set& ps, Vertex_index psv) const
    {
        return Traits::are_vertices_equal(ps_, v, ps.ps_, psv);
    }

    Vertex_index locate_vertex(const Point& p, Vertex_index hint = Vertex_index()) const
    {
        return Traits::locate_vertex(ps_, p, hint);
    }

    Vertex_index relocate_vertex(const Tile_point_set& t, Vertex_index v, Vertex_index hint = Vertex_index()) const
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
std::ostream& write_summary(std::ostream& out, const Tile_point_set<T, Pmap>& t)
{
    return out << t.local_size();
}

}
}

#endif // CGAL_DDT_TILE_TRIANGULATION_H
