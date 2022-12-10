
/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `Scheduler` describes the requirements for the scheduling of the tasks
manipulating tiles stored in a tile container (for example computing the distributed
Delaunay triangulation).


\cgalHasModel `CGAL::DDT::Sequential_scheduler`
\cgalHasModel `CGAL::DDT::Multithread_scheduler`
\cgalHasModel `CGAL::DDT::TBB_scheduler`
\cgalHasModel `CGAL::DDT::MPI_scheduler` (wip)

*/

class Scheduler {
public:

/// \name Types
/// @{

    /// the type of a tile
    typedef unspecified_type Tile;
    /// the type of a tile identifier
    typedef unspecified_type Id;

/// @}

/// \name Scheduling Functions
/// @{
    /// \brief executes a function on all tiles of `tc`, calling `transform(tc, t)` on each tile `t` in `tc`.
    /// \param tc  a tile container.
    /// \param transform function called on each tile
    /// \param reduce function called to produce the aggregated result
    /// \param init initial value aggregated to the result with a call to `reduce`

    /// \tparam TileContainer a model of X
    /// \tparam Transform a model of `Callable` with `TileContainer&` and `Tile&` as argument types
    ///                   and with `V` implicitly constructible from the value type.
    /// \tparam Reduce a model of `Callable` with two instances of `V` as argument types and `V` as value type
    /// \tparam V return type of the function
    /// \return the reduction by `reduce` of the values returned by `transform`, starting at the value `init`.
    template<typename TileContainer,
             typename Transform,
             typename Reduce = std::plus<>,
             typename V = std::invoke_result_t<Reduce,
                                               std::invoke_result_t<Transform, TileContainer&, Tile&>,
                                               std::invoke_result_t<Transform, TileContainer&, Tile&> > >
    V for_each(TileContainer& tc, Transform transform, Reduce reduce = {}, V init = {});

    /// \brief repeatedly executes `transform(tc, t)` on all tiles `t` of `tc`, until each call returns a value `v`
    ///        such that for any value `w` the following invariant is verified `reduce(v, w)==w`.
    ///        There is no requirement on the order of the tiles on which `transform` is called,
    ///        the only requirement is that any further call to `transform` verifies the aforementioned invariant.
    /// \copydetails for_each
    template<typename TileContainer,
             typename Transform,
             typename Reduce = std::plus<>,
             typename V = std::invoke_result_t<Reduce,
                                               std::invoke_result_t<Transform, TileContainer&, Tile&>,
                                               std::invoke_result_t<Transform, TileContainer&, Tile&> > >
    V for_each_rec(TileContainer& tc, Transform transform, Reduce reduce = {}, V init = {});
/// @}

}; /* end Scheduler */
