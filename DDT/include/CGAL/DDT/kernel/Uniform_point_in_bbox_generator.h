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

#ifndef CGAL_DDT_UNIFORM_POINT_IN_BBOX_GENERATOR
#define CGAL_DDT_UNIFORM_POINT_IN_BBOX_GENERATOR

#include <random>
#include <iostream>
#include <CGAL/DDT/kernel/Kernel_traits.h>

namespace CGAL {
namespace DDT {

template<typename P>
struct Uniform_point_in_bbox_generator
{
    typedef Kernel_traits<P>                       Traits;
    typedef typename Traits::Point                 Point;
    //typedef typename Traits::Point_reference       Point_reference;
    typedef typename Traits::Point_const_reference Point_const_reference;
    typedef typename Traits::Bbox                  Bbox;
    typedef Bbox                                   Domain;

    Uniform_point_in_bbox_generator(const Bbox& bbox, unsigned int seed) : bbox_(bbox), distrib_(), gen_(seed), seed_(seed) {}

    void reset() { gen_.seed(seed_); }
    Point_const_reference point() const { return point_; }
    const Bbox& bbox() const { return bbox_; }
    const Domain& domain() const { return bbox_; }
    unsigned int seed() const { return seed_; }

    void next() {
        int D = bbox_.dimension();
        std::vector<double> coords(D);
        for(int i=0; i<D; ++i) {
            coords[i] = bbox_.min(i) + distrib_(gen_)*(bbox_.max(i)-bbox_.min(i));
        }
        //set(point_, coords.begin(), coords.end());
        point_ = Traits::point(coords.begin(), coords.end());
    }

private:
    std::mt19937 gen_;
    std::uniform_real_distribution<> distrib_;
    Point point_;
    Bbox bbox_;
    unsigned int seed_;
};

template<typename Point>
std::ostream& operator<<(std::ostream& out, const Uniform_point_in_bbox_generator<Point>& ps)
{
    return out << ps.bbox() << " " << ps.seed();
}

template<typename Point>
std::istream& operator>>(std::istream& in, Uniform_point_in_bbox_generator<Point>& ps)
{
    typename Uniform_point_in_bbox_generator<Point>::Bbox bbox;
    unsigned int seed;
    in >> ps.bbox() >> ps.seed();
    ps = { bbox, seed };
}

}
}

#endif // CGAL_DDT_UNIFORM_POINT_IN_BBOX_GENERATOR

