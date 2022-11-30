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

#ifndef CGAL_DDT_MINIMUM_SELECTOR_H
#define CGAL_DDT_MINIMUM_SELECTOR_H

#include <assert.h>

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTSelectorClasses
/// Dereferences as the minimum value among the set of inserted values.
/// \cgalModels Selector
template<typename T>
class Minimum_selector
{
public:
    typedef T value_type;
    Minimum_selector() : valid(false), value() {}
    inline clear() { valid = false; }
    inline void insert(value_type v) { if (!valid || v < value) { value = v; valid = true; } }
    inline value_type operator*() { assert(valid); return value; }
private:
    bool valid;
    value_type value;
};

}
}

#endif // CGAL_DDT_MINIMUM_SELECTOR_H
