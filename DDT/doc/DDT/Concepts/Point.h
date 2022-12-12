
/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `Point` describes the requirements of a point in D dimensions.

\cgalHasModel `CGAL::DDT::Cgal_traits_2::Point`
\cgalHasModel `CGAL::DDT::Cgal_traits_3::Point`
\cgalHasModel `CGAL::DDT::Cgal_traits_d::Point`
\cgalHasModel `CGAL::DDT::Cgal_traits::Point`

@todo coord type should not be double !
*/
#ifndef CGAL_DDT_CONCEPT_POINT
#define CGAL_DDT_CONCEPT_POINT

struct Point {
#ifndef DOXYGEN_RUNNING
    Point() {}
    Point(double x, double y) {}
    Point(double x, double y, double z) {}
#endif
    /// copy constructible
    Point(const Point&) {}
    /// \name Access to ith coordinate
    /// @{
    /// precondition : 0 <= i < ambient dimension

    double& operator[](int i) { return coord; }
    const double& operator[](int i) const { return coord; }
    /// @}

private:
    double coord;
};

#endif // CGAL_DDT_CONCEPT_POINT
