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

#ifndef CGAL_DDT_PARITIONER_GRID_PARTITIONER_H
#define CGAL_DDT_PARITIONER_GRID_PARTITIONER_H

#include <CGAL/DDT/traits/Triangulation_traits.h>
#include <iterator>

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTPartitionerClasses
/// Partitions the domain enclosed by an axis aligned bounding box using a uniform grid.
/// The number of grid steps in each dimension may be specified independently.
/// \cgalModels Partitioner
template<typename Triangulation, typename TileIndexProperty>
class Grid_partitioner
{
    typedef CGAL::DDT::Triangulation_traits<Triangulation> Traits;
public:
    typedef typename TileIndexProperty::value_type Tile_index;
    typedef typename Traits::Point Point;
    typedef typename Traits::Bbox Bbox;
    typedef typename std::vector<std::size_t>::const_iterator const_size_iterator;

    /// Construction with a bbox, a range of number of grid steps in each dimension, and a base tile index.
    /// If the range ends before providing enough elements, its last element is repeatedly used.
    template<typename Iterator>
    Grid_partitioner(const Bbox& bbox, Iterator it, Iterator end, Tile_index id0 = {}) : id0(id0)
    {
        int D = bbox.dimension();
        std::size_t n = 1;
        M = 1;
        N.resize(D);
        inv_step.resize(D);
        origin.resize(D);
        for(int i=0; i<D; ++i)
        {
            if (it!=end) n = *it++;
            N[i] = n;
            inv_step[i] = n/(bbox.max(i)-bbox.min(i));
            origin[i] = bbox.min(i);
            M *= n;
        }
    }

    /// Construction with a bbox, a number grid steps, and a base tile index
    /// All dimensions have the same number of grid steps.
    Grid_partitioner(const Bbox& bbox, std::size_t n, Tile_index id0 = {}) : id0(id0)
    {
        std::size_t D = bbox.dimension();
        M = 1;
        N.resize(D);
        inv_step.resize(D);
        origin.resize(D);
        for(int i=0; i<D; ++i)
        {
            N[i] = n;
            inv_step[i] = n/(bbox.max(i)-bbox.min(i));
            origin[i] = bbox.min(i);
            M *= n;
        }
    }

    /// Computes the tile index of the given point using its approximate Cartesian coordinates
    Tile_index operator()(const Point& p) const
    {
        std::size_t D = N.size();
        Tile_index id = id0;
        for(std::size_t i=0; i<D; ++i)
        {
            double f = (Traits::approximate_cartesian_coordinate(p,i)-origin[i])*inv_step[i];
            if (f <   0  ) f=0;
            if (f >= N[i]) f=N[i]-1;
            id = id*N[i] + Tile_index(f);
        }
        return id;
    }
    /// begin iterator to the range providing the number of tile in each dimensions
    const_size_iterator size_begin() const { return N.begin(); }
    /// end iterator to the range providing the number of tile in each dimensions
    const_size_iterator size_end() const { return N.end(); }
    /// number of tile indices
    std::size_t size() const { return M; }

private:
    std::size_t M;
    std::vector<std::size_t> N;
    std::vector<double> inv_step;
    std::vector<double> origin;
    Tile_index id0;
};

template<typename Traits, typename TileIndex>
std::ostream& operator<<(std::ostream& out, const Grid_partitioner<Traits, TileIndex>& partitioner) {
    out << "Grid_partitioner( ";
    std::copy(partitioner.size_begin(), partitioner.size_end(), std::ostream_iterator<std::size_t>(out, " "));
    return out << ")";
}

}
}

#endif // CGAL_DDT_PARITIONER_GRID_PARTITIONER_H
