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
private:
    std::vector<value_type> values;
};

}
}

#endif // CGAL_DDT_MEDIAN_SELECTOR_H
