
#ifndef CGAL_DDT_CONCEPT_RANDOM_POINTS_IN_BALL
#define CGAL_DDT_CONCEPT_RANDOM_POINTS_IN_BALL

#include <Concepts/Point.h>

/*
\ingroup PkgDDTConcepts
\cgalConcept

The concept `RandomPointsInBall` describes the requirements of a output point iterator that generates points within a ball.

\cgalHasModel `CGAL::DDT::Triangulation_traits_2::Random_points_in_ball`
\cgalHasModel `CGAL::DDT::Triangulation_traits_3::Random_points_in_ball`
\cgalHasModel `CGAL::DDT::Triangulation_traits_d::Random_points_in_ball`
\cgalHasModel `CGAL::DDT::Triangulation_traits::Random_points_in_ball`

*/
struct RandomPointsInBall {
    typedef ::Point Point;
    RandomPointsInBall(int dimension, double range) {}
    RandomPointsInBall& operator++() { return *this; }
    Point_const_reference operator*() const { return p; }
private:
    Point p;
};

#endif // RANDOM_POINTS_IN_BALL
