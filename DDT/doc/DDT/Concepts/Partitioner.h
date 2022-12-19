
/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `Partitioner` describes the requirements for the class that assigns tile identifiers to points.
\cgalHasModel `CGAL::DDT::Const_partitioner`
\cgalHasModel `CGAL::DDT::Grid_partitioner`
\cgalHasModel `CGAL::DDT::Random_partitioner`

*/

class Partitioner {
public:

/// \name Types
/// @{

    /// Point type
    typedef ::Point Point;

    /// Identifier type of a tile the partition
    /// \cgalModels Tile_index
    typedef ::Tile_index Tile_index;

/// @}

    /// Returns Tile_index of the given point.
    Tile_index operator()(const Point& p) const;

    /// The number of tile indices.
    std::size_t size() const;

}; /* end Partitioner */
