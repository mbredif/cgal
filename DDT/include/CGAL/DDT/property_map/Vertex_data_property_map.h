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
/// Property map that accesses the data item of a Vertex_iterator in a Triangulation.
/// It is mutable if `T` is not `const` and non-mutable otherwise.
///
/// \cgalModels{ReadWritePropertyMap}
template <typename T>
struct Vertex_data_property_map
{
/// \cond SKIP_IN_MANUAL
  typedef Vertex_data_property_map<T>        Self;
  typedef std::pair<const T&, typename T::Vertex_handle>  key_type;
  typedef std::pair<const T&, typename T::Vertex_const_handle>  const_key_type;
  typedef typename T::Vertex::Data           value_type;
  typedef value_type                         reference;
  typedef boost::read_write_property_map_tag category;

  friend value_type get(const Self&, const const_key_type& k) { return k.second->data(); }
  friend void put(const Self&, const key_type& k, value_type v) { k.second->data() = v; }
/// \endcond
};

/// \ingroup PkgPropertyMapRef
/// Property map that accesses the id member of the data item of a Vertex_iterator in a Triangulation.
/// It is mutable if `T` is not `const` and non-mutable otherwise.
///
/// \cgalModels{ReadWritePropertyMap}
template <typename T>
struct Vertex_data_id_property_map
{
/// \cond SKIP_IN_MANUAL
  typedef Vertex_data_id_property_map<T>       Self;
  typedef std::pair<const T&, typename T::Vertex_handle>    key_type;
  typedef std::pair<const T&, typename T::Vertex_const_handle>  const_key_type;
  typedef typename T::Vertex::Data::Tile_index value_type;
  typedef value_type                           reference;
  typedef boost::read_write_property_map_tag   category;

  friend value_type get(const Self&, const const_key_type& k) { return k.second->data().id; }
  friend void put(const Self&, const key_type& k, value_type v) { k.second->data().id = v; }
/// \endcond
};

}
}

#endif // CGAL_DDT_VERTEX_INFO_PROPERTY_MAP_H
