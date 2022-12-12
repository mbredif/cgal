
/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `DelaunayTriangulation` describes the requirements for a Delaunay triangulation.

The only requirement are the stream operators. All operations on Delaunay triangulations are delegated
to the TriangulationTraits that operates on the opaque `DelaunayTriangulation` type.

*/

#ifndef CGAL_DDT_CONCEPT_DELAUNAY_TRIANGULATION
#define CGAL_DDT_CONCEPT_DELAUNAY_TRIANGULATION

struct DelaunayTriangulation {};

#include <iostream>

/// input stream operator
std::istream& operator>>(std::istream& in, DelaunayTriangulation& dt) { return in; }
/// output stream operator
std::ostream& operator<<(std::ostream& out, const DelaunayTriangulation& dt) { return out; }

#endif // CGAL_DDT_CONCEPT_DELAUNAY_TRIANGULATION
