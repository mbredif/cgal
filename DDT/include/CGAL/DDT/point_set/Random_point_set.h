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

#ifndef CGAL_DDT_RANDOM_POINT_SET_H
#define CGAL_DDT_RANDOM_POINT_SET_H

#include <CGAL/Distributed_point_set.h>
#include <CGAL/DDT/point_set/Point_set_traits.h>
#include <CGAL/DDT/property_map/Partitioner_property_map.h>
#include <random>

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTPointSetClasses
/// A point set that provides a single pass constant iterator to a sequence of on-the-fly generated points.
/// This point set saves memory by keeping a single point in memory, which is the last generated point.
/// Thus the const_reference is invalidated as soon as the iterator is incremented.
/// \cgalModels{PointSet}
/// \tparam PointGenerator a model of `PointGenerator`
template<typename PointGenerator>
struct Random_point_set {

    /// Point type
    typedef typename PointGenerator::value_type      value_type;

    // const reference to Point type
    typedef typename PointGenerator::const_reference const_reference;

    /// constructs a `Random_point_set` from its `size` and the arguments to be forwarded to the `PointGenerator` constructor
    template<typename... Args>
    Random_point_set(std::size_t size, Args&&... args) : size_(size), generator_(std::forward<Args>(args)...) {}

    /// constructs a `Random_point_set` from its `size` and a `PointGenerator` reference
    Random_point_set(std::size_t size, PointGenerator& generator) : size_(size), generator_(generator) {}

#ifdef DOXYGEN_RUNNING
private:
    struct unspecified_type {};
public:
    /// constant single pass iterator type over the generated points.
    typedef unspecified_type const_iterator;
    /// constant single pass iterator type over the generated points.
    typedef unspecified_type iterator;
#else
    /// constant single pass iterator type over the generated points.
    struct const_iterator {
        const_iterator(std::size_t remaining, PointGenerator& generator) : remaining_(remaining), generator_(generator) {
            if (remaining > 0) generator.next();
        }

        const_reference operator*() const { return generator_.point(); }
        bool operator==(const const_iterator& it) const {
            return remaining_ == it.remaining_ && &generator_ == &it.generator_ ;
        }

        bool operator!=(const const_iterator& it) const { return !(*this == it); }
        const_iterator operator++() { generator_.next(); --remaining_; return *this; }

    private :
        PointGenerator& generator_;
        std::size_t remaining_;
    };
    /// constant single pass iterator type over the generated points.
    typedef const_iterator iterator;
#endif

    /// begin const iterator
    const_iterator begin() const { generator_.reset(); return const_iterator(size_, generator_); }
    /// end const iterator
    const_iterator end  () const { return const_iterator(0, generator_); }
    /// number of points
    std::size_t size() const { return size_; }
    /// underlying `PointGenerator`
    const PointGenerator& generator() const { return generator_; }

private:
    mutable PointGenerator generator_;
    std::size_t size_;
};

template<typename PointGenerator>
typename PointGenerator::const_reference
point(const Random_point_set<PointGenerator>& ps, typename Random_point_set<PointGenerator>::const_iterator v) {
    return *v;
}

template<typename PointGenerator>
std::ostream& operator<<(std::ostream& out, const Random_point_set<PointGenerator>& ps)
{
    return out << ps.generator() << " " << ps.size();
}

template<typename PointGenerator>
std::istream& operator>>(std::istream& in, Random_point_set<PointGenerator>& ps)
{
    std::size_t size;
    PointGenerator rpg;
    in >> rpg >> size;
    ps = { rpg, size };
    return in;
}

/// \ingroup PkgDDTPointSetClasses
/// constructs a distributed point set from a `Random_point_set` and a `Partitioner`.
/// It assumes that the tile domains of the partitioner are not overlaping
/// \todo I don't understand what you mean
template<typename PointGenerator, typename Partitioner>
CGAL::Distributed_point_set<
    Random_point_set<PointGenerator>,
    boost::static_property_map<typename Partitioner::Tile_index>
>
make_distributed_point_set(const Random_point_set<PointGenerator>& points, const Partitioner& partitioner)
{
    typedef Random_point_set<PointGenerator>              Point_set;
    typedef typename Partitioner::Tile_index              Tile_index;
    typedef boost::static_property_map<Tile_index>        PropertyMap;
    typedef Distributed_point_set<Point_set, PropertyMap> Distributed_point_set;
    typedef typename PointGenerator::Domain               Domain;
    typedef typename Partitioner::Domain                  Tile_domain;

    Distributed_point_set dpoints;
    std::hash<Tile_index> hash;
    std::mt19937 gen(points.generator().seed());
    Domain domain = points.generator().domain();

    // get the number of generated points that fall into the partitioner's domain
    double M = measure(domain);
    double m = intersection_measure(domain, partitioner.domain());
    std::binomial_distribution<std::size_t> distrib(points.size(), m/M);
    std::size_t n_points = distrib(gen);

    // assign points to the partitions ]begin, end[, using the multinomial distribution
    M = m;
    for(Tile_index id = partitioner.begin()+1; id < partitioner.end() &&  0 < n_points; ++id) {
        Tile_domain d = partitioner.domain(id);
        m = intersection_measure(domain, d);
        std::binomial_distribution<std::size_t> distrib(n_points, m/M);
        std::size_t n = distrib(gen);
        M -= m;
        n_points -= n;
        if (n>0)
            dpoints.try_emplace(id, id, n, d, points.generator().seed() + hash(id));
    }

    // assign, if any, the remaining points to the first partition
    if (n_points > 0) {
        Tile_index id = partitioner.begin();
        Tile_domain d = partitioner.domain(id);
        dpoints.try_emplace(id, id, n_points, d, points.generator().seed() + hash(id));
    }
    return std::move(dpoints);
}

}
}

#endif // CGAL_DDT_RANDOM_POINT_SET_H

