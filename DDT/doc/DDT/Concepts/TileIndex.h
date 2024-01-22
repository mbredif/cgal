#ifndef CGAL_DDT_CONCEPT_TILE_INDEX
#define CGAL_DDT_CONCEPT_TILE_INDEX

#include <string>

/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `TileIndex` describes the requirements for a Tile identifier in a distributed triangulation or a distributed point set.

/// \todo no function is documented here MB: ok. should we put trivial docs for each operator ?
*/
struct TileIndex {

    /// Default constructible
    TileIndex() {}

    /// \name Partitioner requirements
    /// @{
    TileIndex(std::size_t) {}
    TileIndex operator%(std::size_t) const { return {}; }
    TileIndex operator+(TileIndex) const { return {}; }
    TileIndex operator-(TileIndex) const { return {}; }
    TileIndex operator++() const { return {}; }
    TileIndex operator*(std::size_t) const { return {}; }
    explicit operator size_t() { return 0; }
    /// @}

    /// \name Comparisons
    /// @{
    bool operator< (TileIndex) const { return true; }
    bool operator!=(TileIndex) const { return true; }
    bool operator==(TileIndex) const { return true; }
    /// @}
};

/// \todo is this part of the concept? MB: I think it is required for serialization (disk streaming and MPI scheduling)
namespace std {
std::string to_string(TileIndex) { return ""; }
}

/// \todo is this part of the concept? MB: I think it is required for deserialization (disk streaming and MPI scheduling)
std::istream& operator>>(std::istream& in, TileIndex id) { return in; }


/// make it hashable so that it can be used in unordered associative containers
/// \todo Note that there exist `Hashable` and `DefaultConstructible` concepts. MB: ok, we should use it then
template <>
struct std::hash<TileIndex>
{
    std::size_t operator()( const TileIndex& k ) const { return 0; }
};

#endif // CGAL_DDT_CONCEPT_TILE_INDEX
