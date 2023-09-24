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
#include <array>
#include <iostream>

namespace CGAL {
namespace DDT {

namespace Impl {
template<typename Container, typename Derived>
class Bbox
{
protected:
    typedef typename Container::value_type T;
    Container min_values;
    Container max_values;

public:
    Bbox& operator+=(const Bbox& bbox)
    {
        CGAL_assertion(min_values.size() == 0 || min_values.size() == bbox.min_values.size());
        int dim = bbox.min_values.size();
        for(int i=0; i<dim; ++i)
        {
            if(min_values[i] > bbox.min_values[i]) min_values[i] = bbox.min_values[i];
            if(max_values[i] < bbox.max_values[i]) max_values[i] = bbox.max_values[i];
        }
        return *this;
    }

    inline int dimension() const
    {
        return static_cast<const Derived*>(this)->dimension();
    }

    inline T min BOOST_PREVENT_MACRO_SUBSTITUTION (int i) const
    {
        return min_values[i];
    }

    inline T max BOOST_PREVENT_MACRO_SUBSTITUTION (int i) const
    {
        return max_values[i];
    }

    inline T& min BOOST_PREVENT_MACRO_SUBSTITUTION (int i)
    {
        return min_values[i];
    }

    inline T& max BOOST_PREVENT_MACRO_SUBSTITUTION (int i)
    {
        return max_values[i];
    }

    inline T measure() const {
        T result = max_values[0] - min_values[0];
        if (result <= 0) return 0;
        for(int i=1; i<dimension(); ++i)
            result *= max_values[i] - min_values[i];
        return result;
    }

    inline T intersection_measure(const Bbox& bbox) const {
        CGAL_assertion(dimension() == bbox.dimension());
        T result = 1;
        for(int i=0; i<dimension(); ++i) {
            result *= std::min(max(i), bbox.max(i)) -
                      std::max(min(i), bbox.min(i));
            if (result <= 0) return 0;
        }
        return result;
    }

    bool operator==(const Bbox& bbox) const {
        for(int i=0; i<dimension(); ++i)
            if (min_values[i] != bbox.min_values[i] || max_values[i] != bbox.max_values[i])
                return false;
        return true;
    }

    bool operator!=(const Bbox& bbox) const { return !operator==(bbox); }

protected:
    void init(int d, T range = -std::numeric_limits<T>::infinity()) {
        for(int i=0; i<d; ++i)
        {
            min_values[i] = -range;
            max_values[i] =  range;
        }
    }
};

}

// A D-dimensional axis aligned box
template<unsigned int N, typename T>
class Bbox : public Impl::Bbox<std::array<T, N>, Bbox<N,T>>
{
    enum { D = N };
public:
    inline constexpr int dimension() const { return D; }
    Bbox(int d = 0          ) { assert(d==N || d==0); this->init(d       ); }
    Bbox(int d, double range) { assert(d==N || d==0); this->init(d, range); }
};

// A D-dimensional axis aligned box
template<typename T>
class Bbox<0,T> : public Impl::Bbox<std::vector<T>, Bbox<0,T>>
{
public:
    inline int dimension() const { return this->min_values.size(); }
    Bbox(int d = 0          ) { init_values(d); this->init(d       ); }
    Bbox(int d, double range) { init_values(d); this->init(d, range); }

protected:
    void init_values(int d) {
        this->min_values.resize(d);
        this->max_values.resize(d);
    }
};

template<typename Container, typename Derived>
std::ostream& operator<<(std::ostream& out, const Impl::Bbox<Container, Derived>& bbox)
{
    int d = bbox.dimension();
    for(int i=0; i<d; ++i)
        out  << bbox.min(i) << "  " << bbox.max(i) << " ";
    return out;
}


template<typename Container, typename Derived>
std::istream& operator>>(std::istream& in, Impl::Bbox<Container, Derived>& bbox)
{
    int d = bbox.dimension();
    for(int i=0; i<d; ++i)
        in  >> bbox.min(i) >> bbox.max(i);
    return in;
}

template<unsigned int D, typename T, typename InputIterator0, typename InputIterator1>
inline void set(Bbox<D, T>& bb, InputIterator0 begin0, InputIterator0 end0, InputIterator1 begin1, InputIterator1 end1) {
    int dim0 = std::distance(begin0, end0);
    CGAL_assertion(dim0 == std::distance(begin1, end1));
    CGAL_assertion(D==0 || D == dim0);
    bb = { dim0 };
    for(int i = 0; begin0 != end0; ++i, ++begin0, ++begin1) {
        bb.min(i) = std::min(*begin0, *begin1);
        bb.max(i) = std::max(*begin0, *begin1);
    }
}

}
}

#endif // CGAL_DDT_BBOX_H

