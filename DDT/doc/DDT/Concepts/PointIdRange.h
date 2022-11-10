
/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `PointIdRange` describes the requirements for the
...

\cgalHasModel `CGAL::DDT::DDT`

*/

namespace CGAL {
namespace DDT {

class PointIdRange {
public:

/// \name Types
/// @{

    /*!
    Must be the same as the point type `TriangulationTraits_2::Point_2`
    defined by the geometric traits class of the triangulation.
    */
    typedef unspecified_type Point;

    /*!
    Must be the same as the point type `TriangulationTraits_2::Point_2`
    defined by the geometric traits class of the triangulation.
    */
    typedef unspecified_type Id;

/// @}

/// \name Access Functions
/// @{


/// @}


}; /* end TileContainer */

}
}
