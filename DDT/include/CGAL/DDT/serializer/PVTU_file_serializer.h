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

#ifndef CGAL_DDT_PVTU_FILE_SERIALIZER_H
#define CGAL_DDT_PVTU_FILE_SERIALIZER_H

#include <CGAL/DDT/IO/write_pvtu.h>

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTSerializerClasses
/// This serializer is used to save a distributed triangulation on disk using the PVTU format from VTK, which is supported by https://www.paraview.org/ .
/// https://docs.vtk.org/en/latest/design_documents/VTKFileFormats.html#punstructuredgrid
/// \cgalModels{Serializer}
class PVTU_serializer {
public:
    /// constructor
    /// \param dirname directory
    /// \param binary selects VTU binary or ascii mode
    PVTU_serializer(const std::string& dirname, bool binary = true) : dirname_(dirname), binary_(binary)
    {
        boost::filesystem::path p(dirname_);
        boost::filesystem::create_directories(p);
    }

    /// writes the tile triangulation as a VTU file.
    template <class TileTriangulation>
    bool write(const TileTriangulation& tri) const
    {
        std::ofstream os(dirname_ + "/" + std::to_string(tri.id())+".vtu");
        write_vtu_tile(os, tri, binary_);
        return true;
    }

    /// initiates writing the distributed triangulation.
    template <typename DistributedTriangulation>
    bool write_begin(const DistributedTriangulation& tri, int) const
    {
        return true;
    }

    /// terminates writing the distributed triangulation, as a PVTU file.
    template <typename DistributedTriangulation>
    bool write_end(const DistributedTriangulation& tri, int id) const
    {
        std::ofstream os(dirname_ + "/" + std::to_string(id)+".pvtu");
        return write_pvtu(os, tri);
    }

    /// File system directory name.
    const std::string& dirname() const { return dirname_; }

private:
    std::string dirname_;
    bool binary_;
};


std::ostream& operator<<(std::ostream& out, const PVTU_serializer& serializer) {
    return out << "PVTU_serializer(dirname=" << serializer.dirname() << ")";
}

}
}

#endif // CGAL_DDT_FILE_SERIALIZER_H
