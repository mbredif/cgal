
#ifndef CGAL_DDT_CONCEPT_POINTSET
#define CGAL_DDT_CONCEPT_POINTSET

#include <Concepts/TileIndex.h>
#include <Concepts/Point.h>

/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `PointSet` describes the requirements for classes that model sequences of points.
\cgalHasModelsBegin
\cgalHasModels{CGAL::DDT::Random_point_set}
\cgalHasModels{CGAL::DDT::LAS_point_set}
\cgalHasModels{Container}
\cgalHasModels{PairContainer}
\cgalHasModelsEnd

*/
/// \todo MB: should this concept have model or adapt CGAL::Point_set_2, CGAL::Point_set_3... ?
/// \todo MB: empty concept, to be populated
class PointSet {};


#endif // CGAL_DDT_CONCEPT_POINTSET
