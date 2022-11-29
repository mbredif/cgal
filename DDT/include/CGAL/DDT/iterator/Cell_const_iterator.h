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

#ifndef CGAL_DDT_CELL_CONST_ITERATOR_H
#define CGAL_DDT_CELL_CONST_ITERATOR_H

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTIteratorClasses
/// A const iterator to the cell of a distributed Delaunay triangulation
template<typename TileContainer>
class Cell_const_iterator
{
public:
    typedef typename TileContainer::Traits                    Traits;
    typedef typename TileContainer::Tile_cell_const_iterator  Tile_cell_const_iterator;
    typedef typename TileContainer::Tile_const_iterator       Tile_const_iterator;

    using iterator_category = std::forward_iterator_tag;
    using value_type = Tile_cell_const_iterator;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

private:
    const TileContainer *tiles_;
    Tile_const_iterator tile_;
    Tile_cell_const_iterator cell_;

public:
    Cell_const_iterator(const TileContainer *tiles, Tile_const_iterator tile)
        : tiles_(tiles), tile_(tile), cell_()
    {
        if(tile_ != tiles_->cend())
        {
            cell_ = tile_->cells_begin();
            advance_to_main();
        }
        assert(is_valid());
    }

    Cell_const_iterator(const TileContainer *tiles, Tile_const_iterator tile, Tile_cell_const_iterator cell)
        : tiles_(tiles), tile_(tile), cell_(cell)
    {
        // do not enforce main here !
        assert(is_valid());
    }

    Cell_const_iterator(const Cell_const_iterator& c)
        : tiles_(c.tiles_), tile_(c.tile_), cell_(c.cell_)
    {
        // do not enforce main here !
        assert(is_valid());
    }

    Cell_const_iterator& advance_to_main()
    {
        while(tile_ != tiles_->cend())
        {
            if(cell_ == tile_->cells_end())
            {
                if (++tile_ != tiles_->cend())
                    cell_ = tile_->cells_begin();
            }
            else if(tile_->cell_is_main(cell_))
            {
                break;
            }
            else
            {
                ++cell_;
            }
        }
        return *this;
    }

    bool operator<(const Cell_const_iterator& c) const
    {
        assert(tiles_ == c.tiles_);
        if (c.tile_ == tiles_->cend()) return tile_ != tiles_->cend();
        if (tile_ == tiles_->cend()) return false;
        return  tile_->id() < c.tile_->id() || (tile_->id() == c.tile_->id() && cell_ < c.cell_);
    }

    Cell_const_iterator& operator++()
    {
        assert(tile_ != tiles_->cend());
        ++cell_;
        return advance_to_main();
    }

    Cell_const_iterator operator++(int)
    {
        Cell_const_iterator tmp(*this);
        operator++();
        return tmp;
    }

    Cell_const_iterator& operator+=(int n)
    {
        assert(tile_ != tiles_->cend());
        for(auto cit = tile_->cells_begin(); cit != cell_; ++cit)
            if(tile_->cell_is_main(cit))
                ++n;
        int num_main_cells = tile_->number_of_main_cells();
        while(n >= num_main_cells)
        {
            n -= num_main_cells;
            ++tile_;
            num_main_cells = tile_->number_of_main_cells();
        }
        cell_ = tile_->cells_begin();
        advance_to_main();
        for(; n>0 ; --n)
            ++(*this);
        assert(is_valid());
        return *this;
    }

    bool operator==(const Cell_const_iterator& c) const
    {
        if (tiles_ != c.tiles_) return false;
        if (tile_ == tiles_->cend() || c.tile_ == tiles_->cend()) return tile_ == c.tile_;
        if (tile_ == c.tile_) return cell_==c.cell_;
        return tile_->are_cells_equal(cell_, *(c.tile_), c.cell_);
    }

    bool operator!=(const Cell_const_iterator& rhs) const { return !(*this == rhs); }

    const Tile_const_iterator&    tile() const { return tile_; }
    const value_type& operator*() const { return cell_; }

    bool is_valid()    const
    {
        return tile_ == tiles_->cend() || cell_ != tile_->cells_end();
    }
};

}
}

#endif // CGAL_DDT_CELL_CONST_ITERATOR_H
