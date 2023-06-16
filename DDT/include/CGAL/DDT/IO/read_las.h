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

#ifndef CGAL_DDT_READ_LAS_H
#define CGAL_DDT_READ_LAS_H

#include <CGAL/IO/read_las_points.h>
#include <CGAL/Bbox_3.h>

namespace CGAL {
namespace DDT {
template<typename Point>
read_LAS_header(const std::string& fname, std::size_t& npoints, Point& pmin, Point& pmax)
{
  std::ifstream is(fname, std::ios::binary);
  CGAL::IO::set_mode(is, CGAL::IO::BINARY);
  if(!is) return false;

  LASreaderLAS lasreader;
  lasreader.open(is);
  npoints = lasreader.npoints;
  pmin = Point(lasreader.get_min_x(), lasreader.get_min_y(), lasreader.get_min_z());
  pmax = Point(lasreader.get_max_x(), lasreader.get_max_y(), lasreader.get_max_z());
  return true;
}

}
}

#endif // CGAL_DDT_READ_CGAL_H
