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

#ifndef CGAL_DDT_RANDOM_POINTS_IN_BBOX_H
#define CGAL_DDT_RANDOM_POINTS_IN_BBOX_H

#include <random>
#include <CGAL/DDT/point_set/Point_set_traits.h>

namespace CGAL {
namespace DDT {

template<typename P>
struct Random_points_in_bbox
{
    typedef Kernel_traits<P> Traits;
    typedef typename Traits::Point Point;
    typedef typename Traits::Bbox  Bbox;
    std::mt19937 gen;
    std::uniform_real_distribution<> dis;
    Bbox bbox;
    Point point;
    Random_points_in_bbox(const Bbox bbox = {}) : bbox(bbox), dis(), gen() { ++(*this); }

    bool operator==(const Random_points_in_bbox& other) const { return point==other.point && bbox==other.bbox && dis==other.dis && gen==other.gen; }
    const Point& operator*() const { return point; }
    Random_points_in_bbox& operator++() {
        int D = bbox.dimension();
        std::vector<double> p(D);
        for(int i=0; i<D; ++i) {
            p[i] = bbox.min(i) + dis(gen)*(bbox.max(i)-bbox.min(i));
        }
        point = Traits::point(p.begin(), p.end());
        return *this;
    }
    Random_points_in_bbox operator++(int) {
        Random_points_in_bbox tmp = *this;
        ++(*this);
        return tmp;
    }
};


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

/// specialization for Random_point_sets
template<typename RandomPoint>
struct Point_set_traits<Random_point_set<RandomPoint>>
{
    typedef Random_point_set<RandomPoint>     PointSet;
    typedef typename PointSet::value_type     Point;
    typedef typename PointSet::iterator                iterator;
    typedef typename PointSet::const_iterator          const_iterator;

    static std::size_t size(const PointSet& ps) { return ps.size(); }
    static const Point& point(const PointSet& ps, const_iterator v) {
        return *v;
    }
    static void clear(PointSet& ps) {}

    static inline std::ostream& write(std::ostream& out, const PointSet& ps) { return out << ps; }
    static inline std::istream& read(std::istream& in, PointSet& ps) { return in >> ps; }
};


}
}

#endif // CGAL_DDT_RANDOM_POINTS_IN_BBOX_H

