
/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `Scheduler` describes the requirements for the
...

\cgalHasModel `ddt::sequential_scheduler`
\cgalHasModel `ddt::multithread_scheduler`
\cgalHasModel `ddt::tbb_scheduler`
\cgalHasModel `ddt::mpi_scheduler` (wip)

*/

namespace ddt
{

class Scheduler {
public:

/// \name Types
/// @{

    /// the type of a tile
    typedef unspecified_type Tile;

    /// the type of a (Vertex_const_handle, id) pair
    typedef unspecified_type Vertex_const_handle_and_id;

    /// the type of a vertex cont handle in the tile
    typedef unspecified_type Vertex_const_handle;

    /// the type of a point in the tile
    typedef unspecified_type Point;

    /// the type of a tile identifier
    typedef unspecified_type Id;

    /// a container of (Point, Id) pairs, used to retrieve received points
    typedef unspecified_type Point_id_container;

/// @}


    /// default constructor
    Scheduler();

    /// a hint on the degree of parallelism (number of threads, of mpi processes...). returns 1 for sequential schedulers.
    int number_of_threads() const;


/// \name Messaging Functions
/// @{
    /// \brief receive Point_ids sent to a tile
    /// \param id the id of the receiving tile
    /// \param received an empty container of Point_id, which will be filled with received elements.
    /// After calling this function, the inbox for tile id is empty
    void receive(Id id, Point_id_container& received);

    /// \brief send a single Point and Id to a tile
    /// \param p  the point
    /// \param id the id of the tile in which the point p is local
    /// \param target  the id of the tile that will receive the point
    void send(const Point& p, Id id, Id target);

    /// \brief send a single vertex to a tile
    /// \param tile   a tile.
    /// \param v a handle to a vertex contained in the tile.
    /// \param target  the id of the tile that will receive the point and id of v
    bool send_vertex(const Tile& tile, Vertex_const_handle v, Id target);

    /// \brief send a range of (vertex, target) pairs
    /// \param tile   a tile.
    /// \param outbox the range of (vertex, target) pairs. All vertex handles must be part of the input tile.
    int send_one(const Tile& tile, std::vector<Vertex_const_handle_and_id>& outbox);

    /// \brief send (broadcast) a range of vertices to a range of tile ids
    /// \param tile   a tile.
    /// \param outbox the range of vertices.
    /// \param begin,end the range of target tile identifiers
    template<typename Id_iterator>
    int send_all(const Tile& tile, std::vector<Vertex_const_handle>& outbox, Id_iterator begin, Id_iterator end);
/// @}

/// \name Iteration Functions
/// @{
    /// \brief execute, possibly in parallel, a function on all tiles given its range of identifiers
    /// \param tc   a tile container.
    /// \param begin,end the range of tile identifiers
    /// \param func the the function object
    /// \return the sum of the returned values
    template<typename TileContainer, typename Id_iterator>
    int for_each(TileContainer& tc, Id_iterator begin, Id_iterator end, const std::function<int(Tile&)>& func);

    /// \brief execute, possibly in parallel, a function on all tiles that have received at least one point.
    /// The function call func(t), on a tile t, may receive points to the tile t, and send points from this tile t.
    /// Func is called exactly once on each tile that received at least one point.
    /// \param tc   a tile container.
    /// \param func the the function object
    /// \return the sum of the returned values
    template<typename TileContainer>
    int for_each(TileContainer& tc, const std::function<int(Tile&)>& func);

    /// \brief execute recursively, possibly in parallel, a function on all tiles that have received at least one point.
    /// The function call func(t), on a tile t, may receive points to the tile t, and send points from this tile t.
    /// The iterations stop when all points have been received, possibly calling func multiple times on each tile.
    /// \param tc   a tile container.
    /// \param func the the function object
    /// \return the sum of the returned values
    template<typename TileContainer>
    int for_each_rec(TileContainer& tc, const std::function<int(Tile&)>& func);
/// @}


}; /* end Scheduler */

}
