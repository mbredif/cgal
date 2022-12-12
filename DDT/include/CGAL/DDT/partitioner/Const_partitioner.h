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

#ifndef CGAL_DDT_PARITIONER_CONST_PARTITIONER_H
#define CGAL_DDT_PARITIONER_CONST_PARTITIONER_H

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTPartitionerClasses
/// \cgalModels Partitioner
template<typename Traits>
class Const_partitioner
{
public:
    typedef typename Traits::Point Point;
    typedef typename Traits::Id    Id;

    Const_partitioner(Id id) : id_(id) {}

    inline Id operator()(const Point& p) const { return id_;}
    inline Id id() const { return id_; }
    constexpr std::size_t size() const { return 1; }

private:
    Id id_;
};

template<typename Traits>
std::ostream& operator<<(std::ostream& out, const Const_partitioner<Traits>& partitioner) {
    return out << "Const_partitioner( " << partitioner.id() << " )";
}

}
}

#endif // CGAL_DDT_PARITIONER_CONST_PARTITIONER_H
