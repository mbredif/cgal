// Copyright (c) 2003  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Mariette Yvinec, Sylvain Pion

#ifndef CGAL_TRIANGULATION_VERTEX_BASE_WITH_INFO_2_H
#define CGAL_TRIANGULATION_VERTEX_BASE_WITH_INFO_2_H

#include <CGAL/license/Triangulation_2.h>


#include <CGAL/Triangulation_vertex_base_2.h>

namespace CGAL {

template < typename Info_, typename GT,
           typename Vb = Triangulation_vertex_base_2<GT> >
class Triangulation_vertex_base_with_info_2
  : public Vb
{
  Info_ _info;

public:
  typedef typename Vb::Face_handle                   Face_handle;
  typedef typename Vb::Point                         Point;
  typedef Info_                                      Info;

  template < typename TDS2 >
  struct Rebind_TDS {
    typedef typename Vb::template Rebind_TDS<TDS2>::Other          Vb2;
    typedef Triangulation_vertex_base_with_info_2<Info, GT, Vb2>   Other;
  };

  Triangulation_vertex_base_with_info_2()
    : Vb() {}

  Triangulation_vertex_base_with_info_2(const Point & p)
    : Vb(p) {}

  Triangulation_vertex_base_with_info_2(const Point & p, Face_handle c)
    : Vb(p, c) {}


  Triangulation_vertex_base_with_info_2(Face_handle c)
    : Vb(c) {}

  const Info& info() const { return _info; }
  Info&       info()       { return _info; }
};


template < typename Info_, typename GT,
           typename Vb = Triangulation_vertex_base_2<GT> >
std::istream& operator>> (std::istream& is,Triangulation_vertex_base_with_info_2<Info_,GT,Vb> & vb)
{
  is >> static_cast<Vb&>(vb);
  is >> vb.info();
  return is;
}

template < typename Info_, typename GT,
           typename Vb = Triangulation_vertex_base_2<GT> >
std::ostream& operator<< (std::ostream& os,Triangulation_vertex_base_with_info_2<Info_,GT,Vb> & vb)
{
  os << static_cast<Vb&>(vb);
  if(IO::get_mode(os) == IO::BINARY) {
      write(os, vb.info());
  } else {
      os << ' ' << vb.info();
  }
  return os;
}


} //namespace CGAL

#endif // CGAL_TRIANGULATION_VERTEX_BASE_WITH_INFO_2_H
