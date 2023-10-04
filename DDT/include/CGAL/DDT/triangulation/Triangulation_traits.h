// Copyright (c) 2022 Institut Géographique National - IGN (France)
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Mathieu Brédif

#ifndef CGAL_DDT_TILE_TRIANGULATION_TRAITS_H
#define CGAL_DDT_TILE_TRIANGULATION_TRAITS_H

namespace CGAL {
namespace DDT {

template<typename T>
struct Triangulation_traits
{};

template<typename T, typename = void>
struct is_triangulation : public std::false_type {};

template<typename T>
struct is_triangulation<T, typename Triangulation_traits<T>::Point> : public std::true_type {};
template<class T> inline constexpr bool is_triangulation_v = is_triangulation<T>::value;

}
}

#endif // CGAL_DDT_TILE_TRIANGULATION_TRAITS_H
