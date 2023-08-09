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

#ifndef CGAL_DDT_VERTEX_ITERATOR_H
#define CGAL_DDT_VERTEX_ITERATOR_H

#include <iterator>

namespace CGAL {
namespace DDT {

template<typename TileContainer>
class Vertex_iterator
{
public:
    typedef typename TileContainer::const_iterator     Tile_const_iterator;
    typedef typename TileContainer::key_type           Tile_index;
    typedef typename TileContainer::mapped_type        Tile_triangulation;
    typedef typename Tile_triangulation::Vertex_index  Tile_vertex_index;

    using iterator_category = std::forward_iterator_tag;
    using value_type = Tile_vertex_index;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

private:
    const TileContainer *tiles_;
    Tile_const_iterator tile_;
    Tile_vertex_index vertex_;

public:
    Vertex_iterator(const TileContainer *tiles, Tile_const_iterator tile)
        : tiles_(tiles), tile_(tile), vertex_()
    {
        if(tile_ != tiles_->cend())
        {
            vertex_ = triangulation().vertices_begin();
            advance_to_main();
        }
        assert(is_valid());
    }

    Vertex_iterator(const TileContainer *tiles, Tile_const_iterator tile, Tile_vertex_index vertex)
        : tiles_(tiles), tile_(tile), vertex_(vertex)
    {
        // do not enforce main here !
        assert(is_valid());
    }

    Vertex_iterator(const Vertex_iterator& v)
        : tiles_(v.tiles_), tile_(v.tile_), vertex_(v.vertex_)
    {
        // do not enforce main here !
        assert(is_valid());
    }

    Vertex_iterator& advance_to_main()
    {
        while(tile_ != tiles_->cend())
        {
            if(vertex_ == triangulation().vertices_end())
            {
                if (++tile_ != tiles_->cend())
                    vertex_ = triangulation().vertices_begin();
                else
                    vertex_ = {};
            }
            else if(triangulation().vertex_is_main(vertex_))
            {
                break;
            }
            else
            {
                ++vertex_;
            }
        }
        return *this;
    }

    Vertex_iterator& operator++()
    {
        assert(triangulation().vertex_is_main(vertex_));
        ++vertex_;
        return advance_to_main();
    }

    Vertex_iterator operator++(int)
    {
        Vertex_iterator tmp(*this);
        operator++();
        return tmp;
    }

    Vertex_iterator& operator-=(int n)
    {
        assert(triangulation().vertex_is_main(vertex_));
        if (n<0) return operator+=(-n);
        assert(!"Negative offsets are not implemented in Vertex_iterator+=");
        return *this;
    }

    Vertex_iterator& operator+=(int n)
    {
        assert(triangulation().vertex_is_main(vertex_));
        if (n<0) return operator-=(-n);
        Tile_vertex_index end = triangulation().vertices_end();
        for(; vertex_ != end; ++vertex_)
            if(triangulation().vertex_is_main(vertex_)) {
                if (n == 0) return *this;
                --n;
            }

        ++tile_;
        vertex_ = {};
        if(tile_ == tiles_->cend()) { assert(n==0); return *this; }
        int num_main_vertices = triangulation().number_of_main_finite_vertices();
        while(n >= num_main_vertices)
        {
            n -= num_main_vertices;
            ++tile_;
            if(tile_ == tiles_->cend()) { assert(n==0); return *this; }
            num_main_vertices = triangulation().number_of_main_finite_vertices();
        }

        end = triangulation().vertices_end();
        for(vertex_ = triangulation().vertices_begin(); vertex_ != end; ++vertex_)
            if(triangulation().vertex_is_main(vertex_)) {
                if (n == 0) return *this;
                --n;
            }
        ++tile_;
        assert(tile_ == tiles_->cend() && n==0);
        return *this;
    }

    bool operator==(const Vertex_iterator& v) const
    {
        if (tiles_ != v.tiles_) return false;
        if (tile_ == tiles_->cend() || v.tile_ == tiles_->cend()) return tile_ == v.tile_;
        if (tile_ == v.tile_) return vertex_==v.vertex_;
        return triangulation().are_vertices_equal(vertex_, v.triangulation(), v.vertex_);
    }

    bool operator!=(const Vertex_iterator& rhs) const { return !(*this == rhs); }

    const Tile_const_iterator&        tile() const { return tile_;   }
    const value_type&  operator*() const { return vertex_; }
    const Tile_triangulation&    triangulation() const { return tile_->second; }
    const Tile_index&            id()            const { return tile_->first; }

    bool is_valid()    const
    {
        return tile_ == tiles_->cend() || vertex_ != triangulation().vertices_end();
    }
};

}
}

#endif // CGAL_DDT_VERTEX_ITERATOR_H
