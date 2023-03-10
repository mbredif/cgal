#ifndef DOXYGEN_RUNNING
/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `Selector` describes the requirements for the selection of a value given an unordered set of values, possibly with repeated values.
The selector is not required to store these values, but the selected value should be deterministic and should not depend on the insertion order.
A model of the `Selector` concept is used in each `Tile` to select the main id of its simplices (cells and facets).
This main id is selected among the set of tile identifiers of its incident finite vertices.

`CGAL::DDT::Minimum_selector` and `CGAL::DDT::Maximum_selector` are the most efficient to compute (no need to store all inserted values, it suffices to keep the running optimum).
While `CGAL::DDT::Median_selector` stores all inserted values in order to extract the median with `select()`, it has interesting properties :
- It ensures that a simplex is main if the majority of its finite vertices are local.
- It provides a more balanced distribution of the main simplices across tiles : `CGAL::DDT::Minimum_selector` (resp. `CGAL::DDT::Maximum_selector`)
assigns more main simplices to low (resp. high) id tiles.

\cgalHasModel `CGAL::DDT::Minimum_selector`
\cgalHasModel `CGAL::DDT::Maximum_selector`
\cgalHasModel `CGAL::DDT::Median_selector` (default `Selector` in `CGAL::DDT::Tile`)

*/

template<typename T>
struct Selector
{
    /// Type of the values.
    typedef T value_type;

    /// Empty constructor.
    Selector();

    /// Inserts a value in the set.
    void insert(value_type v);

    /// Reinitialize the set of values.
    void clear();

    /// Gets the selected value
    inline value_type select();

}; /* end Selector */

#endif
