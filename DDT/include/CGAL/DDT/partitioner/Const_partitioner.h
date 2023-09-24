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

#ifndef CGAL_DDT_PARITIONER_CONST_PARTITIONER_H
#define CGAL_DDT_PARITIONER_CONST_PARTITIONER_H

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTPartitionerClasses
/// Partitioner that assigns all points to a single tile.
/// \cgalModels{Partitioner}
template<typename TileIndex, typename Triangulation>
class Const_partitioner
{
    typedef CGAL::DDT::Triangulation_traits<Triangulation> Traits;
public:
    typedef TileIndex Tile_index;
    typedef typename Traits::Point Point;

    /// Construction with the constant tile index `id`
    Const_partitioner(Tile_index id) : id_(id) {}

    /// The constant tile id affected to all points.
    inline Tile_index operator()(Point_const_reference p) const { return id_;}
    /// The constant tile id affected to all points.
    inline Tile_index id() const { return id_; }
    /// The number of tile indices, which is 1.
    constexpr std::size_t size() const { return 1; }

private:
    Tile_index id_;
};

template<typename Traits>
std::ostream& operator<<(std::ostream& out, const Const_partitioner<Traits>& partitioner) {
    return out << "Const_partitioner( " << partitioner.id() << " )";
}

}
}

#endif // CGAL_DDT_PARITIONER_CONST_PARTITIONER_H
