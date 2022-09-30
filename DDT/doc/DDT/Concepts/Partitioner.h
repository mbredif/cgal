
/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `Partitioner` describes the requirements for the
...

\cgalHasModel `ddt::const_partitioner`
\cgalHasModel `ddt::grid_partitioner`
\cgalHasModel `ddt::random_partitioner`

*/

class Partitioner {
public:

/// \name Types
/// @{

    /*!
    Must be the same as the point type
    defined by the geometric traits class of the triangulation.
    */
    typedef unspecified_type Point;

    /*!
    The type of the tile identifier.
    */
    typedef unspecified_type Id;

/// @}

/// \name Access Functions
/// @{

/*!
returns tile Id of the given point.
*/

inline Id operator()(const Point& p) const;

/// @}


}; /* end Partitioner */

