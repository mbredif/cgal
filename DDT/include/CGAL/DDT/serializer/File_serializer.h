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

namespace CGAL {
namespace DDT {

template<typename Id, typename Tile>
class File_serializer
{
public:
    File_serializer(const std::map<Id, std::string>& files) : files(files) {}
    File_serializer() : files() {}

    void add(Id id, const std::string& filename) {
        files[id] = filename;
    }

    Tile load(Id id) const {
        const std::string filename = files.at(id);
        Tile tile(id);
        /// @todo return tile loaded from 'filename'
        return tile;
    }

    void save(const Tile& tile) const {
        const std::string filename = files.at(tile.id());
        /// @todo save tile in 'filename'
    }

private:
    std::map<Id,std::string> files;
};

}
}

#endif // CGAL_DDT_FILE_SERIALIZER_H
