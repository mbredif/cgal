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

#ifndef CGAL_DDT_TRAITS_FACET_CONST_ITERATOR_H
#define CGAL_DDT_TRAITS_FACET_CONST_ITERATOR_H

#include <iterator>

namespace CGAL {
namespace DDT {

namespace Impl {
template<typename Cell_index, typename Derived>
class Facet_index
{
public:
    Facet_index(                      ) : cell_(    ), index_of_covertex_(0) {}
    Facet_index(Cell_index cell, int i) : cell_(cell), index_of_covertex_(i) {}

    Facet_index& operator++()
    {
        increment();
        return (*this);
    }

    Facet_index operator++(int)
    {
        Facet_index tmp(*this);
        increment();
        return tmp;
    }

    inline bool operator==(const Facet_index & f) const
    {
        return (cell_ == f.cell_) && (index_of_covertex_ == f.index_of_covertex_);
    }

    inline bool operator!=(const Facet_index & f) const
    {
        return !(*this == f);
    }

    inline bool operator<(const Facet_index & f) const
    {
        if (cell_ < f.cell_) return true;
        if (f.cell_ < cell_) return false;
        if (index_of_covertex_ < f.index_of_covertex_) return true;
        if (f.index_of_covertex_ < index_of_covertex_) return false;
        return false;
    }

    Facet_index& operator=(const Facet_index & f)
    {
        cell_ = f.cell_;
        index_of_covertex_ = f.index_of_covertex_;
        return (*this);
    }

    Cell_index cell() const { return cell_; }
    int index_of_covertex() const { return index_of_covertex_; }

private:
    void increment()
    {
        if (index_of_covertex_ ==  static_cast<const Derived*>(this)->dimension()) {
            ++cell_;
            index_of_covertex_ = 0;
        } else {
            ++index_of_covertex_;
        }
    }

private:
    Cell_index cell_;
    int index_of_covertex_;
};

}

template<unsigned int N, typename Cell_index>
class Facet_index : public Impl::Facet_index<Cell_index, Facet_index<N, Cell_index>> {
    using Base = Impl::Facet_index<Cell_index, Facet_index<N, Cell_index>>;
public:
    Facet_index(                                 ) : Base(       ) {}
    Facet_index(Cell_index cell, int i, int dim=0) : Base(cell, i) { /* assert dim == N */ }
    constexpr int dimension() const { return N; }
};

template<typename Cell_index>
class Facet_index<0, Cell_index> : public Impl::Facet_index<Cell_index, Facet_index<0, Cell_index>> {
    using Base = Impl::Facet_index<Cell_index, Facet_index<0, Cell_index>>;
    int dim_;
public:
    Facet_index(                               ) : Base(       ), dim_( 0 ) {}
    Facet_index(Cell_index cell, int i, int dim) : Base(cell, i), dim_(dim) {}
    inline int dimension() const { return dim_; }
};

}
}

#endif // CGAL_DDT_TRAITS_FACET_CONST_ITERATOR_H
