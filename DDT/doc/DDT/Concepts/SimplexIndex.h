
/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `VertexIterator` describes the requirements of for the vertex iterator of the tile triangulations.

\cgalHasModel `CGAL::DDT::Cgal_traits_2::Vertex_index`
\cgalHasModel `CGAL::DDT::Cgal_traits_3::Vertex_index`
\cgalHasModel `CGAL::DDT::Cgal_traits_d::Vertex_index`
\cgalHasModel `CGAL::DDT::Cgal_traits::Vertex_index`

*/

#ifndef CGAL_DDT_CONCEPT_SIMPLEX_INDEX
#define CGAL_DDT_CONCEPT_SIMPLEX_INDEX

struct SimplexIndex {
    SimplexIndex& operator++() { return *this; }
    bool operator< (SimplexIndex) const { return false; }
    bool operator!=(SimplexIndex) const { return false; }
    bool operator==(SimplexIndex) const { return false; }
};

#endif // CGAL_DDT_CONCEPT_SIMPLEX_INDEX
