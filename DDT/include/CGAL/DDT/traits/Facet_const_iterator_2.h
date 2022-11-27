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

#ifndef CGAL_DDT_FACET_CONST_ITERATOR_2_H
#define CGAL_DDT_FACET_CONST_ITERATOR_2_H

namespace CGAL {
namespace DDT {

template <typename TDS> using Cell_const_iterator_2 = typename TDS::Face_iterator;
template <typename TDS> using Facet_2 = std::pair<Cell_const_iterator_2<TDS>, int>;

template<typename TDS>
class Facet_const_iterator_2
{
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = Facet_2<TDS>;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    typedef Cell_const_iterator_2<TDS> Cell_const_iterator;
    typedef Facet_2<TDS>               Facet;

    Facet_const_iterator_2()
        : tds_(nullptr), ft_()
    {
    }
    Facet_const_iterator_2(const TDS & tds)
        : tds_(&tds), ft_(tds.faces_begin(), 0)
    {
        while( ! canonical() ) raw_increment();
    }
    Facet_const_iterator_2(const TDS & tds, const Facet& ft)
        : tds_(&tds), ft_(ft)
    {
        // do not enforce canonical here !
    }
    Facet_const_iterator_2(const Facet_const_iterator_2 & fci)
        : tds_(fci.tds_), ft_(fci.ft_)
    {
        // do not enforce canonical here !
    }

    Facet_const_iterator_2 & operator++()
    {
        increment();
        return (*this);
    }

    Facet_const_iterator_2 operator++(int)
    {
        Facet_const_iterator_2 tmp(*this);
        increment();
        return tmp;
    }

    bool operator==(const Facet_const_iterator_2 & fi) const
    {
        if (tds_ == fi.tds_)
            return tds_ == nullptr ||
                   ((ft_.second == fi.ft_.second) && (ft_.first == fi.ft_.first));
        return (tds_ == nullptr && fi.tds_->faces_end() == fi.ft_.first ) ||
               (( fi.tds_ == nullptr && tds_->faces_end() == ft_.first ));
    }

    bool operator!=(const Facet_const_iterator_2 & fi) const
    {
        return !(*this == fi);
    }

    const Facet& operator*() const
    {
        return ft_;
    }

    const Facet * operator->() const
    {
        return &ft_;
    }

    Facet_const_iterator_2& operator=(const Facet_const_iterator_2 & fi)
    {
        tds_ = fi.tds_;
        ft_ = fi.ft_;
        return (*this);
    }

private:
    bool canonical()
    {
        if( tds_ == nullptr ) return true;
        if( tds_->faces_end() == ft_.first )
            return ( 0 == ft_.second );
        return ( ft_.first < ft_.first->neighbor(ft_.second) );
    }

    void raw_increment()
    {
        int i = ft_.second;
        if( i == 2 )
            ft_ = Facet(++ft_.first, 0);
        else
            ft_ = Facet(ft_.first, i + 1);
    }

    void increment()
    {
        do
        {
            raw_increment();
        }
        while( ! canonical() );
    }

    const TDS *tds_;
    Facet ft_;
};

}
}

#endif // CGAL_DDT_FACET_CONST_ITERATOR_2_H
