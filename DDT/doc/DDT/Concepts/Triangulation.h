
#ifndef CGAL_DDT_CONCEPT_TRIANGULATION
#define CGAL_DDT_CONCEPT_TRIANGULATION

/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `Triangulation` describes the requirements for the class that maintains a Delaunay triangulation.
\cgalHasModelsBegin
\cgalHasModels{CGAL::Delaunay_Triangulation}
\cgalHasModels{CGAL::Delaunay_Triangulation_2}
\cgalHasModels{CGAL::Delaunay_Triangulation_3}
\cgalHasModelsEnd

*/

/// \todo MB: this file should ensure that the TriangulationTraits specialization and the related free functions are provided (see models)
/// \todo MB: rewrites static functions into free functions in each model.
class Triangulation {
}; /* end Triangulation */



#endif // CGAL_DDT_CONCEPT_TRIANGULATION
