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

template<typename Container, typename UseIterator, typename Iterator>
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
    UseIterator usage_;
    Iterator value_;

public:

    void prepare_load() {
        if(usage_ != container_->usages_end())
        {
            usage_->second.use_count++;
            if (!container_->prepare_load(usage_->first, usage_->second))
                throw std::runtime_error("prepare_load failed");
        }
    }
    void load() const {
        if(!usage_->second.load(value_->first, value_->second, container_->serializer()))
            throw std::runtime_error("lazy loading failed");
    }

    Tile_iterator(Container *container, UseIterator use, Iterator value)
        : container_(container), usage_(use), value_(value)
    {
        prepare_load();
    }

    ~Tile_iterator()
    {
        if(usage_ != container_->usages_end())
        {
            usage_->second.use_count--;
        }
    }

    Tile_iterator(const Tile_iterator& c)
        : container_(c.container_), usage_(c.usage_), value_(c.value_)
    {
        if(usage_ != container_->usages_end())
        {
            usage_->second.use_count++;
        }
    }

    bool operator<(const Tile_iterator& c) const
    {
        return  value_ < c.value_;
    }

    Tile_iterator& operator++()
    {
        usage_->second.use_count--;
        ++value_;
        ++usage_;
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
        return (value_ == c.value_);
    }

    bool operator!=(const Tile_iterator& rhs) const { return !(*this == rhs); }

    const value_type& operator*() const { load(); return *value_; }

    pointer operator->() const { load(); return value_.operator->(); }
};

}
}

#endif // CGAL_DDT_TILE_ITERATOR_H
