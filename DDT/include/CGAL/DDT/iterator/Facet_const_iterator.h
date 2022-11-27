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

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTIteratorClasses
/// A const iterator to the facet of a distributed Delaunay triangulation
template<typename TileContainer>
class Facet_const_iterator
{
public:
    typedef typename TileContainer::Traits                    Traits;
    typedef typename TileContainer::Tile_facet_const_iterator Tile_facet_const_iterator;
    typedef typename TileContainer::Tile_const_iterator       Tile_const_iterator;

    using iterator_category = std::forward_iterator_tag;
    using value_type = Tile_facet_const_iterator;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

private:
    Tile_const_iterator tile_;
    Tile_const_iterator end_;
    Tile_facet_const_iterator facet_;

public:
    Facet_const_iterator(Tile_const_iterator tile, Tile_const_iterator end)
        : tile_(tile), end_(end), facet_()
    {
        if(tile_ != end_)
        {
            facet_ = tile_->facets_begin();
            advance_to_main();
        }
        assert(is_valid());
    }

    Facet_const_iterator(Tile_const_iterator tile, Tile_const_iterator end, Tile_facet_const_iterator facet)
        : tile_(tile), end_(end), facet_(facet)
    {
        // do not enforce main here !
        assert(is_valid());
    }

    Facet_const_iterator(const Facet_const_iterator& c)
        : tile_(c.tile_), end_(c.end_), facet_(c.facet_)
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

    bool operator==(const Facet_const_iterator& f) const
    {
        if (end_ != f.end_) return false;
        if (tile_ == end_ || f.tile_ == end_) return tile_ == f.tile_; // == end_ == f.end_
        if (tile_ == f.tile_) return facet_==f.facet_;
        return tile_->are_facets_equal(facet_, *(f.tile_), f.facet_);
    }

    bool operator!=(const Facet_const_iterator& rhs) const { return !(*this == rhs); }

    const Tile_const_iterator&       tile()  const { return tile_;  }
    const value_type& operator*() const { return facet_; }

    bool is_valid()    const
    {
        return tile_ == end_ || facet_ != tile_->facets_end();
    }
};

}
}

#endif // CGAL_DDT_FACET_CONST_ITERATOR_H
