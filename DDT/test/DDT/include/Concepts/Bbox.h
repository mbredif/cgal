
/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `Bbox` describes the requirements for a bounding box with double coordinates.

\cgalHasModel `CGAL::DDT::Triangulation_traits::Bbox`   (static dimensions)
\cgalHasModel `CGAL::DDT::Triangulation_traits_2::Bbox` (static dimensions)
\cgalHasModel `CGAL::DDT::Triangulation_traits_3::Bbox` (static dimensions)
\cgalHasModel `CGAL::DDT::Triangulation_traits_d::Bbox` (dynamic dimensions)

*/

#ifndef CGAL_DDT_CONCEPT_BBOX
#define CGAL_DDT_CONCEPT_BBOX

#include <iostream>

class Bbox {
public:
    /// Constructor for an empty bounding box in unspecified dimensions.
    Bbox() {}
    /// Grow the bounding box to also bound another given bounding box.
    Bbox& operator+=(const Bbox& bbox) { return *this; }
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


    bool operator==(const Bbox& bbox) const { return true; }
    bool operator!=(const Bbox& bbox) const { return true; }

private:
    double coord;
    int dim;
}; /* end Bbox */

double measure(const Bbox& d) { return 0; }
double intersection_measure(const Bbox& x, const Bbox& y) { return 0; }

/// \name Streaming
/// @{
std::ostream& operator<<(std::ostream& out, const Bbox& bbox) { return out; }
std::istream& operator>>(std::istream& in, Bbox& bbox) { return in; }
/// @}


#endif // CGAL_DDT_CONCEPT_BBOX
