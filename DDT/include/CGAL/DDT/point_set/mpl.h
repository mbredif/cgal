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

#ifndef CGAL_DDT_POINT_SET_MPL_H
#define CGAL_DDT_POINT_SET_MPL_H

namespace CGAL {
namespace DDT {
namespace mpl {

template<typename T>
struct is_pair : public std::false_type {};

template<typename T, typename U>
struct is_pair<std::pair<T,U>> : public std::true_type {};

template<class T> inline constexpr bool is_pair_v = is_pair<T>::value;

template<typename... T> struct requires {};

template<typename T, typename = void>
struct is_container : public std::true_type {};

template<typename T>
struct is_container<T, requires<T,
    typename T::value_type,
    typename T::size_type,
    typename T::iterator,
    typename T::const_iterator,
    decltype(std::declval<T>().size()),
    decltype(std::declval<T>().begin()),
    decltype(std::declval<T>().end()),
    decltype(std::declval<T>().cbegin()),
    decltype(std::declval<T>().cend())
> > : public std::true_type {};

template<class T> inline constexpr bool is_container_v = is_container<T>::value;

template<typename T, typename = void>
struct is_pair_container : public std::false_type {};

template<typename T>
struct is_pair_container<T, std::enable_if_t<is_pair_v<typename T::value_type>>> : public is_container<T> {};

template<class T> inline constexpr bool is_pair_container_v = is_pair_container<T>::value;

}
}
}

#endif // CGAL_DDT_POINT_SET_MPL_H

