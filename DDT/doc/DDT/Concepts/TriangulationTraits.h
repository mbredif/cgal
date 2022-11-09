
/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `TriangulationTraits` describes the requirements of a traits that adapts a Delaunay Triangulation implementation.

\cgalHasModel `ddt::Cgal_traits_2`
\cgalHasModel `ddt::Cgal_traits_3`
\cgalHasModel `ddt::Cgal_traits_d`
\cgalHasModel `ddt::Cgal_traits`

*/

class TriangulationTraits {
public:

/// \name Types
/// @{

    typedef unspecified_type Point;
    typedef unspecified_type Id;
    typedef unspecified_type Flag;
    typedef unspecified_type TDS;

    typedef unspecified_type Vertex_const_iterator;
    typedef unspecified_type Vertex_const_handle;
    typedef unspecified_type Vertex_iterator;
    typedef unspecified_type Vertex_handle;

    typedef unspecified_type Cell_const_iterator;
    typedef unspecified_type Cell_const_handle;
    typedef unspecified_type Cell_iterator;
    typedef unspecified_type Cell_handle;

    typedef unspecified_type Facet;
    typedef unspecified_type Facet_const_iterator;
    typedef unspecified_type Facet_const_handle;
    typedef unspecified_type Facet_iterator;
    typedef unspecified_type Facet_handle;

    typedef unspecified_type Delaunay_triangulation;
    typedef unspecified_type Random_points_in_ball;
    typedef unspecified_type Random_points_in_box;

/// @}

    /// default constructor
    TriangulationTraits();
    /// construct an empty triangulation of the given dimension
    Delaunay_triangulation triangulation(int dimension) const;
    /// get the current dimension of a Delaunay triangulation
    int current_dimension(const Delaunay_triangulation& dt) const;
    /// get the maximal dimension of a Delaunay triangulation
    int maximal_dimension(const Delaunay_triangulation& dt) const;
    /// get the number of cells, including infinite cells
    size_t number_of_cells(const Delaunay_triangulation& dt) const;
    /// get the number of vertices, including the infinite vertex
    size_t number_of_vertices(const Delaunay_triangulation& dt) const;

/// \name Iterators
/// @{
    /// begin const iterator on the set of vertices
    Vertex_const_iterator vertices_begin(const Delaunay_triangulation& dt) const;
    /// begin const iterator on the set of vertices
    Vertex_const_iterator vertices_end(const Delaunay_triangulation& dt) const;
    /// begin iterator on the set of vertices
    Vertex_iterator vertices_begin(Delaunay_triangulation& dt) const;
    /// end iterator on the set of vertices
    Vertex_iterator vertices_end(Delaunay_triangulation& dt) const;
    /// begin iterator on the set of facets
    Facet_const_iterator facets_begin(const Delaunay_triangulation& dt) const;
    /// end iterator on the set of facets
    Facet_const_iterator facets_end(const Delaunay_triangulation& dt) const;
    /// begin iterator on the set of cells
    Cell_const_iterator cells_begin(const Delaunay_triangulation& dt) const;
    /// end iterator on the set of cells
    Cell_const_iterator cells_end(const Delaunay_triangulation& dt) const;
    /// get the infinite vertex of a triangulation
    Vertex_handle infinite_vertex(const Delaunay_triangulation& dt) const;
/// @}


/// \name Triangulation Modification
/// @{
    /// clear the triangulation
    void clear(Delaunay_triangulation& dt) const;
    /// remove a range of (point,id) pairs to the triangulation
    template<class It> void insert(Delaunay_triangulation& dt, It begin, It end) const;
    /// remove a range of vertex handles from the triangulation
    template<class It> void remove(Delaunay_triangulation& dt, It begin, It end) const;
/// @}


/// \name Accessors
/// @{
    /// access the point embedding of a vertex
    const Point& point(const Delaunay_triangulation& dt, Vertex_const_handle v) const;
    /// access the identifier of the tile where this vertex is local
    Id    id  (Vertex_const_handle v) const;
    /// access the flag of the vertex
    Flag& flag(Vertex_const_handle v) const;
    /// compute the circumcenter of the given full cell.
    Point circumcenter(const Delaunay_triangulation& dt, Cell_const_handle c) const;
    /// get the ith coodinate of a point as a (possibly approximated) double
    double coord(const Delaunay_triangulation& dt, const Point& p, int i) const;
/// @}


/// \name Tests
/// @{
    /// test if a vertex is the infinite vertex
    bool vertex_is_infinite(const Delaunay_triangulation& dt, Vertex_const_handle v) const;
    /// test if a facet is infinite (incident to the infinite vertex)
    bool facet_is_infinite(const Delaunay_triangulation& dt, Facet_const_handle f) const;
    /// test if a cell is infinite (incident to the infinite vertex)
    bool cell_is_infinite(const Delaunay_triangulation& dt, Cell_const_handle c) const;
    /// test if two vertices from possibly different triangulations have the same point embedding
    bool are_vertices_equal(const Delaunay_triangulation& t1, Vertex_const_handle v1, const Delaunay_triangulation& t2, Vertex_const_handle v2) const;
    /// test if two facets from possibly different triangulations have the same point embeddings
    bool are_facets_equal(const Delaunay_triangulation& t1, Facet_const_handle f1, const Delaunay_triangulation& t2, Facet_const_handle f2) const;
    /// test if two cells from possibly different triangulations have the same point embeddings
    bool are_cells_equal(const Delaunay_triangulation& t1, Cell_const_handle c1, const Delaunay_triangulation& t2, Cell_const_handle c2) const;
/// @}


/// \name Triangulation Data Structure traversal
/// @{
    /// get the ith vertex of a cell
    Vertex_const_handle vertex(const Delaunay_triangulation& dt, Cell_const_handle c, int i) const;
    /// get the covertex of a facet, which is the only vertex of cell containing the facet that is not incident to the facet
    Vertex_const_handle covertex(const Delaunay_triangulation& dt, Facet_const_handle f) const;
    /// get the covertex of the mirror of a facet
    Vertex_const_handle mirror_vertex(const Delaunay_triangulation& dt, Facet_const_handle f) const;
    /// get the cell incident to a facet
    Cell_const_handle cell(const Delaunay_triangulation& dt, Facet_const_handle f) const;
    /// get the mirror facet of a facet
    Facet_const_handle mirror_facet(const Delaunay_triangulation& dt, Facet_const_handle f) const;
    /// get the neighboring cell of a cell, opposite to its itg vertex
    Cell_const_iterator neighbor(const Delaunay_triangulation& dt, Cell_const_iterator c, int i) const;
    /// get the facet incident to a cell, which covertex is the ith vertex of the cell
    Facet_const_iterator facet(const Delaunay_triangulation& dt, Cell_const_iterator c, int i) const;
    /// get the index of the covertex of a facet
    int index_of_covertex(const Delaunay_triangulation& dt, Facet_const_handle f) const;
    /// get the mirror index of a facet, which is the index of the covertex of its mirror facet
    int mirror_index(const Delaunay_triangulation& dt, Facet_const_handle f) const;
    /// get the index of a cell in its ith neighbor
    int mirror_index(const Delaunay_triangulation& dt, Cell_const_handle c, int i) const;
/// @}


    }; /* end TriangulationTraits */
};
