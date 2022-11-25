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

struct No_info {};

/// \ingroup PkgDDTMisc
/// A pair used to store the Id and (optional) Info data of a vertex in the distributed Delaunay triangulation
template<typename I, typename F>
struct Data
{
    typedef I Id;
    typedef F Info;
    Data() : id(), info() {}
    Id id;
    Info info;
};

template<typename I>
struct Data<I, No_info>
{
    typedef I Id;
    Data() {}
    Id id;
};

template<typename T> std::ostream &write_data_element(std::ostream& os, T t) {
    if(CGAL::is_ascii(os))
        os << " " << t;
    else
        os.write((char*)(&t), sizeof(T));
    return os;
}

template<typename T> std::istream &read_data_element(std::istream& is, T& t) {
    if(CGAL::is_ascii(is))
        is >> t;
    else
        is.read((char*)(&t), sizeof(T));
    return is;
}

template<> std::ostream &write_data_element<unsigned char>(std::ostream& os, unsigned char t) {
    if(CGAL::is_ascii(os))
        os << " " << int(t);
    else
        os.write((char*)(&t), sizeof(unsigned char));
    return os;
}

template<> std::istream &read_data_element<unsigned char>(std::istream& is, unsigned char& t) {
    if(CGAL::is_ascii(is)) {
        int i;
        is >> i;
        t = (unsigned char) i;
    } else
        is.read((char*)(&t), sizeof(unsigned char));
    return is;
}

template< typename Id, typename Info >
std::ostream &
operator<<(std::ostream & os, const Data<Id, Info>& data)
{
    write_data_element(os, data.id);
    write_data_element(os, data.info);
    return os;
}

template< typename Id, typename Info >
std::istream &
operator>>(std::istream & is, Data<Id, Info>& data)
{
    read_data_element(is, data.id);
    read_data_element(is, data.info);
    return is;
}

template< typename Id >
std::ostream &
operator<<(std::ostream & os, const Data<Id, No_info>& data)
{
    return write_data_element(os, data.id);
}

template< typename Id >
std::istream &
operator>>(std::istream & is, Data<Id, No_info>& data)
{
    return read_data_element(is, data.id);
}

}
}

#endif // CGAL_DDT_DATA_H
