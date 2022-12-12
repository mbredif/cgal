
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
    /// \cgalModels Id
    typedef ::Id Id;

/// @}

    /// Returns tile Id of the given point.
    Id operator()(const Point& p) const;

    /// The number of possible Ids.
    std::size_t size() const;

}; /* end Partitioner */
