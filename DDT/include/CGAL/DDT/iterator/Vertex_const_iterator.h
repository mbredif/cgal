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

#ifndef CGAL_DDT_VERTEX_CONST_ITERATOR_H
#define CGAL_DDT_VERTEX_CONST_ITERATOR_H

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTIteratorClasses
/// A const iterator to the vertex of a distributed Delaunay triangulation
template<typename TileContainer>
class Vertex_const_iterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = Vertex_const_iterator<TileContainer>; /// @todo: unused, no operator*(), do we need a cell handle?
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    typedef typename TileContainer::Traits                     Traits;
    typedef typename TileContainer::Tile_vertex_const_iterator Tile_vertex_const_iterator;
    typedef typename TileContainer::Tile_const_iterator        Tile_const_iterator;

private:
    Tile_const_iterator tile_;
    Tile_const_iterator end_;
    Tile_vertex_const_iterator vertex_;

public:
    Vertex_const_iterator(Tile_const_iterator tile, Tile_const_iterator end)
        : tile_(tile), end_(end), vertex_()
    {
        if(tile_ != end_)
        {
            vertex_ = tile_->vertices_begin();
            advance_to_main();
        }
        assert(is_valid());
    }

    Vertex_const_iterator(Tile_const_iterator tile, Tile_const_iterator end, Tile_vertex_const_iterator vertex)
        : tile_(tile), end_(end), vertex_(vertex)
    {
        // do not enforce main here !
        assert(is_valid());
    }

    Vertex_const_iterator(const Vertex_const_iterator& v)
        : tile_(v.tile_), end_(v.end_), vertex_(v.vertex_)
    {
        // do not enforce main here !
        assert(is_valid());
    }

    Vertex_const_iterator& advance_to_main()
    {
        while(tile_ != end_)
        {
            if(vertex_ == tile_->vertices_end())
            {
                if (++tile_ != end_) {
                    //if(!ddt_.is_loaded(*id_)) ddt_.load(*id_);
                    //tile_ = ((const DDT&)(ddt_)).get_tile(*id_); /// @todo constness
                    vertex_ = tile_->vertices_begin();
                }
            }
            else if(tile_->vertex_is_main(vertex_))
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

    Vertex_const_iterator& operator++()
    {
        ++vertex_;
        return advance_to_main();
    }

    Vertex_const_iterator operator++(int)
    {
        Vertex_const_iterator tmp(*this);
        operator++();
        return tmp;
    }

    Vertex_const_iterator& operator+=(int n)
    {
        for(auto vit = tile_->vertices_begin(); vit != vertex_; ++vit)
            if(tile_->vertex_is_main(vit))
                ++n;
        int num_main_vertices = tile_->number_of_main_vertices();
        while(n >= num_main_vertices)
        {
            n -= num_main_vertices;
            ++tile_;
            num_main_vertices = tile_->number_of_main_vertices();
        }
        vertex_ = tile_->vertices_begin();
        advance_to_main();
        for(; n>0 ; --n)
            ++(*this);
        return *this;
    }

    bool operator==(const Vertex_const_iterator& v) const
    {
        if (end_ != v.end_) return false;
        if (tile_ == end_ || v.tile_ == end_) return tile_ == v.tile_; // == end_ == v.end_
        if (tile_ == v.tile_) return vertex_==v.vertex_;
        return tile_->are_vertices_equal(vertex_, *(v.tile_), v.vertex_);
    }

    bool operator!=(const Vertex_const_iterator& rhs) const { return !(*this == rhs); }

    const Tile_const_iterator&        tile  () const { return tile_;   }
    const Tile_vertex_const_iterator& vertex() const { return vertex_; }

    bool is_valid()    const
    {
        return tile_ == end_ || vertex_ != tile_->vertices_end();
    }
};

}
}

#endif // CGAL_DDT_VERTEX_CONST_ITERATOR_H
