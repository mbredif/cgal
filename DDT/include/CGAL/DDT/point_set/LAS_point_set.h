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
#include <CGAL/IO/read_las_points.h>
#include <CGAL/DDT/property_map/All_local_property_map.h>
#include <CGAL/Distributed_point_set.h>

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTClasses
/// \tparam Point type
/// The LAS_tile_points is a proxy to a LAS file, to be read on demand
/// \todo is it model of something?
template<typename TileIndex, typename Point>
class LAS_point_set {
public:
    typedef Kernel_traits<Point> Traits;
    typedef typename Traits::Bbox Bbox;
    typedef Point value_type;
    typedef typename Kernel_traits<Point>::Point_const_reference const_reference;
    typedef TileIndex Tile_index;

    /// Single Pass Iterator
    struct const_iterator {

        const_iterator(LASreaderLAS& lasreader, std::size_t size) : lasreader_(lasreader), size_(size) {
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
        Point point_;
        LASreaderLAS& lasreader_;
        std::size_t size_;
    };

    const_iterator begin() const { return {lasreader_, size()}; }
    const_iterator end  () const { return {lasreader_, 0}; }

    LAS_point_set(Tile_index id, const std::string& fn) : filename_(fn), id_(id) {
        file_.open(filename_, std::ios::binary);
        CGAL::IO::set_mode(file_, CGAL::IO::BINARY);
        CGAL_assertion(!!file_);
        lasreader_.open(file_);
        double coords[] = {
            lasreader_.get_min_x(), lasreader_.get_min_y(), lasreader_.get_min_z(),
            lasreader_.get_max_x(), lasreader_.get_max_y(), lasreader_.get_max_z()
        };
        assign(bbox_, coords  , coords+3, coords+3, coords+6);
    }
    ~LAS_point_set() {
        lasreader_.close();
    }
    const std::string& filename() const { return filename_; }
    const std::size_t size() const { return lasreader_.npoints; }
    const std::size_t local_size() const { return size(); }
    Tile_index id() const { return id_; }
    const Bbox& bbox() const { return bbox_; }

private:
    Bbox bbox_;
    Tile_index id_;
    std::string filename_;
    mutable LASreaderLAS lasreader_;
    std::ifstream file_;
};


/// general case, for Containers of points
/// \todo is that really meant to be doc?
template <typename I, typename P>
struct Point_set_traits<LAS_point_set<I,P>>
{
    typedef LAS_point_set<I,P>                         PointSet;
    typedef typename PointSet::value_type              Point;
    typedef typename PointSet::const_reference         Point_const_reference;
    typedef typename PointSet::const_iterator          iterator;
    typedef typename PointSet::const_iterator          const_iterator;

    static std::size_t size(const PointSet& ps) { return ps.size(); }
    static std::size_t local_size(const PointSet& ps) { return ps.local_size(); }

    static Point_const_reference point(const PointSet& ps, const_iterator v) {
        return *v;
    }
    static void clear(PointSet& ps) { CGAL_assertion(false); }
};

/// \ingroup PkgDDTFunctions
/// makes a distributed point set from point set uniformly generated in its its domain and a partitioner
/// assumes that the tile domains of the partitioner are not overlaping
/// \todo I don't understand what you mean here
template<typename Point, typename TileIndex, typename StringIterator>
CGAL::Distributed_point_set<LAS_point_set<TileIndex, Point>, All_local_property_map<TileIndex>>
make_distributed_LAS_point_set(TileIndex id, StringIterator begin, StringIterator end)
{
    CGAL::Distributed_point_set<LAS_point_set<TileIndex, Point>, All_local_property_map<TileIndex>> points;
    for(StringIterator filename = begin; filename != end; ++filename, ++id) {
        points.try_emplace(id, id, *filename);
    }
    return std::move(points);
}

}
}

#endif // CGAL_DDT_LAS_TILE_POINTS_H
