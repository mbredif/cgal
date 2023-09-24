#ifndef CGAL_DDT_CONCEPT_KERNEL_TRAITS
#define CGAL_DDT_CONCEPT_KERNEL_TRAITS

#include <CGAL/DDT/kernel/Kernel_traits.h>
#include <Concepts/Point.h>
#include <Concepts/Bbox.h>

namespace CGAL {
namespace DDT {

template<> struct Kernel_traits<::Point> {

/// \name Types
/// @{
///
#ifdef DOXYGEN_RUNNING
private:
    struct unspecified_type {};
public:
    /// point type
    typedef unspecified_type Point;

    /// point const reference type
    typedef unspecified_type Point_const_reference;

    /// A model of the `Bbox` concept
    typedef unspecified_type Bbox;

    /// the ambient dimension if it is static, 0 if it is dynamic
    static constexpr int D;
#else
    typedef ::Point Point;
    typedef ::Point const& Point_const_reference;
    typedef ::Bbox Bbox;
    static constexpr int D=33;
#endif


    /// @}

    /// compares the `i`'th Cartesian coodinate of `p` and `q`
    static bool less_coordinate(Point_const_reference p, Point_const_reference q, int i) { return true; }


    static inline Bbox bbox(int dim) {
        return Impl::b;
    }

    static inline Bbox bbox(Point_const_reference p) {
        return Impl::b;
    }

    static inline Bbox bbox(Point_const_reference p, Point_const_reference q) {
        return Impl::b;
    }

    template<typename InputIterator>
    static inline Point point(InputIterator begin, InputIterator end) {
        return Impl::p;
    }

    static inline Point point(int /*dim*/) {
        return Impl::p;
    }


};

/// returns the ith coodinate of a point as a (possibly approximated) double
double approximate_cartesian_coordinate(const ::Point& p, int i)
{
    return 0;
}


::Bbox make_bbox(const ::Point& p, const ::Point& q)
{
    return Impl::b;
}

}
}

#endif // CGAL_DDT_CONCEPT_KERNEL_TRAITS
