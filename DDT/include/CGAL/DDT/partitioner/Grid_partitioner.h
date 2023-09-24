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

#include <CGAL/DDT/kernel/Kernel_traits.h>
#include <iterator>

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTPartitionerClasses
/// A partitioner that splits the domain enclosed by an axis aligned bounding box using a uniform grid with user defined step sizes.
/// \cgalModels{Partitioner}
template<typename TileIndex, typename Point_>
class Grid_partitioner
{
    typedef CGAL::DDT::Kernel_traits<Point_> Traits;
public:
    typedef TileIndex Tile_index;
    typedef typename Traits::Point Point;
    typedef typename Traits::Point_const_reference   Point_const_reference;
    typedef typename Traits::Bbox Bbox;
    typedef Bbox Domain;
    /// \todo not documented
    /// \todo can't you use a tuple or std::array?
    typedef typename std::vector<std::size_t>::const_iterator const_size_iterator;

    /// Construction with a bbox, a range of number of grid steps in each dimension, and a base tile index.
    /// If the range ends before providing enough elements, its last element is repeatedly used.
    /// \todo rename bbox -> domain?
    /// \todo shouldn't you use Iso_cuboid if a kernel with exact FT is used? Then domain would return an Iso_cuboid and bbox a Bbox? (maybe that's a pb for the generic Bbox type?)
    /// \todo base tile index -> first index. Probably want to say that it correspond to the 0 cell of the grid and other cell are assigned an incremented index
    /// \todo you may want to doc each parameter rather than do a sentence with all of them.
    template<typename Iterator>
    Grid_partitioner(Tile_index id0, const Bbox& bbox, Iterator it, Iterator end) : id0(id0)
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

    /// Construction with the base tile index, a bbox, the same step size for all dimensions.
    Grid_partitioner(Tile_index id0, const Bbox& bbox, std::size_t n) : id0(id0)
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

    /// returns the tile index of the point `p`
    Tile_index operator()(Point_const_reference p) const
    {
        int D = N.size();
        Tile_index id = 0;
        for(int i=D-1; i>=0; --i)
        {
            double f = (approximate_cartesian_coordinate(p,i)-origin[i])*inv_step[i];
            if (f <   0  ) f=0;
            if (f >= N[i]) f=N[i]-1;
            id = id*N[i] + Tile_index(f);
        }
        return id+id0;
    }
    /// returns begin iterator to the range providing the number of tile in each dimensions
    /// \todo what about step_size(int i) instead?
    const_size_iterator size_begin() const { return N.begin(); }
    ///return  end iterator to the range providing the number of tile in each dimensions
    const_size_iterator size_end() const { return N.end(); }
    /// number of tile indices
    std::size_t size() const { return M; }

    Tile_index begin() const { return id0; }
    Tile_index end  () const { return id0+M; }

    Bbox bbox(Tile_index id) const {
        if (id < id0 || !(id < id0 + size())) {
            return Traits::bbox(N.size());
        }
        id = id - id0;
        std::size_t f(id);
        std::vector<double> p(origin.begin(), origin.end());
        std::vector<double> q(origin.begin(), origin.end());
        std::size_t D = N.size();
        for(std::size_t i=0; i<D; ++i)
        {
            std::size_t x = f % N[i];
            double d = 1.d/inv_step[i];
            f /= N[i];
            p[i] += d*x;
            q[i] = p[i] + d;
        }
        return make_bbox(Traits::point(p.begin(), p.end()), Traits::point(q.begin(), q.end()));
    }

    Bbox bbox() const {
        std::vector<double> p(origin.begin(), origin.end());
        std::vector<double> q(origin.begin(), origin.end());
        std::size_t D = N.size();
        for(std::size_t i=0; i<D; ++i)
            q[i] = p[i] + N[i]/inv_step[i];
        return make_bbox(Traits::point(p.begin(), p.end()), Traits::point(q.begin(), q.end()));
    }
    Domain domain() const { return bbox(); }
    Domain domain(Tile_index id) const { return bbox(id); }

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
