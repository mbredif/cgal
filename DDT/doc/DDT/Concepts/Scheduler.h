
/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `Scheduler` describes the requirements for the
...

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

    /// default constructor
    Scheduler(int max_concurrency = 0);

    /// maximum concurrency (number of threads, of mpi processes...). returns 1 for sequential schedulers.
    int max_concurrency() const;


/// \name Iteration Functions
/// @{
    /// \brief execute, possibly in parallel, a function on all tiles that have received at least one point.
    /// The function calls op1(tc, tile), may receive points to the tile t, and send points from this tile t using tc.
    /// op1 is called exactly once on each tile.
    /// \param tc  a tile container.
    /// \param op1 unary function object
    /// \param op2 binary function object
    /// \param init value
    /// \return the reduction by op2 of the values return by op1, starting at the init value
    template<typename TileContainer, typename UnaryOp, typename V = int, typename BinaryOp = std::plus<>>
    V for_each(TileContainer& tc, UnaryOp op1, BinaryOp op2 = {}, V init = {});

    /// \brief execute, possibly in parallel, a function on all tiles that have received at least one point.
    /// The function calls op1(tc, tile), may receive points to the tile t, and send points from this tile t using tc.
    /// op1 is called on repeatedly on each tile until all tiles return the init value.
    /// \param tc  a tile container.
    /// \param op1 unary function object
    /// \param op2 binary function object
    /// \param init value
    /// \return the reduction by op2 of the values return by op1, starting at the init value
    template<typename TileContainer, typename UnaryOp, typename V = int, typename BinaryOp = std::plus<>>
    V for_each_rec(TileContainer& tc, UnaryOp op1, BinaryOp op2 = {}, V init = {});
/// @}

}; /* end Scheduler */
