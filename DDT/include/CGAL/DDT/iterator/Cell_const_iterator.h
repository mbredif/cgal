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

namespace ddt
{

template<typename DDT>
class Cell_const_iterator : public std::iterator<std::input_iterator_tag, Cell_const_iterator<DDT> >
{
public:
    typedef typename DDT::Traits                    Traits;
    typedef typename DDT::Tile_cell_const_handle    Tile_cell_const_handle;
    typedef typename DDT::Tile_cell_const_iterator  Tile_cell_const_iterator;
    typedef typename DDT::Tile_const_iterator       Tile_const_iterator;
    typedef typename DDT::Vertex_const_iterator     Vertex_const_iterator;
    typedef typename DDT::Facet_const_iterator      Facet_const_iterator;

private:
    Tile_const_iterator begin_;
    Tile_const_iterator end_;
    Tile_const_iterator tile_;
    Tile_cell_const_iterator cell_;

public:
    Cell_const_iterator(Tile_const_iterator begin, Tile_const_iterator end)
        : begin_(begin), end_(end), tile_(begin), cell_()
    {
        if(tile_ != end_)
        {
            cell_ = tile_->cells_begin();
            advance_to_main();
        }
        assert(is_valid());
    }

    Cell_const_iterator(Tile_const_iterator begin, Tile_const_iterator end, Tile_const_iterator tile)
        : begin_(begin), end_(end), tile_(tile), cell_()
    {
        if(tile_ != end_)
        {
            cell_ = tile_->cells_begin();
            advance_to_main();
        }
        assert(is_valid());
    }

    Cell_const_iterator(Tile_const_iterator begin, Tile_const_iterator end, Tile_const_iterator tile, Tile_cell_const_iterator cell)
        : begin_(begin), end_(end), tile_(tile), cell_(cell)
    {
        // do not enforce main here !
        assert(is_valid());
    }

    Cell_const_iterator(const Cell_const_iterator& c)
        : begin_(c.begin_), end_(c.end_), tile_(c.tile_), cell_(c.cell_)
    {
        // do not enforce main here !
        assert(is_valid());
    }

    Cell_const_iterator& advance_to_main()
    {
        while(tile_ != end_)
        {
            if(cell_ == tile_->cells_end())
            {
                if (++tile_ != end_) cell_ = tile_->cells_begin();
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
        if (c.tile_ == c.end_) return tile_ != end_;
        if (tile_ == end_) return false;
        return  tile_->id() < c.tile_->id() || (tile_->id() == c.tile_->id() && cell_ < c.cell_);
    }

    Cell_const_iterator& operator++()
    {
        assert(tile_ != end_);
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
        assert(tile_ != end_);
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

    bool operator==(const Cell_const_iterator& rhs) const
    {
        return begin_ == rhs.begin_
               && tile_ == rhs.tile_
               && end_ == rhs.end_
               && (tile_ == end_ || cell_==rhs.cell_);
    }

    bool operator!=(const Cell_const_iterator& rhs) const { return !(*this == rhs); }
    Cell_const_iterator& operator*() { return *this; }
    Cell_const_iterator* operator->() { return this; }

    Facet_const_iterator facet(int i) const
    {
        assert(tile_ != end_);
        return Facet_const_iterator(begin_, end_, tile_, tile_->facet(cell_, i));
    }

    Cell_const_iterator neighbor(int i) const
    {
        assert(tile_ != end_);
        Tile_cell_const_iterator c = cell_->neighbor(i);
        if(!tile_->cell_is_foreign(c))
            return Cell_const_iterator(begin_, end_, tile_, c);
        // there is no representative of the neighbor in tile_
        return facet(i)->main()->neighbor()->full_cell();
    }

    int mirror_index(int i) const
    {
        assert(tile_ != end_);
        Tile_cell_const_iterator c = cell_->neighbor(i);
        if(!tile_->cell_is_foreign(c))
            return tile_->mirror_index(cell_, i);
        return facet(i)->main()->mirror_index();
    }

    Id main_id() const
    {
        assert(tile_ != end_);
        Id id = -1;
        int D = tile_->current_dimension();
        for(int i=0; i<=D; ++i)
        {
            auto v = tile_->vertex(cell_, i);
            if(tile_->vertex_is_infinite(v)) continue;
            Id vid = tile_->id(v);
            if (id==-1 || vid < id) id = vid;
        }
        return id;
    }

    Vertex_const_iterator vertex (const int i) const
    {
        assert(tile_ != end_);
        return Vertex_const_iterator(begin_, end_, tile_, tile_->vertex(cell_, i));
    }

    Cell_const_iterator main() const
    {
        assert(tile_ != end_);
        Id id = main_id();
        if (id == tile_->id()) return *this; // <=> is_main

        for (Tile_const_iterator tile = begin_; tile != end_; ++tile)
            if (id == tile->id())
            {
                Tile_cell_const_iterator c = tile->locate_cell(*tile_, cell_);
                if (c==tile->cells_end()) return Cell_const_iterator(begin_, end_, end_);
                return Cell_const_iterator(begin_, end_, tile, c);
            }

        assert(false);
        // no tile has the main id in tiles
        return Cell_const_iterator(begin_, end_, end_);
    }

    Tile_const_iterator    tile()      const { return tile_; }
    Tile_cell_const_handle full_cell() const { return cell_; }

    bool is_local()    const { return tile_->cell_is_local(cell_); }
    bool is_mixed()    const { return tile_->cell_is_mixed(cell_); }
    bool is_foreign()  const { return tile_->cell_is_foreign(cell_); }
    bool is_main()     const { return tile_->cell_is_main(cell_); }
    bool is_infinite() const { return tile_->cell_is_infinite(cell_); }

    bool is_valid()    const
    {
        return tile_ == end_ || cell_ != tile_->cells_end();
    }
};

}

#endif // CGAL_DDT_CELL_CONST_ITERATOR_H
