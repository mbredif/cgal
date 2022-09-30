
/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `TriangulationVertexBase_2` describes the requirements for the
vertex base class of a triangulation data structure
to be plugged in a basic, Delaunay or constrained
triangulations.

The concept `TriangulationVertexBase_2` refines the concept
`TriangulationDSVertexBase_2`
adding geometric information:
the vertex base of a triangulation stores a point.

\cgalRefines `TriangulationDSVertexBase_2`

\cgalHasModel `CGAL::Triangulation_vertex_base_2<Traits,Vb>`

\sa `TriangulationDataStructure_2`
\sa `TriangulationDataStructure_2::Vertex`
\sa `CGAL::Triangulation_vertex_base_2<Traits>`

*/

class Partitioner {
public:

/// \name Types
/// @{

    /*!
    Must be the same as the point type
    defined by the geometric traits class of the triangulation.
    */
    typedef unspecified_type Point;

    /*!
    The type of the tile identifier.
    */
    typedef unspecified_type Id;

/// @}

/// \name Access Functions
/// @{

/*!
returns tile Id of the given point.
*/

inline Id operator()(const Point& p) const;

/// @}


}; /* end Partitioner */

