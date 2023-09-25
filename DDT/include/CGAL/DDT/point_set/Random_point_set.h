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

#include <CGAL/Distributed_point_set.h>
#include <CGAL/DDT/point_set/Point_set_traits.h>
#include <CGAL/DDT/property_map/Partitioner_property_map.h>
#include <random>

namespace CGAL {
namespace DDT {

/// \todo not documented
template<typename PointGenerator>
struct Random_point_set {
    typedef typename PointGenerator::Point                 value_type;
    typedef typename PointGenerator::Point_const_reference const_reference;

    template<typename... Args>
    Random_point_set(std::size_t size, Args&&... args) : size_(size), generator_(std::forward<Args>(args)...) {}
    Random_point_set(std::size_t size, PointGenerator& generator) : size_(size), generator_(generator) {}

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

    const_iterator begin() const { generator_.reset(); return const_iterator(size_, generator_); }
    const_iterator end  () const { return const_iterator(0, generator_); }
    std::size_t size() const { return size_; }

    const PointGenerator& generator() const { return generator_; }
    unsigned int seed() const { return generator_.seed(); }

private:
    mutable PointGenerator generator_;
    std::size_t size_;
};

/// specialization for Random_point_sets
/// \todo meant to be doc?
template<typename PointGenerator>
struct Point_set_traits<Random_point_set<PointGenerator>>
{
    typedef Random_point_set<PointGenerator>   PointSet;
    typedef typename PointSet::value_type      Point;
    typedef typename PointSet::const_reference Point_const_reference;
    typedef typename PointSet::const_iterator  iterator;
    typedef typename PointSet::const_iterator  const_iterator;

    static Point_const_reference point(const PointSet& ps, const const_iterator& v) {
        return *v;
    }

    static inline std::ostream& write(std::ostream& out, const PointSet& ps) { return out << ps; }
};


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

/// \ingroup PkgDDTFunctions
/// makes a distributed point set from point set uniformly generated in its its domain and a partitioner
/// assumes that the tile domains of the partitioner are not overlaping
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
    std::mt19937 gen(points.seed());
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
            dpoints.try_emplace(id, id, n, d, points.seed() + hash(id));
    }

    // assign, if any, the remaining points to the first partition
    if (n_points > 0) {
        Tile_index id = partitioner.begin();
        Tile_domain d = partitioner.domain(id);
        dpoints.try_emplace(id, id, n_points, d, points.seed() + hash(id));
    }
    return std::move(dpoints);
}

}
}

#endif // CGAL_DDT_RANDOM_POINTS_IN_BBOX_H

