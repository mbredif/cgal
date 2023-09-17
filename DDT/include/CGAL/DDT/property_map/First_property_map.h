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

#ifndef CGAL_DDT_FIRST_PROPERTY_MAP_H
#define CGAL_DDT_FIRST_PROPERTY_MAP_H

#include <boost/property_map/property_map.hpp>

namespace CGAL {
namespace DDT {

/// \ingroup PkgPropertyMapRef
/// Property map that evaluates a partitioner on the vertex point.
///
/// \cgalModels `ReadablePropertyMap`
template <typename T>
struct First_property_map
{

/// \cond SKIP_IN_MANUAL
  typedef First_property_map<T>                    Self;
  typedef Point_set_traits<T>                      Traits;
  typedef typename Traits::iterator                iterator;
  typedef typename Traits::const_iterator          const_iterator;
  typedef std::pair<const T&, const_iterator>      key_type;
  typedef typename T::value_type::first_type       value_type;
  typedef value_type&                              reference;
  typedef boost::read_write_property_map_tag       category;

  friend value_type get2(const Self& self, const T& tri, const_iterator it) {
    return it->first;
  }
  friend value_type get(const Self&, const key_type& k) { return k.second->first; }
  friend void put(Self&, const std::pair<const T&, iterator>& k, value_type v) { k.second->first = v; }

/// \endcond
};

}
}

#endif // CGAL_DDT_FIRST_PROPERTY_MAP_H
