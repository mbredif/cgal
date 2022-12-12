
/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `VertexIterator` describes the requirements of for the vertex iterator of the tile triangulations.

\cgalHasModel `CGAL::DDT::Cgal_traits_2::Vertex_iterator`
\cgalHasModel `CGAL::DDT::Cgal_traits_3::Vertex_iterator`
\cgalHasModel `CGAL::DDT::Cgal_traits_d::Vertex_iterator`
\cgalHasModel `CGAL::DDT::Cgal_traits::Vertex_iterator`

*/

#ifndef CGAL_DDT_CONCEPT_VERTEX_ITERATOR
#define CGAL_DDT_CONCEPT_VERTEX_ITERATOR

#include "Point.h"
struct VertexIterator {
    typedef typename ::Point Point;
    VertexIterator& operator++()  { return *this; }
    bool operator< (VertexIterator) const { return false; }
    bool operator!=(VertexIterator) const { return false; }
    bool operator==(VertexIterator) const { return false; }
    const Point& operator*() const { return p; }
private:
    Point p;
};

#endif // CGAL_DDT_CONCEPT_VERTEX_ITERATOR
