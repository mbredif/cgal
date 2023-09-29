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

#ifndef CGAL_DDT_INTERNAL_PROPERTY_MAP_H
#define CGAL_DDT_INTERNAL_PROPERTY_MAP_H

#include <boost/property_map/property_map.hpp>

namespace CGAL {
namespace DDT {

/// \ingroup PkgPropertyMapRef
/// Default property map. Any `PointSet` or `Triangulation` tile should specialize
/// this property map to expose its internal storage for a property if on is available
/// \cgalModels{ReadablePropertyMap}
template <typename Tile, typename = void>
struct Internal_property_map;

}
}

#endif // CGAL_DDT_INTERNAL_PROPERTY_MAP_H
