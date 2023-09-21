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

#ifndef CGAL_DDT_ALL_LOCAL_PROPERTY_MAP_H
#define CGAL_DDT_ALL_LOCAL_PROPERTY_MAP_H

#include <boost/property_map/property_map.hpp>

namespace CGAL {
namespace DDT {

/// \ingroup PkgPropertyMapRef
/// Property map that returns the `TileIndex` of the Tile for all its iterators.
///
/// \cgalModels{ReadablePropertyMap}
template <typename TileIndex>
struct All_local_property_map
{
/// \cond SKIP_IN_MANUAL
  typedef All_local_property_map<TileIndex>  Self;
  typedef TileIndex                          value_type;
  typedef const value_type&                  reference;
  typedef boost::readable_property_map_tag   category;

  template<typename Tile, typename ConstIterator>
  friend value_type get(const Self& s, const std::pair<const Tile&, ConstIterator>& key) {
    return key.first.id();
  }
/// \endcond
};

}
}

#endif // CGAL_DDT_ALL_LOCAL_PROPERTY_MAP_H
