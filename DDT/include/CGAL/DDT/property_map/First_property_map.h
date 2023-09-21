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
/// Property map that looks up the `first` member of the PointSet's `value_type`.
///
/// \cgalModels{ReadablePropertyMap}
template <typename PointSet>
struct First_property_map
{

/// \cond SKIP_IN_MANUAL
  typedef First_property_map<PointSet>               Self;
  typedef Point_set_traits<PointSet>                 Traits;
  typedef typename Traits::iterator                  iterator;
  typedef typename Traits::const_iterator            const_iterator;
  typedef std::pair<const PointSet&, const_iterator> const_key_type;
  typedef std::pair<const PointSet&, iterator>       key_type;
  typedef typename PointSet::value_type::first_type  value_type;
  typedef value_type&                                reference;
  typedef boost::read_write_property_map_tag         category;

  friend value_type get(const Self&, const const_key_type& k) { return k.second->first; }
  friend void put(Self&, const key_type& k, value_type v) { k.second->first = v; }

/// \endcond
};

}
}

#endif // CGAL_DDT_FIRST_PROPERTY_MAP_H
