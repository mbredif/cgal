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

#ifndef CGAL_DDT_FILE_SERIALIZER_H
#define CGAL_DDT_FILE_SERIALIZER_H

#include <boost/filesystem.hpp>
#include <CGAL/DDT/IO/read_cgal.h>
#include <CGAL/DDT/IO/write_cgal.h>

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTSerializerClasses
/// This serializer saves and loads the bounding box and triangulation of each tile to the filesystem.
/// It contains the iostream serialization of the bounding box and the tile triangulation.
/// \cgalModels Serializer
class File_serializer {
public:
    File_serializer(const std::string& dirname) : dirname_(dirname)
    {
        boost::filesystem::path p(dirname_);
        boost::filesystem::create_directories(p);
    }

    template <class TileTriangulation>
    bool write(const TileTriangulation& tri) const
    {
        return write_cgal_tile(tri, dirname_);
    }

    template <typename DistributedTriangulation>
    bool write_begin(const DistributedTriangulation& tri) const
    {
        return true;
    }

    template <typename DistributedTriangulation>
    bool write_end(const DistributedTriangulation& tri) const
    {
        return write_json_tiles(tri, dirname_);
    }

    template <typename TileIndex>
    bool readable(TileIndex id) const
    {
        std::cout << dirname_ << std::endl;
        std::cout << "/" << std::endl;
        std::cout << std::to_string(id) << std::endl;
        std::cout << std::endl;
        const std::string fname = dirname_ + "/" + std::to_string(id) + ".txt";
        std::ifstream in(fname, std::ios::in);
        return in.is_open();
    }

    template<typename TileTriangulation>
    bool read(TileTriangulation& tri) const
    {
        return read_cgal_tile(tri, dirname_);
    }

    template <typename DistributedTriangulation>
    bool read_begin(DistributedTriangulation& tri) const
    {
        return read_json_tiles(tri, dirname_);
    }

    template <typename DistributedTriangulation>
    bool read_end(DistributedTriangulation& tri) const
    {
        return true;
    }

    /// File system directory name
    const std::string& dirname() const { return dirname_; }

private:
    std::string dirname_;
};

std::ostream& operator<<(std::ostream& out, const File_serializer& serializer) {
    return out << "File_serializer(dirname=" << serializer.dirname() << ")";
}

}
}

#endif // CGAL_DDT_FILE_SERIALIZER_H
