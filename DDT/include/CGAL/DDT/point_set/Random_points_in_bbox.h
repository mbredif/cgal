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
struct Uniform_point_in_bbox
{
    typedef Kernel_traits<P>       Traits;
    typedef typename Traits::Point Point;
    typedef typename Traits::Bbox  Bbox;
    typedef Bbox                   Domain;


    Uniform_point_in_bbox(const Bbox bbox, unsigned int seed) : bbox_(bbox), distrib_(), gen_(seed), seed_(seed) {}

    void reset() { gen_.seed(seed_); }
    const Point& point() const { return point_; }
    const Bbox& bbox() const { return bbox_; }
    const Domain& domain() const { return bbox_; }
    unsigned int seed() const { return seed_; }

    const Point& next() {
        int D = bbox_.dimension();
        std::vector<double> p(D);
        for(int i=0; i<D; ++i) {
            p[i] = bbox_.min(i) + distrib_(gen_)*(bbox_.max(i)-bbox_.min(i));
        }
        return point_ = Traits::point(p.begin(), p.end());
    }

private:
    std::mt19937 gen_;
    std::uniform_real_distribution<> distrib_;
    Bbox bbox_;
    Point point_;
    unsigned int seed_;
};


template<typename RandomPointGenerator>
struct Random_point_set {
    typedef typename RandomPointGenerator::Point  value_type;

    template<typename... Args>
    Random_point_set(std::size_t size, Args&&... args) : size_(size), generator_(std::forward<Args>(args)...) {}
    Random_point_set(std::size_t size, RandomPointGenerator& generator) : size_(size), generator_(generator) {}

    struct const_iterator {
        const_iterator(RandomPointGenerator& generator, std::size_t remaining) : generator_(generator), remaining_(remaining) {
            generator.next();
        }

        value_type operator*() const { return generator_.point(); }
        bool operator==(const const_iterator& it) const {
            return remaining_ == it.remaining_ && &generator_ == &it.generator_ ;
        }

        bool operator!=(const const_iterator& it) const { return !(*this == it); }
        const_iterator operator++() { generator_.next(); --remaining_; return *this; }

    private :
        RandomPointGenerator& generator_;
        std::size_t remaining_;
    };

    const_iterator begin() const { generator_.reset(); return const_iterator(generator_, size_); }
    const_iterator end  () const { return const_iterator(generator_, 0); }
    std::size_t size() const { return size_; }

    const RandomPointGenerator& generator() const { return generator_; }
    unsigned int seed() const { return generator_.seed(); }

private:
    mutable RandomPointGenerator generator_;
    std::size_t size_;
};

/// specialization for Random_point_sets
template<typename RandomPointGenerator>
struct Point_set_traits<Random_point_set<RandomPointGenerator>>
{
    typedef Random_point_set<RandomPointGenerator>     PointSet;
    typedef typename PointSet::value_type     Point;
    typedef typename PointSet::const_iterator iterator;
    typedef typename PointSet::const_iterator const_iterator;

    static std::size_t size(const PointSet& ps) { return ps.size(); }
    static Point point(const PointSet& ps, const const_iterator& v) {
        return *v;
    }

    static inline std::ostream& write(std::ostream& out, const PointSet& ps) { return out << ps; }
};

template<typename Point>
std::ostream& operator<<(std::ostream& out, const Uniform_point_in_bbox<Point>& ps)
{
    return out << ps.bbox() << " " << ps.seed();
}

template<typename RandomPointGenerator>
std::ostream& operator<<(std::ostream& out, const Random_point_set<RandomPointGenerator>& ps)
{
    return out << ps.generator() << " " << ps.size();
}

template<typename Point>
std::istream& operator>>(std::istream& in, Uniform_point_in_bbox<Point>& ps)
{
    typename Uniform_point_in_bbox<Point>::Bbox bbox;
    unsigned int seed;
    in >> ps.bbox() >> ps.seed();
    ps = { bbox, seed };
}

template<typename RandomPointGenerator>
std::istream& operator>>(std::istream& in, Random_point_set<RandomPointGenerator>& ps)
{
    std::size_t size;
    RandomPointGenerator rpg;
    in >> rpg >> size;
    ps = { rpg, size };
    return in;
}

}
}

#endif // CGAL_DDT_RANDOM_POINTS_IN_BBOX_H

