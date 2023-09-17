#ifndef CGAL_DDT_CONCEPT_POINT_SET_TRAITS
#define CGAL_DDT_CONCEPT_POINT_SET_TRAITS

#include <CGAL/DDT/point_set/Point_set_traits.h>
#include <Concepts/Point.h>
#include <Concepts/Bbox.h>

namespace CGAL {
namespace DDT {

template<> struct Point_set_traits<::PointSet> {

/// \name Types
/// @{
///
#ifdef DOXYGEN_RUNNING
private:
    struct unspecified_type {};
public:
    /// point type
    typedef unspecified_type value_type;
    typedef unspecified_type const_iterator;
    typedef unspecified_type iterator;
#else
    typedef ::Point value_type;
    typedef ::TileIndex const_iterator;
    typedef ::TileIndex iterator;
#endif


    static std::size_t size(const PointSet& ps) { return 0; }

    static const value_type& point(const PointSet& ps, const_iterator v) {
        return Impl::p;
    }
    static void clear(PointSet& ps) { }

    /// @}
};

template<> struct Point_set_traits<::Triangulation> {

/// \name Types
/// @{
///
#ifdef DOXYGEN_RUNNING
private:
    struct unspecified_type {};
public:
    /// point type
    typedef unspecified_type value_type;
    typedef unspecified_type const_iterator;
    typedef unspecified_type iterator;
#else
    typedef ::Point     value_type;
    typedef ::TileIndex const_iterator;
    typedef ::TileIndex iterator;
#endif


    static std::size_t size(const PointSet& ps) { return 0; }

    static const value_type& point(const PointSet& ps, const_iterator v) {
        return Impl::p;
    }
    static void clear(PointSet& ps) { }

    /// @}
};

}
}

#endif // CGAL_DDT_CONCEPT_POINT_SET_TRAITS
