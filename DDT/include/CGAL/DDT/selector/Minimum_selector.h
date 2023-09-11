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

// Selects the minimum value among the set of inserted values.
// \cgalModels Selector
template<typename T>
class Minimum_selector
{
public:
    typedef T value_type;
    Minimum_selector() : valid(false), value() {}
    inline void clear() { valid = false; }
    inline void insert(value_type v) { if (!valid || v < value) { value = v; valid = true; } }
    inline value_type select() { assert(valid); return value; }

    template<typename S>
    void cell_statistics(int lower, int equal, int D, int finite,
                S& cells,
                S& finite_cells,
                S& facets,
                S& finite_facets)
    {
        if (equal==0) return; // cell and facets are all foreign
        switch(lower) {
        case 0: { // the cell and all its facets are main
            int f = D+(equal>1);
            ++cells;
            facets += f;
            if(finite) {
                ++finite_cells;
                finite_facets += f;
            } else {
                ++finite_facets;
            }
            return;
        }
        case 1: {
            ++facets;
            finite_facets += finite;
            return;
        }
        default: {}
        }
    }
private:
    bool valid;
    value_type value;
};

}
}

#endif // CGAL_DDT_MINIMUM_SELECTOR_H
