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

#ifndef CGAL_DDT_NO_TILE_POINTS_H
#define CGAL_DDT_NO_TILE_POINTS_H

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTClasses
/// \tparam T is a model of the TriangulationTraits concept
/// The No_tile_points is the trivial empty point set.
class No_tile_points {
public:
    template<typename PointOutputIterator>
    void read(PointOutputIterator out) {}
    const std::size_t size() const { return 0; }
};

}
}

#endif // CGAL_DDT_NO_TILE_POINTS_H
