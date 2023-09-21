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
/// This serializer saves and loads the triangulation of each tile to the filesystem.
/// It contains the iostream serialization of the tile triangulation.
/// \cgalModels{Serializer}
class File_serializer {
public:
    File_serializer(const std::string& dirname = "") : dirname_(dirname)
    {
        if(dirname_.empty()) {
            dirname_ = "tmp/" + boost::filesystem::unique_path().string();
        }
        boost::filesystem::path p(dirname_);
        dirname_ = p.string()+"/";
        boost::filesystem::create_directories(p);
    }

    template <class TileTriangulation>
    bool write(const TileTriangulation& tri) const
    {
        std::ofstream os(dirname_ + std::to_string(tri.id()) + ".txt");
        return write_cgal_tile(os, tri);
    }

    template <typename DistributedTriangulation>
    bool write_begin(const DistributedTriangulation& tri, int) const
    {
        return true;
    }

    template <typename DistributedTriangulation>
    bool write_end(const DistributedTriangulation& tri, int id) const
    {
        std::ofstream os(dirname_ + std::to_string(id) + ".json");
        return write_json_tiles(os, tri);
    }

    template <typename TileIndex>
    bool is_readable(TileIndex id) const
    {
        std::ifstream is(dirname_ + std::to_string(id) + ".txt", std::ios::in);
        return is.is_open();
    }

    template<typename TileTriangulation>
    bool read(TileTriangulation& tri) const
    {
        std::ifstream is(dirname_ + std::to_string(tri.id()) + ".txt", std::ios::in);
        return read_cgal_tile(is, tri);
    }

    template <typename DistributedTriangulation>
    bool read_begin(DistributedTriangulation& tri, int id) const
    {
        std::ifstream is(dirname_ + std::to_string(id) + ".json", std::ios::in);
        return read_cgal_json(is, tri);
    }

    template <typename DistributedTriangulation>
    bool read_end(DistributedTriangulation& tri, int id) const
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
