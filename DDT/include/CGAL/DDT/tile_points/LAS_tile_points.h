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

#ifndef CGAL_DDT_LAS_TILE_POINTS_H
#define CGAL_DDT_LAS_TILE_POINTS_H
#include <string>
#include <CGAL/DDT/IO/read_las.h>

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTClasses
/// \tparam Point type
/// The LAS_tile_points is a proxy to a LAS file, to be read on demand
template<class Point>
class LAS_tile_points {
public:
    LAS_tile_points(const std::string& fn) : filename_(fn) {
        read_LAS_header(fn, size_, pmin_, pmax_);
    }
    template<typename PointOutputIterator>
    void read(PointOutputIterator out) {
        std::ifstream is(filename_, std::ios_base::binary);
        CGAL::IO::set_mode(is, CGAL::IO::BINARY);
        CGAL::IO::read_LAS(is, out);
    }
    const std::string& filename() const { return filename_; }
    const std::size_t size() const { return size_; }
    const Point& min BOOST_PREVENT_MACRO_SUBSTITUTION() const { return pmin_; }
    const Point& max BOOST_PREVENT_MACRO_SUBSTITUTION() const { return pmax_; }

private:
    std::string filename_;
    std::size_t size_;
    Point pmin_;
    Point pmax_;
};


}
}

#endif // CGAL_DDT_LAS_TILE_POINTS_H
