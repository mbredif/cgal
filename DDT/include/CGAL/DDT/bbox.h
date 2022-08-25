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

#ifndef CGAL_DDT_BBOX_H
#define CGAL_DDT_BBOX_H

#include <boost/config.hpp> // defines BOOST_PREVENT_MACRO_SUBSTITUTION
#include <stddef.h>
#include <limits>
#include <iostream>

namespace ddt
{

template<int D, typename T>
struct Bbox
{
    Bbox(T range)
    {
        for(int i=0; i<D; ++i)
        {
            value[i  ] = -range;
            value[i+D] =  range;
        }
    }
    Bbox(T m, T M)
    {
        for(int i=0; i<D; ++i)
        {
            value[i  ] = m;
            value[i+D] = M;
        }
    }

    // empty box (min>max in all dimensions)
    Bbox()
    {
        for(int i=0; i<D; ++i)
        {
            value[i  ] = +std::numeric_limits<T>::infinity();
            value[i+D] = -std::numeric_limits<T>::infinity();
        }
    }

    template<typename It>
    Bbox& insert(It begin, It end)
    {
        for(It it = begin; it != end; ++it)
            *this += *it;
        return *this;
    }

    template<typename P>
    Bbox& operator+=(const P& p)
    {
        for(int i=0; i<D; ++i)
        {
            if(value[i  ] > p[i]) value[i  ] = p[i];
            if(value[i+D] < p[i]) value[i+D] = p[i];
        }
        return *this;
    }

    Bbox& operator+=(const Bbox& bbox)
    {
        for(int i=0; i<D; ++i)
        {
            if(value[i  ] > bbox.value[i  ]) value[i  ] = bbox.value[i  ];
            if(value[i+D] < bbox.value[i+D]) value[i+D] = bbox.value[i+D];
        }
        return *this;
    }

    inline T min BOOST_PREVENT_MACRO_SUBSTITUTION (int i) const
    {
        return value[i  ];
    }
    inline T max BOOST_PREVENT_MACRO_SUBSTITUTION (int i) const
    {
        return value[i+D];
    }

    T value[2*D];
};


template<int D, typename T>
std::ostream& operator<<(std::ostream& out, const Bbox<D, T>& bbox)
{
    for(int i=0; i<D; ++i)
        out  << bbox.value[i] << "  " << bbox.value[i+D] << " ";
    return out;
}


template<int D, typename T>
std::istream& operator>>(std::istream& in, Bbox<D, T>& bbox)
{
    for(int i=0; i<D; ++i)
        in  >> bbox.value[i] >> bbox.value[i+D];
    return in;
}


}

#endif // CGAL_DDT_BBOX_H

