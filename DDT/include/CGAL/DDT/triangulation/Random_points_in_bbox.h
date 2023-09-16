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

namespace CGAL {
namespace DDT {

template<typename Point_, typename Bbox_>
struct Random_points_in_bbox
{
    typedef Point_ Point;
    typedef Bbox_ Bbox;
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
        // point = Traits.point(p.begin(), p.end());
        point = Point(p[0], p[1]); // todo: other dimensions
        return *this;
    }
    Random_points_in_bbox operator++(int) {
        Random_points_in_bbox tmp = *this;
        ++(*this);
        return tmp;
    }
};

}
}

#endif // CGAL_DDT_RANDOM_POINTS_IN_BBOX_H

