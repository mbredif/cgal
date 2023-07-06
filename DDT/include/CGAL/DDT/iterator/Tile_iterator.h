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

#ifndef CGAL_DDT_TILE_ITERATOR_H
#define CGAL_DDT_TILE_ITERATOR_H

#include <iterator>

namespace CGAL {
namespace DDT {

template<typename Container, typename Iterator>
class Tile_iterator
{
public:
    using value_type = typename Iterator::value_type;
    using iterator_category = typename Iterator::iterator_category;
    using difference_type = typename Iterator::difference_type;
    using pointer = typename Iterator::pointer;
    using reference = typename Iterator::reference;

private:
    Container *container_;
    Iterator it_;

public:

    void prepare_load() {
        if(it_ != container_->raw_end())
        {
            it_->second.use_count++;
            if (!container_->prepare_load(it_->first, it_->second))
                throw std::runtime_error("prepare_load failed");
        }
    }
    void load() const {
        if(!it_->second.load(it_->first, container_->serializer()))
            throw std::runtime_error("lazy loading failed");
    }

    Tile_iterator(Container *container, Iterator it)
        : container_(container), it_(it)
    {
        prepare_load();
    }

    ~Tile_iterator()
    {
        if(it_ != container_->raw_end())
        {
            it_->second.use_count--;
        }
    }

    Tile_iterator(const Tile_iterator& c)
        : container_(c.container_), it_(c.it_)
    {
        if(it_ != container_->raw_end())
        {
            it_->second.use_count++;
        }
    }

    bool operator<(const Tile_iterator& c) const
    {
        return  it_ < c.it_;
    }

    Tile_iterator& operator++()
    {
        it_->second.use_count--;
        ++it_;
        prepare_load();
        return *this;
    }

    Tile_iterator operator++(int)
    {
        Tile_iterator tmp(*this);
        operator++();
        return tmp;
    }

    bool operator==(const Tile_iterator& c) const
    {
        return (it_ == c.it_);
    }

    bool operator!=(const Tile_iterator& rhs) const { return !(*this == rhs); }

    const value_type& operator*() const { load(); return *it_; }

    pointer operator->() const { load(); return it_.operator->(); }
};

}
}

#endif // CGAL_DDT_TILE_ITERATOR_H
