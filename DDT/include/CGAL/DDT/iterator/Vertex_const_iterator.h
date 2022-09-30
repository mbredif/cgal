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
    typedef typename DDT::Point                      Point;
    typedef typename DDT::Tile_id_set_const_iterator Tile_id_set_const_iterator;

private:
    DDT& ddt_;
    Tile_id_set_const_iterator id_;
    Tile_const_iterator tile_;
    Tile_vertex_const_iterator vertex_;

public:
    Vertex_const_iterator(DDT& ddt, Tile_id_set_const_iterator id)
        : ddt_(ddt), id_(id), tile_(), vertex_()
    {
        if(id_ != ddt.tile_ids_end())
        {
            if(!ddt_.is_loaded(*id_)) ddt_.load(*id_);
            tile_ = ((const DDT&)(ddt_)).get_tile(*id_); /// @todo constness
            vertex_ = tile_->vertices_begin();
            advance_to_main();
        }
    }

    Vertex_const_iterator(DDT& ddt, Tile_id_set_const_iterator id,
                          Tile_const_iterator tile, Tile_vertex_const_iterator vertex)
        : ddt_(ddt), id_(id), tile_(tile), vertex_(vertex)
    {
        // do not enforce main here !
    }

    Vertex_const_iterator(const Vertex_const_iterator& v)
        : ddt_(v.ddt_), id_(v.id_), tile_(v.tile_), vertex_(v.vertex_)
    {
        // do not enforce main here !
    }

    Vertex_const_iterator& advance_to_main()
    {
        while(id_ != ddt_.tile_ids_end())
        {
            if(vertex_ == tile_->vertices_end())
            {
                if (++id_ != ddt_.tile_ids_end()) {
                    if(!ddt_.is_loaded(*id_)) ddt_.load(*id_);
                    tile_ = ((const DDT&)(ddt_)).get_tile(*id_); /// @todo constness
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
        assert(id_ != ddt_.tile_ids_end());
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
        assert(id_ != ddt_.tile_ids_end());
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
        return &ddt_ == &(rhs.ddt_)
               && tile_ == rhs.tile_
               && id_ == rhs.id_
               && (id_ == ddt_.tile_ids_end() || vertex_ == rhs.vertex_);
    }

    bool operator!=(const Vertex_const_iterator& rhs) const { return !(*this == rhs); }
    Vertex_const_iterator& operator*() { return *this; }
    Vertex_const_iterator* operator->() { return this; }

    Id main_id() const
    {
        assert(id_ == ddt_.tile_ids_end());
        return tile_->id(vertex_);
    }

    Tile_const_iterator        tile  () const { return tile_;   }
    Tile_vertex_const_iterator vertex() const { return vertex_; }
    DDT& ddt() { return ddt_; }

    bool is_local()    const { return tile_->vertex_is_local(vertex_); }
    bool is_foreign()  const { return tile_->vertex_is_foreign(vertex_); }
    bool is_main()     const { return tile_->vertex_is_main(vertex_); }
    bool is_infinite() const { return tile_->vertex_is_infinite(vertex_); }

    bool is_valid()    const
    {
        return id_ == ddt_.tile_ids_end() || vertex_ != tile_->vertices_end();
    }
};

}

#endif // CGAL_DDT_VERTEX_CONST_ITERATOR_H
