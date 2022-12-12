
/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `Id` describes the requirements for a Tile identifier in a distributed Delaunay triangulation.

*/
#ifndef CGAL_DDT_CONCEPT_ID
#define CGAL_DDT_CONCEPT_ID

#include <string>

struct Id {

    /// Default constructible
    Id() {}

    /// \name Grid_partitioner requirements
    /// @{
    Id(double) {}
    Id operator%(std::size_t) const { return {}; }
    Id operator+(Id) const { return {}; }
    Id operator*(std::size_t) const { return {}; }
    /// @}

    /// \name Comparisons
    /// @{
    bool operator< (Id) const { return true; }
    bool operator!=(Id) const { return true; }
    bool operator==(Id) const { return true; }
    /// @}
};

namespace std {
std::string to_string(Id) { return ""; }
}

std::istream& operator>>(std::istream& in, Id& id) { return in; }
#endif // CGAL_DDT_CONCEPT_ID
