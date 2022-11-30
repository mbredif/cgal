
/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `Selector` describes the requirements for the selection of a value given an unordered set of values, possibly with repeated values.
The selector is not required to store these values, but the selected value should not depend on their order of insertion.
A model of the `Selector` concept is used in each Tile to select the main id of its simplices (cells and facets).
This main id is selected among the set of tile identifiers of its incident finite vertices.

`CGAL::DDT::Minimum_selector` and `CGAL::DDT::Maximum_selector` are the most efficient to compute (no need to store all inserted values).
While `CGAL::DDT::Median_selector` stores all inserted values in order to extract the median when dereferenced, it has interesting properties :
- It ensures that a simplex is main if the majority of its finite vertices are local.
- It provides a more balanced distribution of the main simplices across tiles : `CGAL::DDT::Minimum_selector` (resp. `CGAL::DDT::Maximum_selector`)
assigns more main simplices to low (resp. high) id tiles.

\cgalHasModel `CGAL::DDT::Minimum_selector`
\cgalHasModel `CGAL::DDT::Maximum_selector`
\cgalHasModel `CGAL::DDT::Median_selector`

@todo benchmark selectors to set the default (currently : `CGAL::DDT::Median_selector`)

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

    /// Dereferences as the selected value
    inline value_type operator*();

}; /* end Selector */
