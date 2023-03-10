
/*
\ingroup PkgDDTConcepts
\cgalConcept

The concept `Tile_index` describes the requirements for a Tile identifier in a distributed Delaunay triangulation.

*/
#ifndef CGAL_DDT_CONCEPT_ID
#define CGAL_DDT_CONCEPT_ID

#include <string>

struct TileIndex {

    // Default constructible
    TileIndex() {}

    // \name Grid_partitioner requirements
    // @{
    TileIndex(double) {}
    TileIndex operator%(std::size_t) const { return {}; }
    TileIndex operator+(TileIndex) const { return {}; }
    TileIndex operator*(std::size_t) const { return {}; }
    // @}

    // \name Comparisons
    // @{
    bool operator< (TileIndex) const { return true; }
    bool operator!=(TileIndex) const { return true; }
    bool operator==(TileIndex) const { return true; }
    // @}
};

namespace std {
std::string to_string(TileIndex) { return ""; }
}

std::istream& operator>>(std::istream& in, TileIndex id) { return in; }
#endif // CGAL_DDT_CONCEPT_ID
