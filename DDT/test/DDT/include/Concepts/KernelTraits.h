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
};

template<typename InputIterator>
void assign(::Point&, InputIterator begin, InputIterator end) {}

/// compares the `i`'th Cartesian coodinate of `p` and `q`
bool less_coordinate(::Point const & p, ::Point const & q, int i) { return true; }

/// returns the ith coodinate of a point as a (possibly approximated) double
double approximate_cartesian_coordinate(const ::Point& p, int i)
{
    return 0;
}

void assign(::Bbox&, int dim) {}
void assign(::Bbox&, ::Point const & p) {}
void assign(::Bbox&, ::Point const & p, ::Point const & q) {}
template<typename InputIterator0, typename InputIterator1>
void assign(::Bbox&, InputIterator0 begin0, InputIterator0 end0, InputIterator1 begin1, InputIterator1 end1) {}


}
}

#endif // CGAL_DDT_CONCEPT_KERNEL_TRAITS
