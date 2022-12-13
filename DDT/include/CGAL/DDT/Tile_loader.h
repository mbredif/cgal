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

#ifndef CGAL_DDT_TILE_LOADER_H
#define CGAL_DDT_TILE_LOADER_H


    /*
     *             typename TileContainer::Tile_iterator tile = tc.find(*it);
            if(tile == tc.end()) {
                while(tc.number_of_triangulations_mem_ >= tc.number_of_triangulations_mem_max_) {
                    auto it = tc.begin();
                    Id id0 = it->id();
                    std::size_t count0 = inbox[id0].size();
                    for(++it; it != tc.end() && count0; ++it)
                    {
                        Id id = it->id();
                        std::size_t count = inbox[id].size();
                        if(count0 > count) {
                            count0 = count;
                            id0 = id;
                        }
                    }
                    tc.unload(id0);
                }
*/

template<typename Serializer>
class Tile_loader {
    /// unload a tile from memory, automatically saving it.
    /// returns true after the loaded tile id is successfully saved and unloaded from memory.
    /// @todo attention à la perennité des handles (tile is possibly unloaded), ou alors lock ou shared pointer.
    template<typename TileContainer, typename Tile>
    void unload(TileContainer& tc, Tile& tile) {

        std::cout << "[" << std::setw(4) << std::to_string(tile.id()) << "] " << std::setfill('_');
        for(const Tile& t : tc) {
            if(t.locked           ) std::cout << "\x1b[1m" ; // blocked
            if(t.id() == tile.id()) std::cout << "\x1b[41m\x1b[1m" ; // bg red
            else if(!t.in_mem     ) std::cout << "\x1b[37m" ; // gray
            std::cout << std::to_string(t.id()) << "\x1B[0m" ; // reset
        }
        std::cout << std::setfill(' ') << " (" << tc.number_of_triangulations_mem_ << " in mem)" << std::endl;

        if (!tile.locked && tile.in_mem && serializer_.save(tile)) {
            tile.finalize();
            tile.triangulation().clear();
            tile.in_mem = false;
            --tc.number_of_triangulations_mem_;
        }
    }

    /// load a tile to memory, automatically saving it.
    template<typename TileContainer, typename Tile>
    void prepare_load(TileContainer& tc, Tile& tile) {
        if(tile.in_mem) return;

        std::cout << "[" << std::setw(4) << std::to_string(tile.id()) << "] " << std::setfill('_');
        for(const Tile& t : tc) {
            if(t.locked           ) std::cout << "\x1b[1m" ; // blocked
            if(t.id() == tile.id()) std::cout << "\x1b[42m" ; // bg green
            else if(!t.in_mem     ) std::cout << "\x1b[37m" ; // gray
            std::cout << std::to_string(t.id()) << "\x1B[0m" ; // reset
        }
        std::cout << std::setfill(' ') << " (" << tc.number_of_triangulations_mem_ << " in mem)" << std::endl;

        // make room if necessary
        while(tc.number_of_triangulations_mem_ >= tc.number_of_triangulations_mem_max_) {
            // pick a loaded id at random and try to unload it
            std::size_t n = rand() % tc.number_of_triangulations_mem_;
            for(Tile& t : tc) {
                if(t.in_mem) {
                    if (n == 0) {
                        if (!t.locked) unload(t);
                        break;
                    }
                    --n;
                 }
            }
        }
        // reserve the mem slot in memory, so that it is not stealed by a concurrent thread
        ++tc.number_of_triangulations_mem_;
    }

    template<typename TileContainer, typename Tile>
    bool safe_load(TileContainer& tc, Tile& tile) {
        if(tile.in_mem) return true;
        if (!serializer_.has_tile(tile.id()) || serializer_.load(tile)) {
            tile.in_mem = true;
            return true;
        } else {
            --tc.number_of_triangulations_mem_;
            return false;
        }
    }

    /// load a tile to memory.
    template<typename TileContainer, typename Tile>
    bool load(TileContainer& tc, Tile& tile) {
        prepare_load(tc, tile);
        return safe_load(tc, tile);
    }
private:
    Serializer serializer_;
};

#endif // CGAL_DDT_TILE_LOADER_H
