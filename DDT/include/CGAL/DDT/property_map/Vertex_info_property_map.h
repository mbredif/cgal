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
#include <CGAL/DDT/property_map/Data.h>


namespace CGAL {
namespace DDT {

/// \ingroup PkgPropertyMapRef
/// Property map that accesses the info item of a Triangulation_vertex_base_with_info_2/3 handle.
/// It is mutable if `T` is not `const` and non-mutable otherwise.
///
/// \cgalModels{ReadWritePropertyMap}
template <typename T>
struct Vertex_info_property_map
{
/// \cond SKIP_IN_MANUAL
  typedef Vertex_info_property_map<T>        Self;
  typedef typename T::Vertex_handle          Vertex_index;
  typedef std::pair<const T&, Vertex_index>  key_type;
  typedef typename T::Vertex::Info           value_type;
  typedef value_type                         reference;
  typedef boost::read_write_property_map_tag category;

  friend value_type get(const Self&, const key_type& k) { return k.second->info(); }
  friend void put(const Self&, const key_type& k, value_type v) { k.second->info() = v; }
/// \endcond
};

/// \ingroup PkgPropertyMapRef
/// Property map that accesses the id member of the info item of a Triangulation_vertex_base_with_info_2/3 handle.
/// It is mutable if `T` is not `const` and non-mutable otherwise.
///
/// \cgalModels `ReadWritePropertyMap`
template <typename T>
struct Vertex_info_id_property_map
{
/// \cond SKIP_IN_MANUAL
  typedef Vertex_info_id_property_map<T>       Self;
  typedef typename T::Vertex_handle            Vertex_index;
  typedef std::pair<const T&, Vertex_index>    key_type;
  typedef typename T::Vertex::Info::Tile_index value_type;
  typedef value_type                           reference;
  typedef boost::read_write_property_map_tag   category;

  friend value_type get(const Self&, const key_type& k) { return k.second->info().id; }
  friend void put(const Self&, const key_type& k, value_type v) { k.second->info().id = v; }
/// \endcond
};

}
}

#endif // CGAL_DDT_VERTEX_INFO_PROPERTY_MAP_H
