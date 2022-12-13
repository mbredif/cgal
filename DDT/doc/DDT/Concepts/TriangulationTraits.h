
/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `TriangulationTraits` describes the requirements of a traits that adapts a Delaunay Triangulation implementation.

\cgalHasModel `CGAL::DDT::Cgal_traits_2`
\cgalHasModel `CGAL::DDT::Cgal_traits_3`
\cgalHasModel `CGAL::DDT::Cgal_traits_d`
\cgalHasModel `CGAL::DDT::Cgal_traits`

*/

#ifndef CGAL_DDT_CONCEPT_TRIANGULATION_TRAITS
#define CGAL_DDT_CONCEPT_TRIANGULATION_TRAITS

#include "Bbox.h"
#include "Point.h"
#include "Id.h"
#include "VertexIterator.h"
#include "CellIterator.h"
#include "FacetIterator.h"
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
    /// Point embedding of a vertex
    typedef ::Point Point;
    /// Tile identifier type
    typedef ::Id Id;
    /// user Info type
    typedef unspecified_type Info;

    /// Const iterator to a vertex
    typedef ::VertexIterator Vertex_const_iterator;
    /// Const handle to a vertex
    typedef ::VertexIterator Vertex_const_handle;
    /// Iterator to a vertex, model of `InputIterator` and `LessThanComparable`
    typedef ::VertexIterator Vertex_iterator;
    /// Handle to a vertex
    typedef ::VertexIterator Vertex_handle;

    /// Const iterator to a cell
    typedef ::CellIterator Cell_const_iterator;
    /// Const handle to a cell
    typedef ::CellIterator Cell_const_handle;
    /// Iterator to a cell
    typedef ::CellIterator Cell_iterator;
    /// Handle to a cell
    typedef ::CellIterator Cell_handle;

    /// Const iterator to a facet
    typedef ::FacetIterator Facet_const_iterator;
    /// Const handle to a facet
    typedef ::FacetIterator Facet_const_handle;
    /// Iterator to a facet
    typedef ::FacetIterator Facet_iterator;
    /// Handle to a facet
    typedef ::FacetIterator Facet_handle;

    /// The adapted Delaunay Triangulation
    typedef unspecified_type Delaunay_triangulation;
    /// Random point generator, in a ball
    typedef ::RandomPointsInBall Random_points_in_ball;
    /// Random point generator, in a box
    typedef ::RandomPointsInBox Random_points_in_box;
    /// A model of the Bbox concept
    typedef ::Bbox Bbox;
    /// the ambient dimension if it is static, 0 if it is dynamic
    enum { D };
    /// @}

    /// default constructor
    TriangulationTraits(int dimension) {}
    /// construct an empty triangulation of the given dimension
    Delaunay_triangulation triangulation() const { return {}; }
    /// get the dimension of the traits
    int dimension() const { return 0; }
    /// get the current dimension of a Delaunay triangulation
    int current_dimension(const Delaunay_triangulation& dt) const { return 0; }
    /// get the maximal dimension of a Delaunay triangulation
    int maximal_dimension(const Delaunay_triangulation& dt) const { return 0; }
    /// get the number of cells, including infinite cells
    std::size_t number_of_cells(const Delaunay_triangulation& dt) const { return 0; }
    /// get the number of vertices, including the infinite vertex
    std::size_t number_of_vertices(const Delaunay_triangulation& dt) const { return 0; }
    /// checks the validity of a Delaunay triangulation
    bool is_valid(const Delaunay_triangulation& dt, bool verbose = false, int level = 0) const { return true; }
    /// locates a vertex in a Delaunay triangulation, given a query point `p`, and a `hint`.
    /// returns `vertices_end(dt)` if no vertices of `dt` are located at `p`
    Vertex_const_handle locate_vertex(const Delaunay_triangulation& dt, const Point& p, Vertex_handle hint = Vertex_handle()) const { return {}; }
    /// collect in the output iterator `out` all cells incident to the query vertex `v` in the triangulation `dt`
    template<typename OutputIterator>
    OutputIterator incident_cells(const Delaunay_triangulation& dt, Vertex_const_handle v, OutputIterator out) const { return out; }
    /// collect in the output iterator `out` all vertices adjacent to the query vertex `v` in the triangulation `dt`
    template<typename OutputIterator>
    OutputIterator adjacent_vertices(const Delaunay_triangulation& dt, Vertex_const_handle v, OutputIterator out) const { return out; }
    /// computes a permutation in `indices` of `points`, using the geometric traits of `dt`
    void spatial_sort(const Delaunay_triangulation& dt, std::vector<std::size_t>& indices, const std::vector<Point>& points) const {}

/// \name Iterators
/// @{
    /// begin const iterator on the set of vertices
    Vertex_const_iterator vertices_begin(const Delaunay_triangulation& dt) const { return {}; }
    /// begin const iterator on the set of vertices
    Vertex_const_iterator vertices_end(const Delaunay_triangulation& dt) const { return {}; }
    /// begin iterator on the set of vertices
    Vertex_iterator vertices_begin(Delaunay_triangulation& dt) const { return {}; }
    /// end iterator on the set of vertices
    Vertex_iterator vertices_end(Delaunay_triangulation& dt) const { return {}; }
    /// begin iterator on the set of facets
    Facet_const_iterator facets_begin(const Delaunay_triangulation& dt) const { return {}; }
    /// end iterator on the set of facets
    Facet_const_iterator facets_end(const Delaunay_triangulation& dt) const { return {}; }
    /// begin iterator on the set of cells
    Cell_const_iterator cells_begin(const Delaunay_triangulation& dt) const { return {}; }
    /// end iterator on the set of cells
    Cell_const_iterator cells_end(const Delaunay_triangulation& dt) const { return {}; }
    /// get the infinite vertex of a triangulation
    Vertex_handle infinite_vertex(const Delaunay_triangulation& dt) const { return {}; }
/// @}


/// \name Triangulation Modification
/// @{
    /// clear the triangulation
    void clear(Delaunay_triangulation& dt) const {}
    /// inserts a vertex located at `point` in tile `id` into the triangulation, using the `hint` if available.
    /// @return a pair of the vertex handle located at `point` and a boolean that reports if a vertex has been created.
    std::pair<Vertex_handle, bool> insert(Delaunay_triangulation& dt, const Point& p, Id id, Vertex_handle hint = Vertex_handle()) const { return {{},false}; }
    /// remove a vertex from the triangulation
    void remove(Delaunay_triangulation& dt, Vertex_handle v) const {}
/// @}


/// \name Accessors
/// @{
    /// access the point embedding of a vertex
    const Point& point(const Delaunay_triangulation& dt, Vertex_const_handle v) const { return p; }
    /// access the identifier of the tile where this vertex is local
    Id    id  (Vertex_const_handle v) const { return {}; }
    /// access the flag of the vertex
    const Info& info(Vertex_const_handle v) const { return i; }
    /// get the ith coodinate of a point as a (possibly approximated) double
    double coord(const Delaunay_triangulation& dt, const Point& p, int i) const { return {}; }
    /// locate the vertex which point is equal to p, otherwise return the infinite vertex
    Vertex_const_handle locate_vertex(const Delaunay_triangulation& dt, const Point& p) const { return {}; }
/// @}


/// \name Tests
/// @{
    /// test if a vertex is the infinite vertex
    bool vertex_is_infinite(const Delaunay_triangulation& dt, Vertex_const_handle v) const { return {}; }
    /// test if a facet is infinite (incident to the infinite vertex)
    bool facet_is_infinite(const Delaunay_triangulation& dt, Facet_const_handle f) const { return {}; }
    /// test if a cell is infinite (incident to the infinite vertex)
    bool cell_is_infinite(const Delaunay_triangulation& dt, Cell_const_handle c) const { return {}; }
    /// test if two vertices from possibly different triangulations have the same point embedding
    bool are_vertices_equal(const Delaunay_triangulation& t1, Vertex_const_handle v1, const Delaunay_triangulation& t2, Vertex_const_handle v2) const { return {}; }
    /// test if two facets from possibly different triangulations have the same point embeddings
    bool are_facets_equal(const Delaunay_triangulation& t1, Facet_const_handle f1, const Delaunay_triangulation& t2, Facet_const_handle f2) const { return {}; }
    /// test if two cells from possibly different triangulations have the same point embeddings
    bool are_cells_equal(const Delaunay_triangulation& t1, Cell_const_handle c1, const Delaunay_triangulation& t2, Cell_const_handle c2) const { return {}; }
/// @}


/// \name Triangulation Data Structure traversal
/// @{
    /// get the ith vertex of a cell
    Vertex_const_handle vertex(const Delaunay_triangulation& dt, Cell_const_handle c, int i) const { return {}; }
    /// get the facet incident to a cell, which covertex is the ith vertex of the cell
    Facet_const_iterator facet(const Delaunay_triangulation& dt, Cell_const_iterator c, int i) const { return {}; }
    /// get the index of the covertex of a facet
    int index_of_covertex(const Delaunay_triangulation& dt, Facet_const_handle f) const { return {}; }
    /// get the covertex of a facet, which is the only vertex of cell containing the facet that is not incident to the facet
    Vertex_const_handle covertex(const Delaunay_triangulation& dt, Facet_const_handle f) const { return {}; }
    /// get the covertex of the mirror of a facet
    Vertex_const_handle mirror_vertex(const Delaunay_triangulation& dt, Facet_const_handle f) const { return {}; }
    /// get the cell incident to a facet
    Cell_const_handle cell(const Delaunay_triangulation& dt, Facet_const_handle f) const { return {}; }
    /// get one of the cells incident to a vertex
    Cell_const_handle cell(const Delaunay_triangulation& dt, Vertex_const_handle f) const { return {}; }
    /// get the mirror facet of a facet
    Facet_const_handle mirror_facet(const Delaunay_triangulation& dt, Facet_const_handle f) const { return {}; }
    /// get the mirror index of a facet, which is the index of the covertex of its mirror facet
    int mirror_index(const Delaunay_triangulation& dt, Facet_const_handle f) const { return {}; }
    /// get the index of a cell in its ith neighbor
    int mirror_index(const Delaunay_triangulation& dt, Cell_const_handle c, int i) const { return {}; }
    /// get the neighboring cell of a cell, opposite to its itg vertex
    Cell_const_iterator neighbor(const Delaunay_triangulation& dt, Cell_const_iterator c, int i) const { return {}; }
/// @}

/// \name Streaming
/// @{
    std::ostream& write(std::ostream& out, const Delaunay_triangulation& dt) const { return out; }
    std::istream& read(std::istream& in, Delaunay_triangulation& dt) const { return in; }
/// @}


private:
    Point p;
    Info i;
}; /* end TriangulationTraits */

#endif // CGAL_DDT_CONCEPT_TRIANGULATION_TRAITS
