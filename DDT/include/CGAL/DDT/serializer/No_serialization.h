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

#ifndef CGAL_DDT_NO__SERIALIZATION_H
#define CGAL_DDT_NO__SERIALIZATION_H

namespace CGAL {
namespace DDT {

template<typename Tile>
struct No_serialization
{
  Tile load(typename Tile::Id Id) const {}
  bool save(const Tile& ) const { return false;}
};

} }  // CGAL::DDT namespace

#endif // CGAL_DDT_NO__SERIALIZATION_H
