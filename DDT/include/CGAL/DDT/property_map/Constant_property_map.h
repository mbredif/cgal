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

#ifndef CGAL_DDT_CONSTANT_PROPERTY_MAP_H
#define CGAL_DDT_CONSTANT_PROPERTY_MAP_H

#include <boost/property_map/property_map.hpp>

namespace CGAL {
namespace DDT {

/// \ingroup PkgPropertyMapRef
/// Property map that evaluates a partitioner on the vertex point.
///
/// \cgalModels `ReadablePropertyMap`
template <typename T>
struct Constant_property_map
{
  Constant_property_map(const T& t) : value(t) {}

/// \cond SKIP_IN_MANUAL
  typedef Constant_property_map<T>           Self;
  typedef T                                  value_type;
  typedef const value_type&                  reference;
  typedef boost::readable_property_map_tag   category;

  template<typename Key> friend value_type get(const Self& s, const Key&) { return s.value; }
  T value;
/// \endcond
};

}
}

#endif // CGAL_DDT_CONSTANT_PROPERTY_MAP_H
