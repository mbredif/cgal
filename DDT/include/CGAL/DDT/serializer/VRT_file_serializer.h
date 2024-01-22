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

/// \ingroup PkgDDTSerializerClasses
/// This serializer is used to save a distributed triangulation on disk using the VRT and CSV formats from GDAL, for GIS applications such as https://www.qgis.org .
/// https://gdal.org/drivers/vector/vrt.html
/// \cgalModels{Serializer}
class VRT_serializer {
public:
    /// constructor.
    /// \param dirname the directory path.
    /// \param tins enables the `wkbTIN` export of the tile triangulations.
    /// \param verts enables the `wkbPoint` export of the tile triangulation vertices.
    /// \param facets enables the `wkbLineString` export of the tile triangulation facets.
    /// \param cells enables the `wkbPolygon` export of the tile triangulation cells.
    VRT_serializer(const std::string& dirname, bool tins = true, bool verts = true, bool facets = true, bool cells = true)
    : dirname_(dirname), tins(tins), verts(verts), facets(facets), cells(cells)
    {
        boost::filesystem::create_directories(dirname_+"/v");
        boost::filesystem::create_directories(dirname_+"/f");
        boost::filesystem::create_directories(dirname_+"/c");
        boost::filesystem::create_directories(dirname_+"/t");
    }

    /// writes the tile triangulation as VRT+CSV file pairs.
    template <class TileTriangulation>
    bool write(const TileTriangulation& tri) const
    {
        if (verts  && !write_tile_vrt_verts (dirname_+"/v", tri)) return false;
        if (facets && !write_tile_vrt_facets(dirname_+"/f", tri)) return false;
        if (cells  && !write_tile_vrt_cells (dirname_+"/c", tri)) return false;
        if (tins   && !write_tile_vrt_tins  (dirname_+"/t", tri)) return false;
        return true;
    }

    /// initiates writing the distributed triangulation.
    template <typename DistributedTriangulation>
    bool write_begin(const DistributedTriangulation& tri, int) const
    {
        return true;
    }

    /// terminates writing the distributed triangulation, as a union VRT file.
    template <typename DistributedTriangulation>
    bool write_end(const DistributedTriangulation& tri, int id) const
    {
        std::string name = std::to_string(id);
        if (verts  && !write_union_vrt_header(dirname_, "v", name, "wkbPoint",      "vertices", tri, false)) return false;
        if (facets && !write_union_vrt_header(dirname_, "f", name, "wkbLineString", "facets",   tri, true )) return false;
        if (cells  && !write_union_vrt_header(dirname_, "c", name, "wkbPolygon",    "cells",    tri, true )) return false;
        if (tins   && !write_union_vrt_header(dirname_, "t", name, "wkbTIN",        "tins",     tri, false)) return false;
        return true;
    }

    /// File system directory name.
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
