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

#ifndef CGAL_DDT_CELL_ITERATOR_H
#define CGAL_DDT_CELL_ITERATOR_H

#include <iterator>

namespace CGAL {
namespace DDT {

template<typename TileContainer>
class Cell_iterator
{
public:
    typedef typename TileContainer::const_iterator     Tile_const_iterator;
    typedef typename TileContainer::Tile_index         Tile_index;
    typedef typename TileContainer::value_type         Tile_triangulation;
    typedef typename Tile_triangulation::Cell_index    Tile_cell_index;

    using iterator_category = std::forward_iterator_tag;
    using value_type = Tile_cell_index;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

private:
    const TileContainer *tiles_;
    Tile_const_iterator tile_;
    Tile_cell_index cell_;

public:
    Cell_iterator(const TileContainer *tiles, Tile_const_iterator tile)
        : tiles_(tiles), tile_(tile), cell_()
    {
        if(tile_ != tiles_->cend())
        {
            cell_ = triangulation().cells_begin();
            advance_to_main();
        }
        assert(is_valid());
    }

    Cell_iterator(const TileContainer *tiles, Tile_const_iterator tile, Tile_cell_index cell)
        : tiles_(tiles), tile_(tile), cell_(cell)
    {
        // do not enforce main here !
        assert(is_valid());
    }

    Cell_iterator(const Cell_iterator& c)
        : tiles_(c.tiles_), tile_(c.tile_), cell_(c.cell_)
    {
        // do not enforce main here !
        assert(is_valid());
    }

    Cell_iterator& advance_to_main()
    {
        while(tile_ != tiles_->cend())
        {
            if(cell_ == triangulation().cells_end())
            {
                if (++tile_ != tiles_->cend())
                    cell_ = triangulation().cells_begin();
            }
            else if(triangulation().cell_is_main(cell_))
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

    bool operator<(const Cell_iterator& c) const
    {
        assert(tiles_ == c.tiles_);
        if (c.tile_ == tiles_->cend()) return tile_ != tiles_->cend();
        if (tile_ == tiles_->cend()) return false;
        return  tile_->id() < c.tile_->id() || (tile_->id() == c.tile_->id() && cell_ < c.cell_);
    }

    Cell_iterator& operator++()
    {
        assert(tile_ != tiles_->cend());
        ++cell_;
        return advance_to_main();
    }

    Cell_iterator operator++(int)
    {
        Cell_iterator tmp(*this);
        operator++();
        return tmp;
    }

    Cell_iterator& operator+=(int n)
    {
        assert(tile_ != tiles_->cend());
        for(Tile_cell_index c = triangulation().cells_begin(); c != cell_; ++c)
            if(triangulation().cell_is_main(c))
                ++n;
        int num_main_cells = tile_->number_of_main_cells();
        while(n >= num_main_cells)
        {
            n -= num_main_cells;
            ++tile_;
            num_main_cells = tile_->number_of_main_cells();
        }
        cell_ = triangulation().cells_begin();
        advance_to_main();
        for(; n>0 ; --n)
            ++(*this);
        assert(is_valid());
        return *this;
    }

    bool operator==(const Cell_iterator& c) const
    {
        if (tiles_ != c.tiles_) return false;
        if (tile_ == tiles_->cend() || c.tile_ == tiles_->cend()) return tile_ == c.tile_;
        if (tile_ == c.tile_) return cell_==c.cell_;
        return triangulation().are_cells_equal(cell_, c.triangulation(), c.cell_);
    }

    bool operator!=(const Cell_iterator& rhs) const { return !(*this == rhs); }

    const Tile_const_iterator&    tile() const { return tile_; }
    const value_type& operator*() const { return cell_; }
    const Tile_triangulation&    triangulation() const { return tile_->second.value(); }
    const Tile_index&            id()            const { return tile_->first; }

    bool is_valid()    const
    {
        return tile_ == tiles_->cend() || cell_ != triangulation().cells_end();
    }
};

}
}

#endif // CGAL_DDT_CELL_ITERATOR_H
