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

#ifndef CGAL_DDT_PAIR_CONTAINER_PROPERTY_MAP_H
#define CGAL_DDT_PAIR_CONTAINER_PROPERTY_MAP_H

#include <boost/property_map/property_map.hpp>

namespace CGAL {
namespace DDT {

/// \ingroup PkgPropertyMapRef
/// Property map that looks up the `first` member of the container's `value_type`,
/// given a key pair composed of a reference to the container and an iterator.
/// \cgalModels{ReadWritePropertyMap}
template <typename PairContainer>
struct Pair_container_property_map
{
/// \cond SKIP_IN_MANUAL
    typedef typename PairContainer::value_type::first_type  value_type;
    typedef value_type&                                     reference;
    typedef boost::read_write_property_map_tag              category;
    typedef Pair_container_property_map<PairContainer>      Self;
    typedef typename PairContainer::iterator                iterator;
    typedef typename PairContainer::const_iterator          const_iterator;
    typedef std::pair<const PairContainer&, const_iterator> const_key_type;
    typedef std::pair<PairContainer&, iterator>             key_type;

    friend value_type get(Self pmap, const const_key_type& key) { return key.second->first; }
    friend void put(Self pmap, const key_type& key, value_type v) { key.second->first = v; }
/// \endcond
};

}
}

#endif // CGAL_DDT_PAIR_CONTAINER_PROPERTY_MAP_H
