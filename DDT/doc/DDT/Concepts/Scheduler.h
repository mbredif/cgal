
/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `Scheduler` describes the requirements for the scheduling of the tasks
manipulating tiles stored in a tile container (for example computing the distributed
Delaunay triangulation).


\cgalHasModel `CGAL::DDT::Sequential_scheduler`
\cgalHasModel `CGAL::DDT::TBB_scheduler`
\cgalHasModel `CGAL::DDT::MPI_scheduler` (wip)

*/

class Scheduler {
public:

/// \name Scheduling Functions
/// @{
    /// \brief executes a function on all values of the assiociative container `c`, calling `transform(k,v)` on each key-value pair `k,v` in `c`.
    /// \param c an associative container.
    /// \param init initial value of the aggregation
    /// \param transform function called on each value
    /// \param reduce function called to produce the aggregated result

    /// \tparam Container a model of an `AssociativeContainer`
    /// \tparam V the return type of the function
    /// \tparam Transform a model of `Callable` with a `key_type` value and a `mapped_type` reference of a `Container` as its argument type
    /// \tparam Reduce a model of `Callable` with `V` and `U` as argument types and `V` as its return type
    /// \return the reduction by `reduce` of the values returned by `transform`, starting at the value `init`.
    template<typename Container,
             typename V,
             typename Transform,
             typename Reduce = std::plus<>>
    V transform_reduce(Container& c, V init, Transform transform, Reduce reduce = {}) { return init; }

    /// \brief joins two associative containers `c1` and `c2` by key and aggregate, using a `reduce` function operating on the initial value `init_v`,
    /// the values returned by `transform(k, v1, v2)` on all values `v1` and `v2` that share a key `k`.
    /// This is a right join : if `c1` has no elements for a key `k` in `c2`, then an object is emplaced in `c1` at this key and constructed with `(key, args...)`
    /// \param c1 an associative container.
    /// \param c2 an associative container.
    /// \param init initial value of the aggregation
    /// \param transform function called on each joined values
    /// \param reduce function called to produce the aggregated result
    /// \param args extra arguments to the constructor for object when a key in `c2` is not present in `c1`
    ///
    /// \tparam Container1 a model of an `AssociativeContainer`
    /// \tparam Container2 a model of an `AssociativeContainer`, `Container1` and `Container2` must share a common `key_type`
    /// \tparam V the return type of the function
    /// \tparam Transform a model of `Callable` with a `key_type` value and `mapped_type` references of the `Container`s as its argument type
    /// \tparam Reduce a model of `Callable` with `V` and `V` as argument types and `V` as its return type
    /// \tparam Args... types for the extra constructor arguments
    /// \return the reduction by `reduce` of the values returned by `transform`, starting at the value `init`.
    template<typename Container1,
             typename Container2,
             typename V,
             typename Transform,
             typename Reduce = std::plus<>,
             typename... Args>
    V join_transform_reduce(Container1& c1, Container2& c2, V init, Transform transform, Reduce reduce = {}, Args... args) { return init; }

    /// \brief repeatedly joins two associative containers `c1` and `c2` by key and aggregate `v=reduce(v, u)` with `u=transform(k,v1,v2)` and
    ///        values `v1,v2` sharing a key `k` in the containers `c1,c2`, starting with `v=init`. Repeated joins occur until convergence, meaning that
    ///        `reduce(v, u)==v` for all keys in `c2`. There is no requirement on the order and the number of calls of `transform` on the tiles,
    ///        the only requirement is that any further call to `transform` verifies the aforementioned invariant.
    /// \copydetails join_transform_reduce
    template<typename Container1,
             typename Container2,
             typename V,
             typename Transform,
             typename Reduce = std::plus<>,
             typename... Args>
    V join_transform_reduce_loop(Container1& c1, Container2& c2, V init, Transform transform, Reduce reduce = {}, Args... args) { return init; }
/// @}

};
/* end Scheduler */
