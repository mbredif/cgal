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

#ifndef CGAL_DDT_VERTEX_INFO_PROPERTY_MAP_H
#define CGAL_DDT_VERTEX_INFO_PROPERTY_MAP_H

#include <boost/property_map/property_map.hpp>

namespace CGAL {
namespace DDT {

/// \ingroup PkgPropertyMapRef
/// Property map that accesses the info item of a Triangulation_vertex_base_with_info_2 handle.
/// It is mutable if `T` is not `const` and non-mutable otherwise.
///
/// \cgalModels `LvaluePropertyMap`
template <typename T>
struct Vertex_info_property_map
{
/// \cond SKIP_IN_MANUAL
  typedef Vertex_info_property_map<T> Self;
  typedef typename T::Vertex_handle key_type;
  typedef typename T::Vertex::Info value_type;
  typedef value_type& reference;
  typedef boost::lvalue_property_map_tag category;

  value_type& operator[](key_type& k) const { return k->info(); }

  friend value_type& get(const Self&, key_type& k) { return k->info(); }
  friend const value_type& get(const Self&, const key_type& k) { return k->info(); }
  friend void put(const Self&, key_type& k, const value_type& v) { k->info() = v; }
/// \endcond
};

/// \ingroup PkgPropertyMapRef
/// Property map that accesses the info item of a Triangulation_vertex_base_with_info_2 handle.
/// It is mutable if `T` is not `const` and non-mutable otherwise.
///
/// \cgalModels `LvaluePropertyMap`
template <typename T>
struct Vertex_data_info_property_map
{
/// \cond SKIP_IN_MANUAL
  typedef Vertex_data_info_property_map<T> Self;
  typedef typename T::Vertex_handle key_type;
  typedef typename T::Vertex::Info::Info value_type;
  typedef value_type& reference;
  typedef boost::lvalue_property_map_tag category;

  value_type& operator[](key_type& k) const { return k->info().id; }

  friend value_type& get(const Self&, key_type& k) { return k->info().id; }
  friend const value_type& get(const Self&, const key_type& k) { return k->info().id; }
  friend void put(const Self&, key_type& k, const value_type& v) { k->info().id = v; }
/// \endcond
};

}
}

#endif // CGAL_DDT_VERTEX_INFO_PROPERTY_MAP_H
