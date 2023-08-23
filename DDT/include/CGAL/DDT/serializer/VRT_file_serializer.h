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

#ifndef CGAL_DDT_VRT_FILE_SERIALIZER_H
#define CGAL_DDT_VRT_FILE_SERIALIZER_H

#include <CGAL/DDT/IO/write_vrt.h>

namespace CGAL {
namespace DDT {


class VRT_serializer {
public:
    VRT_serializer(const std::string& dirname, bool tins = true, bool verts = true, bool facets = true, bool cells = true)
    : dirname_(dirname), tins(tins), verts(verts), facets(facets), cells(cells)
    {
        boost::filesystem::create_directories(dirname_+"_v");
        boost::filesystem::create_directories(dirname_+"_f");
        boost::filesystem::create_directories(dirname_+"_c");
        boost::filesystem::create_directories(dirname_+"_t");
    }

    template <class TileTriangulation>
    bool write(const TileTriangulation& tri) const
    {
        std::string s("/" + std::to_string(tri.id()));
        if (verts  && !write_tile_vrt_verts (dirname_+"_v"+s, tri)) return false;
        if (facets && !write_tile_vrt_facets(dirname_+"_f"+s, tri)) return false;
        if (cells  && !write_tile_vrt_cells (dirname_+"_c"+s, tri)) return false;
        if (tins   && !write_tile_vrt_tins  (dirname_+"_t"+s, tri)) return false;
        return true;
    }

    template <typename DistributedTriangulation>
    bool write_begin(const DistributedTriangulation& tri) const
    {
        return true;
    }

    template <typename DistributedTriangulation>
    bool write_end(const DistributedTriangulation& tri) const
    {
        if (verts  && !write_union_vrt_header(dirname_+"_v", "wkbPoint",      "vertices", tri, false)) return false;
        if (facets && !write_union_vrt_header(dirname_+"_f", "wkbLineString", "facets",   tri, true )) return false;
        if (cells  && !write_union_vrt_header(dirname_+"_c", "wkbPolygon",    "cells",    tri, true )) return false;
        if (tins   && !write_union_vrt_header(dirname_+"_t", "wkbTIN",        "tins",     tri, false)) return false;
        return true;
    }

    /// File system directory name
    const std::string& dirname() const { return dirname_; }

    bool tins, verts, facets, cells;
private:
    std::string dirname_;
};


std::ostream& operator<<(std::ostream& out, const VRT_serializer& serializer) {
    return out << "VRT_serializer(dirname=" << serializer.dirname() << ")";
}

}
}

#endif // CGAL_DDT_FILE_SERIALIZER_H
