
/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `RandomPointsInBox` describes the requirements of a output point iterator that generates points within a box.

\cgalHasModel `CGAL::DDT::Cgal_traits_2::Random_points_in_box`
\cgalHasModel `CGAL::DDT::Cgal_traits_3::Random_points_in_box`
\cgalHasModel `CGAL::DDT::Cgal_traits_d::Random_points_in_box`
\cgalHasModel `CGAL::DDT::Cgal_traits::Random_points_in_box`

*/

#ifndef CGAL_DDT_CONCEPT_RANDOM_POINTS_IN_BOX
#define CGAL_DDT_CONCEPT_RANDOM_POINTS_IN_BOX

#include "Point.h"

struct RandomPointsInBox {
    typedef ::Point Point;
    RandomPointsInBox(int dimension, double range) {}
    RandomPointsInBox& operator++() { return *this; }
    const Point& operator*() const { return p; }
private:
    Point p;
};

#endif // CGAL_DDT_CONCEPT_RANDOM_POINTS_IN_BOX


