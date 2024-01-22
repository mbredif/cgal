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

#ifndef CGAL_DDT_LAS_POINT_SET_H
#define CGAL_DDT_LAS_POINT_SET_H
#include <string>
#include <CGAL/DDT/point_set/Point_set_traits.h>
#include <CGAL/IO/read_las_points.h>
#include <CGAL/Distributed_point_set.h>

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTPointSetClasses
/// The LAS_tile_points is a proxy to a LAS file.
/// Points are read on demand, providing a constant-memory iteration over the points in a LAS file.
/// \cgalModels{PointSet}
/// \tparam Point type.
template<typename Point>
class LAS_point_set {
public:
    typedef Kernel_traits<Point> Traits;
    /// Bbox type
    typedef typename Traits::Bbox Bbox;
    /// Point type
    typedef Point value_type;
    /// const reference to Point type
    typedef typename Kernel_traits<Point>::Point_const_reference const_reference;

#ifdef DOXYGEN_RUNNING
private:
    struct unspecified_type {};
public:
    /// single pass const iterator : all iterators of a LAS_point_set share the same reader and point storage
    typedef unspecified_type const_iterator;
    /// single pass iterator : all iterators of a LAS_point_set share the same reader and point storage
    /// \todo do we really need `iterator` ?
    typedef unspecified_type iterator;
#else
    struct const_iterator {

        const_iterator(LASreaderLAS& lasreader, Point& point, std::size_t size) : lasreader_(lasreader), point_(point), size_(size) {
            next();
        }
        const_iterator& operator++() { --size_; next(); return *this;  }
        bool operator==(const_iterator it) const { return size_ == it.size_; }
        bool operator!=(const_iterator it) const { return size_ != it.size_; }
        const_reference operator*() const { return point_; }

    private:
        void next() {
            if (size_==0) return;
            bool ok = lasreader_.read_point();
            CGAL_assertion(ok);
            double coords[] = {
                lasreader_.point.get_x(),
                lasreader_.point.get_y(),
                lasreader_.point.get_z()
            };
            assign(point_, coords, coords+3);
        }
        Point& point_;
        LASreaderLAS& lasreader_;
        std::size_t size_;
    };
    typedef const_iterator iterator;
#endif

    /// begin single pass iterator
    const_iterator begin() const { return {lasreader_, point_, size()}; }
    /// end single pass iterator
    const_iterator end  () const { return {lasreader_, point_, 0}; }

    /// constructor
    /// \param fn filename of the LAS file.
    LAS_point_set(const std::string& fn) : filename_(fn) {
        file_.open(filename_, std::ios::binary);
        CGAL::IO::set_mode(file_, CGAL::IO::BINARY);
        CGAL_assertion(!!file_);
        lasreader_.open(file_);
        double coord0[] = { lasreader_.get_min_x(), lasreader_.get_min_y(), lasreader_.get_min_z() };
        double coord1[] = { lasreader_.get_max_x(), lasreader_.get_max_y(), lasreader_.get_max_z() };
        assign(bbox_, coord0, coord0+3, coord1, coord1+3);
    }
    ~LAS_point_set() {
        lasreader_.close();
    }

    /// filename of the LAS file.
    const std::string& filename() const { return filename_; }
    /// number of points in the LAS file, provided by the LAS header.
    const std::size_t size() const { return lasreader_.npoints; }
    /// returns `size()`
    const std::size_t local_size() const { return size(); }
    /// bounding box of the points provided by the LAS header.
    const Bbox& bbox() const { return bbox_; }

private:
    Bbox bbox_;
    std::string filename_;
    mutable Point point_;
    mutable LASreaderLAS lasreader_;
    std::ifstream file_;
};

template <typename P>
typename Point_set_traits<LAS_point_set<P>>::const_reference
point(const LAS_point_set<P>& ps, typename Point_set_traits<LAS_point_set<P>>::const_iterator v) {
    return *v;
}

/// \ingroup PkgDDTPointSetClasses
/// makes a distributed point set from a collection of LAS files
/// \tparam Point model.
/// \tparam TileIndex model.
/// \tparam StringIterator model.
/// \param (begin, end) range of strings, used as filenames for LAS files.
/// \param id tile index of the LAS point set read at filename "*begin". Tile indices are consecutive from `id` to id+distance(begin,end)-1
template<typename Point, typename TileIndex, typename StringIterator>
CGAL::Distributed_point_set<LAS_point_set<Point>, boost::static_property_map<TileIndex>>
make_distributed_LAS_point_set(TileIndex id, StringIterator begin, StringIterator end)
{
    typedef LAS_point_set<Point> PointSet;
    CGAL::Distributed_point_set<PointSet,boost::static_property_map<TileIndex>> points;
    for(StringIterator filename = begin; filename != end; ++filename, ++id) {
        points.try_emplace(id, id, *filename);
    }
    return std::move(points);
}

}
}

#endif // CGAL_DDT_LAS_POINT_SET_H
