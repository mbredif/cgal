#ifndef CGAL_DDT_CONCEPT_TRIANGULATION_TRAITS
#define CGAL_DDT_CONCEPT_TRIANGULATION_TRAITS

#include <Concepts/Point.h>
#include <Concepts/SimplexIndex.h>
#include <Concepts/Bbox.h>
#include <Concepts/TileIndex.h>
#include <Concepts/RandomPointsInBox.h>
#include <Concepts/RandomPointsInBall.h>
#include <CGAL/DDT/triangulation/Triangulation_traits.h>
#include <vector>

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


namespace CGAL {
namespace DDT {

namespace Impl {
    ::Point p;
    ::VertexIndex v;
    ::CellIndex c;
    ::FacetIndex f;
}

template<> struct Triangulation_traits<::Triangulation> {

/// \name Types
/// @{
///
#ifdef DOXYGEN_RUNNING
private:
    struct unspecified_type {};
public:
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
    typedef VertexIndex Vertex_index;
    typedef FacetIndex Facet_index;
    typedef CellIndex Cell_index;
    typedef ::Triangulation Triangulation;
    typedef ::RandomPointsInBall Random_points_in_ball;
    typedef ::RandomPointsInBox Random_points_in_box;
    typedef ::Bbox Bbox;
    static constexpr int D=33;
#endif


    /// @}

    /// constructs an empty triangulation of the given dimension
    static Triangulation triangulation(int dim) { return {}; }
    /// returns the current dimension of a Delaunay triangulation
    static int current_dimension(const Triangulation& tri) { return 0; }
    /// returns the maximal dimension of a Delaunay triangulation
    static int maximal_dimension(const Triangulation& tri) { return 0; }
    /// returns the number of cells, including infinite cells
    static std::size_t number_of_cells(const Triangulation& tri) { return 0; }
    /// returns the number of vertices, including the infinite vertex
    static std::size_t number_of_vertices(const Triangulation& tri) { return 0; }
    /// checks the validity of a Delaunay triangulation
    static bool is_valid(const Triangulation& tri, bool verbose = false, int level = 0) { return true; }
    /// locates a vertex in a Delaunay triangulation, given a query point `p`, and a `hint`, otherwise returns the infinite vertex
    static Vertex_index locate_vertex(const Triangulation& tri, const Point& p, Vertex_index hint = Vertex_index()) { return hint; }
    /// collects in the output iterator `out` all cells incident to the query vertex `v` in the triangulation `tri`
    template<typename OutputIterator>
    static OutputIterator incident_cells(const Triangulation& tri, Vertex_index v, OutputIterator out) { return out; }
    /// collects in the output iterator `out` all vertices adjacent to the query vertex `v` in the triangulation `tri`
    template<typename OutputIterator>
    static OutputIterator adjacent_vertices(const Triangulation& tri, Vertex_index v, OutputIterator out) { return out; }
    /// computes a permutation in `indices` of `points`, using the geometric traits of `tri`
    static void spatial_sort(const Triangulation& tri, std::vector<std::size_t>& indices, const std::vector<Point>& points) {}

/// \name Iterators
/// @{
    /// returns begin const iterator on the set of vertices
    static Vertex_index vertices_begin(const Triangulation& tri) { return Impl::v; }
    /// returns begin const iterator on the set of vertices
    static Vertex_index vertices_end(const Triangulation& tri) { return Impl::v; }
    /// returns begin iterator on the set of facets
    static Facet_index facets_begin(const Triangulation& tri) { return Impl::f; }
    /// returns end iterator on the set of facets
    static Facet_index facets_end(const Triangulation& tri) { return Impl::f; }
    /// returns begin iterator on the set of cells
    static Cell_index cells_begin(const Triangulation& tri) { return Impl::c; }
    /// returns end iterator on the set of cells
    static Cell_index cells_end(const Triangulation& tri) { return Impl::c; }
    /// returns the infinite vertex of a triangulation
    static Vertex_index infinite_vertex(const Triangulation& tri) { return Impl::v; }
/// @}


/// \name Triangulation Modification
/// @{
    /// clears the triangulation
    static void clear(Triangulation& tri) {}
    /// inserts a vertex located at `point` in tile `id` into the triangulation, using the `hint` if available.
    /// @return a pair of the vertex index located at `point` and a boolean that reports if a vertex has been created.
    static std::pair<Vertex_index, bool> insert(Triangulation& tri, const Point& p, Vertex_index hint = Vertex_index()) { return {hint,false}; }
    /// removes a vertex from the triangulation
    static void remove(Triangulation& tri, Vertex_index v) {}
/// @}


/// \name Accessors
/// @{
    /// returns the point embedding of a vertex
    static const Point& point(const Triangulation& tri, Vertex_index v) { return Impl::p; }
    /// creates a box bounding the approximation of a point
    static Bbox bbox(const Point& p) { return {}; }
    /// creates an an empty bounding box in d dimensions.
    /// If the geometric traits have a static dimension, the dimensions should match.
    static Bbox bbox(int d) { return {}; }
    /// creates a bounding box in d dimensions with intervals [-range, range] on each axis.
    /// If the geometric traits have a static dimension, the dimensions should match
    static  Bbox bbox(int d, double range) { return {}; }

    /// returns the identifier of the tile where this vertex is local
    Tile_index  vertex_id  (const Triangulation& tri, Vertex_index v) { return {}; }
    /// returns the ith coodinate of a point as a (possibly approximated) double
    static double approximate_cartesian_coordinate(const Point& p, int i) { return {}; }
    /// compares the `i`'th Cartesian coodinate of `p` and `q`
    static bool less_coordinate(const Point& p, const Point& q, int i) { return true; }
/// @}


/// \name Tests
/// @{
    /// tests if a vertex is the infinite vertex
    static bool vertex_is_infinite(const Triangulation& tri, Vertex_index v) { return {}; }
    /// tests if a facet is infinite (incident to the infinite vertex)
    static bool facet_is_infinite(const Triangulation& tri, Facet_index f) { return {}; }
    /// tests if a cell is infinite (incident to the infinite vertex)
    static bool cell_is_infinite(const Triangulation& tri, Cell_index c) { return {}; }
    /// tests if two vertices from possibly different triangulations have the same point embedding
    static bool are_vertices_equal(const Triangulation& t1, Vertex_index v1, const Triangulation& t2, Vertex_index v2) { return {}; }
    /// tests if two facets from possibly different triangulations have the same point embeddings
    static bool are_facets_equal(const Triangulation& t1, Facet_index f1, const Triangulation& t2, Facet_index f2) { return {}; }
    /// tests if two cells from possibly different triangulations have the same point embeddings
    static bool are_cells_equal(const Triangulation& t1, Cell_index c1, const Triangulation& t2, Cell_index c2) { return {}; }
/// @}


/// \name Triangulation Data Structure traversal
/// @{
    /// returns the ith vertex of a cell
    static Vertex_index vertex(const Triangulation& tri, Cell_index c, int i) { return {}; }
    /// returns the facet incident to a cell, which covertex is the ith vertex of the cell
    static Facet_index facet(const Triangulation& tri, Cell_index c, int i) { return {}; }
    /// returns the index of the covertex of a facet
    static int index_of_covertex(const Triangulation& tri, Facet_index f) { return {}; }
    /// returns the covertex of a facet, which is the only vertex of cell containing the facet that is not incident to the facet
    static Vertex_index covertex(const Triangulation& tri, Facet_index f) { return {}; }
    /// returns the covertex of the mirror of a facet
    static Vertex_index mirror_vertex(const Triangulation& tri, Facet_index f) { return {}; }
    /// returns the cell incident to a facet
    static Cell_index cell(const Triangulation& tri, Facet_index f) { return {}; }
    /// returns one of the cells incident to a vertex
    static Cell_index cell(const Triangulation& tri, Vertex_index f) { return {}; }
    /// returns the mirror facet of a facet
    static Facet_index mirror_facet(const Triangulation& tri, Facet_index f) { return {}; }
    /// returns the mirror index of a facet, which is the index of the covertex of its mirror facet
    static int mirror_index(const Triangulation& tri, Facet_index f) { return {}; }
    /// returns the index of a cell in its ith neighbor
    static int mirror_index(const Triangulation& tri, Cell_index c, int i) { return {}; }
    /// returns the neighboring cell of a cell, opposite to its itg vertex
    static Cell_index neighbor(const Triangulation& tri, Cell_index c, int i) { return {}; }
/// @}

/// \name Streaming
/// @{
    static std::ostream& write(std::ostream& out, const Triangulation& tri) { return out; }
    static std::istream& read(std::istream& in, Triangulation& tri) { return in; }
/// @}
};


}
}

#endif // CGAL_DDT_CONCEPT_TRIANGULATION_TRAITS
