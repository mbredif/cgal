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

#ifndef CGAL_DDT_DATA_H
#define CGAL_DDT_DATA_H

#include <iostream>
#include <CGAL/IO/io.h>

namespace CGAL {
namespace DDT {

template<typename I, typename F>
struct Data
{
    typedef I Id;
    typedef F Flag;
    Data(Id i=0, Flag f=0) : id(i), flag(f) {}

    void write(std::ostream& os) const
    {
        if(CGAL::is_ascii(os))
        {
            os << " " << id << " "  << flag;
        }
        else
        {
            os.write((char*)(&(id)), sizeof(id));
            os.write((char*)(&(flag)), sizeof(flag));
        }
    }
    void read(std::istream& is)
    {
        if(CGAL::is_ascii(is))
        {
            is >> id >> flag;
        }
        else
        {
            is.read((char*)(&(id)), sizeof(id));
            is.read((char*)(&(flag)), sizeof(flag));
        }
    }
    Id id;
    mutable Flag flag;
};


template< typename I, typename F>
std::ostream &
operator<<(std::ostream & os, const Data<I,F> & dd)
{
    dd.write(os);
    return os;
}

template< typename I, typename F>
std::istream &
operator>>(std::istream & is,  Data<I,F> & dd)
{
    dd.read(is);
    return is;
}

}
}

#endif // CGAL_DDT_DATA_H
