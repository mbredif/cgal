
#ifndef CGAL_DDT_CONCEPT_PARTITIONER
#define CGAL_DDT_CONCEPT_PARTITIONER

#include <Concepts/TileIndex.h>
#include <Concepts/Point.h>

/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `Partitioner` describes the requirements for the class that assigns a tile index to each point.
\cgalHasModelsBegin
\cgalHasModels{CGAL::DDT::Const_partitioner}
\cgalHasModels{CGAL::DDT::Grid_partitioner}
\cgalHasModelsEnd

*/

class Partitioner {
public:

/// \name Types
/// @{

#ifdef DOXYGEN_RUNNING
    /// model of `TileIndex`
    typedef unspecified_type Tile_index;
    /// Point type
    typedef unspecified_type Point;
    /// point const reference type
    typedef unspecified_type Point_const_reference;
#else
    typedef ::TileIndex Tile_index;
    typedef ::Point Point;
    typedef ::Point const& Point_const_reference;
#endif

/// @}

    /// returns the tile index of point `p`.
    Tile_index operator()(Point_const_reference p) const { return {}; }

    /// return the number of tile indices.
    /// \todo Is it the number of tiles or the largest tile index (maybe tile indices are not contiguous)
    std::size_t size() const { return 0; }

}; /* end Partitioner */

#endif // CGAL_DDT_CONCEPT_PARTITIONER
