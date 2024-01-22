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
/// This serializer may be used to save and load a distributed triangulation on disk.
/// It uses CGAL's iostream serialization for reading and writing each tile triangulation and a JSON file for the overall distributed triangulation metadata.
/// \todo most of my remarks for File_point_serializer applies here too MB: ok
/// \cgalModels{Serializer}
class File_serializer {
public:
    /// constructor
    File_serializer(const std::string& dirname = "") : dirname_(dirname)
    {
        if(dirname_.empty()) {
            dirname_ = "tmp/" + boost::filesystem::unique_path().string();
        }
        boost::filesystem::path p(dirname_);
        dirname_ = p.string()+"/";
        boost::filesystem::create_directories(p);
    }

    /// writes a tile triangulation to disk using cgal's ostream serialization, as a TXT file using CGAL's iostream serialization.
    template <class TileTriangulation>
    bool write(const TileTriangulation& tri) const
    {
        std::ofstream os(dirname_ + std::to_string(tri.id()) + ".txt");
        return write_cgal_tile(os, tri);
    }

    /// initiates the writing of a distributed triangulation to disk.
    template <typename DistributedTriangulation>
    bool write_begin(const DistributedTriangulation& tri, int) const
    {
        return true;
    }

    /// finalizes the writing of a distributed triangulation to disk, as a JSON file.
    template <typename DistributedTriangulation>
    bool write_end(const DistributedTriangulation& tri, int id) const
    {
        std::ofstream os(dirname_ + std::to_string(id) + ".json");
        return write_json_tiles(os, tri);
    }

    /// tests whether a tile is readable, given its index.
    template <typename TileIndex>
    bool is_readable(TileIndex id) const
    {
        std::ifstream is(dirname_ + std::to_string(id) + ".txt", std::ios::in);
        return is.is_open();
    }

    /// reads in place a tile from disk, using its index `tri.id()`.
    template<typename TileTriangulation>
    bool read(TileTriangulation& tri) const
    {
        std::ifstream is(dirname_ + std::to_string(tri.id()) + ".txt", std::ios::in);
        return read_cgal_tile(is, tri);
    }

    /// initiates the reading of distributed triangulation from disk.
    template <typename DistributedTriangulation>
    bool read_begin(DistributedTriangulation& tri, int id) const
    {
        std::ifstream is(dirname_ + std::to_string(id) + ".json", std::ios::in);
        return read_cgal_json(is, tri);
    }

    /// terminates the reading of distributed triangulation from disk.
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
