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
class LAS_point_set {
public:
    typedef Point value_type;

    struct const_iterator {
        const_iterator(std::ifstream *file, std::size_t size) : file_(file), size(size) {}
        const_iterator& operator++() { ++size; return *this; }
        bool operator==(const_iterator it) { return size == it.size && file_ == it.file_; }
        bool operator!=(const_iterator it) { return size != it.size || file_ != it.file_; }
        Point operator*() const {
        //    CGAL::IO::read_LAS(file_, out);
            return {};
        }
        std::ifstream *file_;
        std::size_t size;
    };

    const_iterator begin() const {
        if (!file_.is_open()) {
            file_.open(filename_);
            CGAL::IO::set_mode(file_, CGAL::IO::BINARY);
        }
        return {&file_, 0};
    }
    const_iterator end  () const { return {&file_, size_}; }

    LAS_point_set(const std::string& fn) : filename_(fn) {
        read_LAS_header(fn, size_, pmin_, pmax_);
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
    mutable std::ifstream file_;
};


/// general case, for Containers of points
template <typename P>
struct Point_set_traits<LAS_point_set<P>>
{
    typedef LAS_point_set<P>                           PointSet;
    typedef typename PointSet::value_type              Point;
    typedef typename PointSet::const_iterator          iterator;
    typedef typename PointSet::const_iterator          const_iterator;

    static std::size_t size(const PointSet& ps) { return ps.size(); }

    static const Point& point(const PointSet& ps, const_iterator v) {
        return *v;
    }
    static void clear(PointSet& ps) { CGAL_assertion(false); }
};

}
}

#endif // CGAL_DDT_LAS_TILE_POINTS_H
