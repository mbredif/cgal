
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

    /// Must be the same as the point type defined by the geometric traits class of the triangulation.
    typedef unspecified_type Point;

    /// The type of the tile identifier.
    typedef unspecified_type Id;

/// @}

    /// Returns tile Id of the given point.
    Id operator()(const Point& p) const;

    /// The number of possible Ids.
    size_t size() const;

}; /* end Partitioner */


/// Streaming
std::ostream& operator<<(std::ostream& out, const Partitioner& partitioner);
