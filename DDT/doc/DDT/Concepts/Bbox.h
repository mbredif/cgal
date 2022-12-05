
/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `Bbox` describes the requirements for a bounding box with double coordinates.

\cgalHasModel `CGAL::DDT::Cgal_traits::Bbox`   (static dimensions)
\cgalHasModel `CGAL::DDT::Cgal_traits_2::Bbox` (static dimensions)
\cgalHasModel `CGAL::DDT::Cgal_traits_3::Bbox` (static dimensions)
\cgalHasModel `CGAL::DDT::Cgal_traits_d::Bbox` (dynamic dimensions)

*/

class Bbox {
public:
    /// Constructor for an empty bounding box in unspecified dimensions.
    Bbox();
    /// Constructor for an empty bounding box in d dimensions.
    /// if the geometric traits have a static dimension, the dimensions should match.
    Bbox(unsigned int d);
    /// Constructor for a bounding box in d dimensions with intervals [-range, range] on each axis.
    /// if the geometric traits have a static dimension, the dimensions should match
    Bbox(unsigned int d, double range);
    /// Grow the bounding box to also bound another given bounding box.
    Bbox& operator+=(const Bbox& bbox);
    /// Grow the bounding box to also bound a bounding box with double coordinates around the given `Traits::Point`.
    Bbox& operator+=(const Point& p);
    /// The ambient dimension.
    int dimension() const;

    /// \name Coordinates
    /// @{
    /// Access to the minimum and maximum coordinates along each axis.
    /// precondition : 0 <= i < dimension()

    double min BOOST_PREVENT_MACRO_SUBSTITUTION (int i) const;
    double max BOOST_PREVENT_MACRO_SUBSTITUTION (int i) const;
    double& min BOOST_PREVENT_MACRO_SUBSTITUTION (int i);
    double& max BOOST_PREVENT_MACRO_SUBSTITUTION (int i);
    /// @}

}; /* end Bbox */

/// \name Streaming
/// @{
std::ostream& operator<<(std::ostream& out, const Bbox& bbox);
std::istream& operator>>(std::istream& in, Bbox& bbox);
/// @}
