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
// Author(s)     : Sylvain Pion

#ifndef CGAL_TRIANGULATION_VERTEX_BASE_WITH_INFO_3_H
#define CGAL_TRIANGULATION_VERTEX_BASE_WITH_INFO_3_H

#include <CGAL/license/Triangulation_3.h>
#include <CGAL/Triangulation_vertex_base_3.h>

namespace CGAL {

template < typename Info_, typename GT,
           typename Vb = Triangulation_vertex_base_3<GT> >
class DDT_vertex_base_with_info_3
  : public Vb
{
  Info_ _info;
public:

  typedef typename Vb::Cell_handle                   Cell_handle;
  typedef typename Vb::Point                         Point;
  typedef Info_                                      Info;

  template < typename TDS2 >
  struct Rebind_TDS {
    typedef typename Vb::template Rebind_TDS<TDS2>::Other          Vb2;
    typedef DDT_vertex_base_with_info_3<Info, GT, Vb2>   Other;
  };

  DDT_vertex_base_with_info_3()
    : Vb() {}

  DDT_vertex_base_with_info_3(const Point & p)
    : Vb(p) {}

  DDT_vertex_base_with_info_3(const Point & p, Cell_handle c)
    : Vb(p, c) {}

  DDT_vertex_base_with_info_3(Cell_handle c)
    : Vb(c) {}

  const Info& info() const { return _info; }
  Info&       info()       { return _info; }
};

template < typename Info_, typename GT,
           typename Vb = Triangulation_vertex_base_3<GT> >
std::istream& operator>> (std::istream& is,DDT_vertex_base_with_info_3<Info_,GT,Vb> & vb)
{
  is >> static_cast<Vb&>(vb);
  is >> vb.info();
  return is;
}

template < typename Info_, typename GT,
           typename Vb = Triangulation_vertex_base_3<GT> >
std::ostream& operator<< (std::ostream& os,DDT_vertex_base_with_info_3<Info_,GT,Vb> & vb)
{
  os << static_cast<Vb&>(vb);	     
  os << vb.info();
  return os;
}

  
} //namespace CGAL

#endif // CGAL_TRIANGULATION_VERTEX_BASE_WITH_INFO_3_H

