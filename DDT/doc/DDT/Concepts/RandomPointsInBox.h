
#ifndef CGAL_DDT_CONCEPT_RANDOM_POINTS_IN_BOX
#define CGAL_DDT_CONCEPT_RANDOM_POINTS_IN_BOX

#include "Point.h"

/*
\ingroup PkgDDTConcepts
\cgalConcept

The concept `RandomPointsInBox` describes the requirements of a output point iterator that generates points within a box.

\cgalHasModel `CGAL::DDT::Triangulation_traits_2::Random_points_in_box`
\cgalHasModel `CGAL::DDT::Triangulation_traits_3::Random_points_in_box`
\cgalHasModel `CGAL::DDT::Triangulation_traits_d::Random_points_in_box`
\cgalHasModel `CGAL::DDT::Triangulation_traits::Random_points_in_box`

*/
struct RandomPointsInBox {
    typedef ::Point Point;
    RandomPointsInBox(int dimension, double range) {}
    RandomPointsInBox& operator++() { return *this; }
    const Point& operator*() const { return p; }
private:
    Point p;
};

#endif // CGAL_DDT_CONCEPT_RANDOM_POINTS_IN_BOX


