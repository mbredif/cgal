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

#ifndef CGAL_DDT_FACET_CONST_ITERATOR_D_H
#define CGAL_DDT_FACET_CONST_ITERATOR_D_H

#include <cassert>
#include <iterator>

namespace CGAL {
namespace DDT {

template<typename TDS>
class Facet_const_iterator_d
{
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = typename TDS::Facet;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    typedef typename TDS::Full_cell_const_handle     Cell_const_handle;
    typedef std::pair<Cell_const_handle, int> Facet;

    Facet_const_iterator_d()
        : tds_(nullptr), ft_(), cur_dim_(0)
    {
    }
    Facet_const_iterator_d(const TDS & tds)
        : tds_(&tds), ft_(tds.full_cells_begin(), 0), cur_dim_(tds.current_dimension())
    {
        assert( cur_dim_ > 0 );
        while( ! canonical() )
            raw_increment();
    }
    Facet_const_iterator_d(const TDS & tds, const Facet& ft)
        : tds_(&tds), ft_(ft), cur_dim_(tds.current_dimension())
    {
        assert( cur_dim_ > 0 );
        // do not enforce canonical here !
    }
    Facet_const_iterator_d(const Facet_const_iterator_d & fci)
        : tds_(fci.tds_), ft_(fci.ft_), cur_dim_(fci.cur_dim_)
    {
        assert( cur_dim_ > 0 );
        // do not enforce canonical here !
    }

    Facet_const_iterator_d & operator++()
    {
        increment();
        return (*this);
    }

    Facet_const_iterator_d operator++(int)
    {
        Facet_const_iterator_d tmp(*this);
        increment();
        return tmp;
    }

    bool operator==(const Facet_const_iterator_d & fi) const
    {
        if (tds_ == fi.tds_)
            return tds_ == nullptr ||
                   ((ft_.second == fi.ft_.second) && (ft_.first == fi.ft_.first));
        return (tds_ == nullptr && fi.tds_->full_cells_end() == fi.ft_.first ) ||
               (( fi.tds_ == nullptr && tds_->full_cells_end() == ft_.first ));
    }

    bool operator!=(const Facet_const_iterator_d & fi) const
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

    Facet_const_iterator_d& operator=(const Facet_const_iterator_d & fi)
    {
        tds_ = fi.tds_;
        ft_ = fi.ft_;
        cur_dim_ = fi.cur_dim_;
        return (*this);
    }

private:
    bool canonical()
    {
        if( tds_ == nullptr ) return true;
        if( tds_->full_cells_end() == ft_.first )
            return ( 0 == ft_.second );
        return ( ft_.first < ft_.first->neighbor(ft_.second) );
    }

    void raw_increment()
    {
        int i = ft_.second;
        if( i == cur_dim_ )
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
    int cur_dim_;
};

}
}

#endif // CGAL_DDT_FACET_CONST_ITERATOR_D_H
