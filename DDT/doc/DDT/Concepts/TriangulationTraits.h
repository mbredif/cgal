#ifndef CGAL_DDT_CONCEPT_TRIANGULATION_TRAITS
#define CGAL_DDT_CONCEPT_TRIANGULATION_TRAITS

#include "Point.h"
#include "SimplexIndex.h"
struct VertexIndex : public SimplexIndex {};
struct FacetIndex: public SimplexIndex {};
struct CellIndex: public SimplexIndex {};

/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `TriangulationTraits` describes the requirements of a traits that adapts a Delaunay Triangulation implementation.

\cgalHasModel `CGAL::DDT::Triangulation_traits_2`
\cgalHasModel `CGAL::DDT::Triangulation_traits_3`
\cgalHasModel `CGAL::DDT::Triangulation_traits_d`
\cgalHasModel `CGAL::DDT::Triangulation_traits`

*/

#include "Bbox.h"
#include "TileIndex.h"
#include "RandomPointsInBox.h"
#include "RandomPointsInBall.h"
#include <vector>

class TriangulationTraits {
private:
    struct unspecified_type {};
public:

/// \name Types
/// @{
///
#ifdef DOXYGEN_RUNNING
    /// point type
    typedef unspecified_type Point;
    /// Tile identifier type, model of `TileIndex`
    typedef unspecified_type Tile_index;
    /// user Info type
    typedef unspecified_type Info;

    /// Handle to a vertex, model of `std::incrementable` and `LessThanComparable`.
    typedef unspecified_type Vertex_index;
    /// Handle to a facet, model of `std::incrementable` and `LessThanComparable`.
    typedef unspecified_type  Facet_index;
    /// Handle to a cell, model of `std::incrementable` and `LessThanComparable`.
    typedef unspecified_type   Cell_index;

    /// The adapted Triangulation
    typedef unspecified_type Triangulation;
    /// A model of the `Bbox` concept
    typedef unspecified_type Bbox;
    /// the ambient dimension if it is static, 0 if it is dynamic
    static constexpr int D;
#else
    typedef ::Point Point;
    typedef ::TileIndex Tile_index;
    typedef unspecified_type Info;
    typedef VertexIndex Vertex_index;
    typedef FacetIndex Facet_index;
    typedef CellIndex Cell_index;
    typedef unspecified_type Triangulation;
    typedef ::RandomPointsInBall Random_points_in_ball;
    typedef ::RandomPointsInBox Random_points_in_box;
    typedef ::Bbox Bbox;
    static constexpr int D=33;
#endif


    /// @}

    /// default constructor
    TriangulationTraits(int dimension) {}
    /// constructs an empty triangulation of the given dimension
    Triangulation triangulation() const { return {}; }
    /// returns the dimension of the traits
    int dimension() const { return 0; }
    /// returns the current dimension of a Delaunay triangulation
    int current_dimension(const Triangulation& tri) const { return 0; }
    /// returns the maximal dimension of a Delaunay triangulation
    int maximal_dimension(const Triangulation& tri) const { return 0; }
    /// returns the number of cells, including infinite cells
    std::size_t number_of_cells(const Triangulation& tri) const { return 0; }
    /// returns the number of vertices, including the infinite vertex
    std::size_t number_of_vertices(const Triangulation& tri) const { return 0; }
    /// checks the validity of a Delaunay triangulation
    bool is_valid(const Triangulation& tri, bool verbose = false, int level = 0) const { return true; }
    /// locates a vertex in a Delaunay triangulation, given a query point `p`, and a `hint`, otherwise returns the infinite vertex
    Vertex_index locate_vertex(const Triangulation& tri, const Point& p, Vertex_index hint = Vertex_index()) const { return v; }
    /// collects in the output iterator `out` all cells incident to the query vertex `v` in the triangulation `tri`
    template<typename OutputIterator>
    OutputIterator incident_cells(const Triangulation& tri, Vertex_index v, OutputIterator out) const { return out; }
    /// collects in the output iterator `out` all vertices adjacent to the query vertex `v` in the triangulation `tri`
    template<typename OutputIterator>
    OutputIterator adjacent_vertices(const Triangulation& tri, Vertex_index v, OutputIterator out) const { return out; }
    /// computes a permutation in `indices` of `points`, using the geometric traits of `tri`
    void spatial_sort(const Triangulation& tri, std::vector<std::size_t>& indices, const std::vector<Point>& points) const {}

/// \name Iterators
/// @{
    /// returns begin const iterator on the set of vertices
    Vertex_index vertices_begin(const Triangulation& tri) const { return v; }
    /// returns begin const iterator on the set of vertices
    Vertex_index vertices_end(const Triangulation& tri) const { return v; }
    /// returns begin iterator on the set of facets
    Facet_index facets_begin(const Triangulation& tri) const { return f; }
    /// returns end iterator on the set of facets
    Facet_index facets_end(const Triangulation& tri) const { return f; }
    /// returns begin iterator on the set of cells
    Cell_index cells_begin(const Triangulation& tri) const { return c; }
    /// returns end iterator on the set of cells
    Cell_index cells_end(const Triangulation& tri) const { return c; }
    /// returns the infinite vertex of a triangulation
    Vertex_index infinite_vertex(const Triangulation& tri) const { return v; }
/// @}


/// \name Triangulation Modification
/// @{
    /// clears the triangulation
    void clear(Triangulation& tri) const {}
    /// inserts a vertex located at `point` in tile `id` into the triangulation, using the `hint` if available.
    /// @return a pair of the vertex index located at `point` and a boolean that reports if a vertex has been created.
    std::pair<Vertex_index, bool> insert(Triangulation& tri, const Point& p, Tile_index id, Vertex_index hint = Vertex_index()) const { return {v,false}; }
    /// removes a vertex from the triangulation
    void remove(Triangulation& tri, Vertex_index v) const {}
/// @}


/// \name Accessors
/// @{
    /// returns the point embedding of a vertex
    const Point& point(const Triangulation& tri, Vertex_index v) const { return p; }
    /// creates a box bounding the approximation of a point
    Bbox bbox(const Point& p) const { return {}; }
    /// creates an an empty bounding box in d dimensions.
    /// If the geometric traits have a static dimension, the dimensions should match.
    static Bbox bbox(int d) { return {}; }
    /// creates a bounding box in d dimensions with intervals [-range, range] on each axis.
    /// If the geometric traits have a static dimension, the dimensions should match
    static  Bbox bbox(int d, double range) { return {}; }

    /// returns the identifier of the tile where this vertex is local
    Tile_index  vertex_id  (const Triangulation& tri, Vertex_index v) const { return {}; }
    /// returns the flag of the vertex
    const Info& info(Vertex_index v) const { return i; }
    /// returns the ith coodinate of a point as a (possibly approximated) double
    static double approximate_cartesian_coordinate(const Point& p, int i) { return {}; }
    /// compares the `i`'th Cartesian coodinate of `p` and `q`
    bool less_coordinate(const Point& p, const Point& q, int i) const { return true; }
/// @}


/// \name Tests
/// @{
    /// tests if a vertex is the infinite vertex
    bool vertex_is_infinite(const Triangulation& tri, Vertex_index v) const { return {}; }
    /// tests if a facet is infinite (incident to the infinite vertex)
    bool facet_is_infinite(const Triangulation& tri, Facet_index f) const { return {}; }
    /// tests if a cell is infinite (incident to the infinite vertex)
    bool cell_is_infinite(const Triangulation& tri, Cell_index c) const { return {}; }
    /// tests if two vertices from possibly different triangulations have the same point embedding
    bool are_vertices_equal(const Triangulation& t1, Vertex_index v1, const Triangulation& t2, Vertex_index v2) const { return {}; }
    /// tests if two facets from possibly different triangulations have the same point embeddings
    bool are_facets_equal(const Triangulation& t1, Facet_index f1, const Triangulation& t2, Facet_index f2) const { return {}; }
    /// tests if two cells from possibly different triangulations have the same point embeddings
    bool are_cells_equal(const Triangulation& t1, Cell_index c1, const Triangulation& t2, Cell_index c2) const { return {}; }
/// @}


/// \name Triangulation Data Structure traversal
/// @{
    /// returns the ith vertex of a cell
    Vertex_index vertex(const Triangulation& tri, Cell_index c, int i) const { return {}; }
    /// returns the facet incident to a cell, which covertex is the ith vertex of the cell
    Facet_index facet(const Triangulation& tri, Cell_index c, int i) const { return {}; }
    /// returns the index of the covertex of a facet
    int index_of_covertex(const Triangulation& tri, Facet_index f) const { return {}; }
    /// returns the covertex of a facet, which is the only vertex of cell containing the facet that is not incident to the facet
    Vertex_index covertex(const Triangulation& tri, Facet_index f) const { return {}; }
    /// returns the covertex of the mirror of a facet
    Vertex_index mirror_vertex(const Triangulation& tri, Facet_index f) const { return {}; }
    /// returns the cell incident to a facet
    Cell_index cell(const Triangulation& tri, Facet_index f) const { return {}; }
    /// returns one of the cells incident to a vertex
    Cell_index cell(const Triangulation& tri, Vertex_index f) const { return {}; }
    /// returns the mirror facet of a facet
    Facet_index mirror_facet(const Triangulation& tri, Facet_index f) const { return {}; }
    /// returns the mirror index of a facet, which is the index of the covertex of its mirror facet
    int mirror_index(const Triangulation& tri, Facet_index f) const { return {}; }
    /// returns the index of a cell in its ith neighbor
    int mirror_index(const Triangulation& tri, Cell_index c, int i) const { return {}; }
    /// returns the neighboring cell of a cell, opposite to its itg vertex
    Cell_index neighbor(const Triangulation& tri, Cell_index c, int i) const { return {}; }
/// @}

/// \name Streaming
/// @{
    std::ostream& write(std::ostream& out, const Triangulation& tri) const { return out; }
    std::istream& read(std::istream& in, Triangulation& tri) const { return in; }
/// @}


private:
    Point p;
    Info i;
    Vertex_index v;
    Cell_index c;
    Facet_index f;
}; /* end TriangulationTraits */

#endif // CGAL_DDT_CONCEPT_TRIANGULATION_TRAITS
