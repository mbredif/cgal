
#ifndef CGAL_DDT_CONCEPT_PARTITIONER
#define CGAL_DDT_CONCEPT_PARTITIONER

#include "TileIndex.h"
#include "Point.h"
/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `Partitioner` describes the requirements for the class that assigns a tile index to each point.
\cgalHasModel `CGAL::DDT::Const_partitioner`
\cgalHasModel `CGAL::DDT::Grid_partitioner`

*/

class Partitioner {
public:

/// \name Types
/// @{

#ifdef DOXYGEN_RUNNING
    /// model of `TileIndex`
    typedef unspecified_type Tile_index;
    /// model of `Point`
    typedef unspecified_type Point;
#else
    typedef ::TileIndex Tile_index;
    typedef ::Point Point;
#endif

/// @}

    /// Returns Tile_index of the given point.
    Tile_index operator()(const Point& p) const { return {}; }

    /// The number of tile indices.
    std::size_t size() const { return 0; }

}; /* end Partitioner */

#endif // CGAL_DDT_CONCEPT_PARTITIONER
