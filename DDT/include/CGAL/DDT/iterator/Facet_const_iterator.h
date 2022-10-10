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

#ifndef CGAL_DDT_FACET_CONST_ITERATOR_H
#define CGAL_DDT_FACET_CONST_ITERATOR_H

namespace ddt
{

template<typename DDT>
class Facet_const_iterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = Facet_const_iterator<DDT>;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    typedef typename DDT::Traits                    Traits;
    typedef typename DDT::Tile_facet_const_iterator Tile_facet_const_iterator;
    typedef typename DDT::Tile_cell_const_iterator  Tile_cell_const_iterator;
    typedef typename DDT::Tile_const_iterator       Tile_const_iterator;
    typedef typename DDT::Cell_const_iterator       Cell_const_iterator;

private:
    Tile_const_iterator begin_;
    Tile_const_iterator end_;
    Tile_const_iterator tile_;
    Tile_facet_const_iterator facet_;

public:
    Facet_const_iterator(Tile_const_iterator begin, Tile_const_iterator end)
        : begin_(begin), end_(end), tile_(begin), facet_()
    {
        if(tile_ != end_)
        {
            facet_ = tile_->facets_begin();
            advance_to_main();
        }
        assert(is_valid());
    }

    Facet_const_iterator(Tile_const_iterator begin, Tile_const_iterator end, Tile_const_iterator tile)
        : begin_(begin), end_(end), tile_(tile), facet_()
    {
        if(tile_ != end_)
        {
            facet_ = tile_->facets_begin();
            advance_to_main();
        }
        assert(is_valid());
    }

    Facet_const_iterator(Tile_const_iterator begin, Tile_const_iterator end, Tile_const_iterator tile, Tile_facet_const_iterator facet)
        : begin_(begin), end_(end), tile_(tile), facet_(facet)
    {
        // do not enforce main here !
        assert(is_valid());
    }

    Facet_const_iterator(const Facet_const_iterator& c)
        : begin_(c.begin_), end_(c.end_), tile_(c.tile_), facet_(c.facet_)
    {
        // do not enforce main here !
        assert(is_valid());
    }
    Facet_const_iterator& advance_to_main()
    {
        while(tile_ != end_)
        {
            if(facet_ == tile_->facets_end())
            {
                if (++tile_ != end_) facet_ = tile_->facets_begin();
            }
            else if(tile_->facet_is_main(facet_))
            {
                break;
            }
            else
            {
                ++facet_;
            }
        }
        return *this;
    }

    Facet_const_iterator& operator++()
    {
        assert(tile_ != end_);
        ++facet_;
        return advance_to_main();
    }

    Facet_const_iterator operator++(int)
    {
        Facet_const_iterator tmp(*this);
        operator++();
        return tmp;
    }

    bool operator==(const Facet_const_iterator& rhs) const
    {
        return begin_ == rhs.begin_
               && tile_ == rhs.tile_
               && end_ == rhs.end_
               && (tile_ == end_ || facet_ == rhs.facet_);
    }

    bool operator!=(const Facet_const_iterator& rhs) const { return !(*this == rhs); }

    const Tile_const_iterator&       tile()  const { return tile_;  }
    const Tile_facet_const_iterator& facet() const { return facet_; }

    bool is_valid()    const
    {
        return tile_ == end_ || facet_ != tile_->facets_end();
    }
};

}

#endif // CGAL_DDT_FACET_CONST_ITERATOR_H
