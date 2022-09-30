
/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `TileContainer` describes the requirements for the
...

\cgalHasModel `ddt::DDT`

*/

class TileContainer {
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

/*!
\return the number of inserted points
*/

std::size_t insert()(const PointRange& p);

/*!
\return the number of inserted points
*/

std::size_t insert()(const PointRange& p);

/// @}


}; /* end TileContainer */

