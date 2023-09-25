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

#ifndef CGAL_DDT_Default_tile_index_map_H
#define CGAL_DDT_Default_tile_index_map_H

#include <boost/property_map/property_map.hpp>

namespace CGAL {
namespace DDT {

/// \ingroup PkgPropertyMapRef
/// Property map that looks up the `first` member of the PointSet's `value_type`.
///
/// \cgalModels{ReadablePropertyMap}
template <typename TileIndex, typename Tile>
struct Default_tile_index_map;

}
}

#endif // CGAL_DDT_Default_tile_index_map_H
