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

#ifndef CGAL_DDT_PARTITIONER_PROPERTY_MAP_H
#define CGAL_DDT_PARTITIONER_PROPERTY_MAP_H

#include <boost/property_map/property_map.hpp>
#include <CGAL/DDT/point_set/Point_set_traits.h>

namespace CGAL {
namespace DDT {

/// \ingroup PkgPropertyMapRef
/// Property map that evaluates a partitioner on the vertex point.
///
/// \cgalModels{ReadablePropertyMap}
template <typename T, typename Partitioner>
struct Partitioner_property_map
{
  Partitioner_property_map(const Partitioner& part) : partitioner(part) {}

/// \cond SKIP_IN_MANUAL
  typedef Point_set_traits<T>                      Traits;
  typedef Partitioner_property_map<T, Partitioner> Self;
  typedef typename Traits::const_iterator          Vertex_index;
  typedef std::pair<const T&, Vertex_index>        key_type;
  typedef typename Partitioner::Tile_index         value_type;
  typedef value_type                               reference;
  typedef boost::readable_property_map_tag         category;


  friend value_type get(const Self& self, key_type k) {
    return self.partitioner(Traits::point(k.first, k.second));
  }

/// \endcond

private:
  Partitioner partitioner;
};

}
}

#endif // CGAL_DDT_PARTITIONER_PROPERTY_MAP_H
