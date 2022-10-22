
/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `Scheduler` describes the requirements for the
...

\cgalHasModel `ddt::DDT`

*/

class Scheduler {
public:

/// \name Types
/// @{

    /*!
    Must be the same as the point type `TriangulationTraits_2::Point_2`
    defined by the geometric traits class of the triangulation.
    */
    typedef unspecified_type Tile;

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

