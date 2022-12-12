
/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `Bbox` describes the requirements for a bounding box with double coordinates.

\cgalHasModel `CGAL::DDT::Cgal_traits::Bbox`   (static dimensions)
\cgalHasModel `CGAL::DDT::Cgal_traits_2::Bbox` (static dimensions)
\cgalHasModel `CGAL::DDT::Cgal_traits_3::Bbox` (static dimensions)
\cgalHasModel `CGAL::DDT::Cgal_traits_d::Bbox` (dynamic dimensions)

*/

#ifndef CGAL_DDT_CONCEPT_BBOX
#define CGAL_DDT_CONCEPT_BBOX

#include "Point.h"

#include <iostream>

class Bbox {
public:
    typedef ::Point Point;

    /// Constructor for an empty bounding box in unspecified dimensions.
    Bbox() {}
    /// Constructor for an empty bounding box in d dimensions.
    /// if the geometric traits have a static dimension, the dimensions should match.
    Bbox(int d) {}
    /// Constructor for a bounding box in d dimensions with intervals [-range, range] on each axis.
    /// if the geometric traits have a static dimension, the dimensions should match
    Bbox(int d, double range) {}
    /// Grow the bounding box to also bound another given bounding box.
    Bbox& operator+=(const Bbox& bbox) { return *this; }
    /// Grow the bounding box to also bound a bounding box with double coordinates around the given `Traits::Point`.
    Bbox& operator+=(const Point& p) { return *this; }
    /// The ambient dimension.
    int dimension() const  { return dim; }

    /// \name Coordinates
    /// @{
    /// Access to the minimum and maximum coordinates along each axis.
    /// precondition : 0 <= i < dimension()

    double min(int i) const { return 0; }
    double max(int i) const { return 0; }
    double& min(int i) { return coord; }
    double& max(int i) { return coord; }
    /// @}

private:
    double coord;
    int dim;
}; /* end Bbox */

/// \name Streaming
/// @{
std::ostream& operator<<(std::ostream& out, const Bbox& bbox) { return out; }
std::istream& operator>>(std::istream& in, Bbox& bbox) { return in; }
/// @}


#endif // CGAL_DDT_CONCEPT_BBOX
