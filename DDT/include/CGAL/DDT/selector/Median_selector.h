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

#ifndef CGAL_DDT_MEDIAN_SELECTOR_H
#define CGAL_DDT_MEDIAN_SELECTOR_H

#include <vector>
#include <algorithm>

namespace CGAL {
namespace DDT {

// Select the median value among the set of inserted values (counting multiplicities).
// \cgalModels Selector
template<typename T>
class Median_selector
{
public:
    typedef T value_type;
    Median_selector() : values() {}
    inline void insert(value_type v) { values.push_back(v); }
    inline void clear() { values.clear(); }
    inline value_type select() {
        typedef typename std::vector<value_type>::iterator iterator;
        iterator begin = values.begin();
        iterator median = begin + (values.size()/2);
        std::nth_element(begin, median, values.end());
        return *median;
    }

    template<typename S>
    void cell_statistics(int lower, int equal, int D, int finite,
                S& cells,
                S& finite_cells,
                S& facets,
                S& finite_facets)
    {
        if (equal==0) return; // cell and facets are all foreign
        int upper = lower + equal;
        int all   = D+finite;
        int fmed1 = (all+1)/2;
        int cmed  = all/2;

        if (lower < fmed1 && fmed1 < upper) { // the cell and all its facets are main
            ++cells;
            facets += D+1;
            ++finite_facets;
            if (finite) {
                ++finite_cells;
                finite_facets += D;
            }
        } else if (fmed1 == lower) {
            facets += fmed1;
            if (finite) finite_facets += fmed1;
            if (cmed == lower) {
                ++cells;
                if (finite) {
                    ++finite_cells;
                } else {
                    ++facets;
                    ++finite_facets;
                }
            }
        } else if (fmed1 == upper) {
            facets += cmed;
            if (finite) finite_facets += cmed;
            if (cmed < upper) {
                ++cells;
                if (finite) {
                    ++finite_cells;
                } else {
                    ++facets;
                    ++finite_facets;
                }
            }
        }
    }
private:
    std::vector<value_type> values;
};

}
}

#endif // CGAL_DDT_MEDIAN_SELECTOR_H
