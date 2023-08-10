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

template<typename Container, typename PairIterator>
class Tile_iterator
{
    using iterator_type = typename PairIterator::value_type::second_type::iterator_type;
    using Tile_const_iterator = Tile_iterator<std::add_const_t<Container>, PairIterator>;

public:
    using value_type = typename iterator_type::value_type;
    using iterator_category = typename PairIterator::iterator_category;
    using difference_type = typename PairIterator::difference_type;
    using pointer = typename iterator_type::pointer;
    using reference = typename iterator_type::reference;
    friend Tile_const_iterator;

private:
    Container *container_;
    PairIterator it_;


    void prepare_load() {
        if(it_ != container_->usages_end())
        {
            it_->second.use_count++;
            if (!container_->prepare_load(it_->second))
                throw std::runtime_error("prepare_load failed");
        }
    }
    void load() const {
        if(!it_->second.load(container_->serializer()))
            throw std::runtime_error("lazy loading failed");
    }

public:

    Tile_iterator(Container *container, PairIterator it)
        : container_(container), it_(it)
    {
        prepare_load();
    }

    ~Tile_iterator()
    {
        if(it_ != container_->usages_end())
        {
            it_->second.use_count--;
        }
    }

    // enable non-const -> const copy construction
    template<typename C, typename = std::enable_if_t<std::is_convertible_v<C, Container>>>
    Tile_iterator(const Tile_iterator<C, PairIterator>& t)
        : container_(t.container_), it_(t.it_)
    {
        if(it_ != container_->usages_end())
        {
            it_->second.use_count++;
        }
    }

    Tile_iterator(const Tile_iterator& t)
        : container_(t.container_), it_(t.it_)
    {
        if(it_ != container_->usages_end())
        {
            it_->second.use_count++;
        }
    }

    bool operator<(const Tile_iterator& t) const
    {
        return  it_ < t.it_;
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

    bool operator==(const Tile_iterator& t) const
    {
        return (it_ == t.it_);
    }

    bool operator!=(const Tile_iterator& rhs) const { return !(*this == rhs); }

    const value_type& operator*() const { load(); return *(it_->second); }
    pointer operator->() const { load(); return it_->second.operator->(); }
};

}
}

#endif // CGAL_DDT_TILE_ITERATOR_H
