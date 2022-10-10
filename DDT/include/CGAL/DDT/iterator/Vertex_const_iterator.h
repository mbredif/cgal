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

namespace ddt
{

template<typename DDT>
class Vertex_const_iterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = Vertex_const_iterator<DDT>;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    typedef typename DDT::Traits                     Traits;
    typedef typename DDT::Tile_vertex_const_iterator Tile_vertex_const_iterator;
    typedef typename DDT::Tile_const_iterator        Tile_const_iterator;

private:
    Tile_const_iterator tile_;
    Tile_const_iterator end_;
    Tile_vertex_const_iterator vertex_;

public:
    Vertex_const_iterator(Tile_const_iterator begin, Tile_const_iterator end)
        : tile_(begin), end_(end), vertex_()
    {
        if(begin != end)
        {
            // if(!ddt_.is_loaded(*id_)) ddt_.load(*id_);
            // tile_ = ((const DDT&)(ddt_)).get_tile(*id_); /// @todo constness
            vertex_ = tile_->vertices_begin();
            advance_to_main();
        }
    }

    Vertex_const_iterator(Tile_const_iterator begin, Tile_const_iterator end,
        Tile_vertex_const_iterator vertex)
        : tile_(begin), end_(end), vertex_(vertex)
    {
        // do not enforce main here !
    }

    Vertex_const_iterator(const Vertex_const_iterator& v)
        : tile_(v.tile_), end_(v.end_), vertex_(v.vertex_)
    {
        // do not enforce main here !
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
        //assert(id_ != ddt_.tile_ids_end());
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
        //assert(id_ != ddt_.tile_ids_end());
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

    bool operator==(const Vertex_const_iterator& rhs) const
    {
        return    tile_ == rhs.tile_
               && end_ == rhs.end_
               && (tile_ == end_ || vertex_ == rhs.vertex_);
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

#endif // CGAL_DDT_VERTEX_CONST_ITERATOR_H
