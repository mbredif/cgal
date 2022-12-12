
/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `FacetIterator` describes the requirements of for the facet iterator of the tile triangulations.

\cgalHasModel `CGAL::DDT::Cgal_traits_2::Facet_iterator`
\cgalHasModel `CGAL::DDT::Cgal_traits_3::Facet_iterator`
\cgalHasModel `CGAL::DDT::Cgal_traits_d::Facet_iterator`
\cgalHasModel `CGAL::DDT::Cgal_traits::Facet_iterator`

*/

#ifndef CGAL_DDT_CONCEPT_FACET_ITERATOR
#define CGAL_DDT_CONCEPT_FACET_ITERATOR

struct FacetIterator {
    FacetIterator& operator++() { return *this; }
    bool operator< (FacetIterator) const { return false; }
    bool operator!=(FacetIterator) const { return false; }
    bool operator==(FacetIterator) const { return false; }
};

#endif // CGAL_DDT_CONCEPT_FACET_ITERATOR
