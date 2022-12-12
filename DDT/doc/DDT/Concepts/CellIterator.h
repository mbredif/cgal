
/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `CellIterator` describes the requirements of for the cell iterator of the tile triangulations.

\cgalHasModel `CGAL::DDT::Cgal_traits_2::Cell_iterator`
\cgalHasModel `CGAL::DDT::Cgal_traits_3::Cell_iterator`
\cgalHasModel `CGAL::DDT::Cgal_traits_d::Cell_iterator`
\cgalHasModel `CGAL::DDT::Cgal_traits::Cell_iterator`
*/

#ifndef CGAL_DDT_CONCEPT_CELL_ITERATOR
#define CGAL_DDT_CONCEPT_CELL_ITERATOR

struct CellIterator {
    CellIterator& operator++() { return *this; }
    bool operator< (CellIterator) const { return false; }
    bool operator!=(CellIterator) const { return false; }
    bool operator==(CellIterator) const { return false; }
};

#endif // CGAL_DDT_CONCEPT_CELL_ITERATOR
