
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
#include <utility> // pair

class Scheduler {
public:

/// \name Functions
/// @{

    /// \brief executes a function on all values of the associative container `c`, calling `transform(k,v,out)` on each key-value pair `k,v` in `c`.
    /// \param c an associative container.
    /// \param out output iterator
    /// \param transform function called on each value
    /// \tparam OutputValue a value type that may be output to `out`
    /// \tparam Container a model of an `AssociativeContainer`
    /// \tparam OutputIterator an output iterator used by `Transform`
    /// \tparam Transform a model of `Callable` with `Container::key_type`, `Container::mapped_type` and `OutputIterator` as argument types,
    ///         and `OutputIterator` as return type.
    /// \return `out` after all updates by `transform`
    template<typename OutputValue,
             typename Container,
             typename OutputIterator,
             typename Transform>
    OutputIterator flat_map(Container& c, OutputIterator out, Transform transform) { return out; }

    /// \brief executes a function on all ranges of values with equivalent keys from the associative container `c`, calling `transform(range,out)` on each such `range` in `c`.
    /// \param c an associative container.
    /// \param out output iterator
    /// \param value initial value of the reduction
    /// \param reduce function called to produce the reduced result
    /// \param transform function called on each range
    /// \tparam OutputValue a value type that may be output to `out`
    /// \tparam Container a model of an `AssociativeContainer` that supports equivalent keys
    /// \tparam OutputIterator an output iterator used by `Transform`
    /// \tparam V the return type of the function
    /// \tparam Reduce a model of `Callable` with `V` and a type `U` as argument types and `V` as its return type
    /// \tparam Transform a model of `Callable` with `Container::key_type`, `Container::mapped_type` and `OutputIterator` as argument types,
    ///         and `std::pair<U, OutputIterator>` as return type.
    /// \return `(value, out)` the reduced value and the output iterator after all updates by `transform`.
    ///         The reduced value is computed by applying `value=reduce(value, transform(range, out))` on all aforementioned ranges.
    template<typename OutputValue,
             typename Container,
             typename OutputIterator,
             typename V,
             typename Reduce,
             typename Transform>
    std::pair<V,OutputIterator>
    reduce_by_key(Container& c, OutputIterator out, V value, Reduce reduce, Transform transform) { return { value, out }; }

    /// \brief executes a function on all values of the associative container `c`, calling `transform(k,v)` on each key-value pair `k,v` in `c`.
    /// \param c an associative container.
    /// \param value initial value of the reduction
    /// \param transform function called on each value
    /// \param reduce function called to produce the reduced result

    /// \tparam Container a model of an `AssociativeContainer`
    /// \tparam V the return type of the function
    /// \tparam Transform a model of `Callable` with a `key_type` value and a `mapped_type` reference of a `Container` as its argument type
    /// \tparam Reduce a model of `Callable` with `V` and `U` as argument types and `V` as its return type
    /// \return the reduction by `reduce` of the values returned by `transform`, starting at the value `value`.
    template<typename Container,
             typename V,
             typename Reduce,
             typename Transform>
    V transform_reduce(Container& c, V value, Reduce reduce, Transform transform) { return value; }

    /// \brief joins two associative containers `c1` and `c2` by key, by applying `transform(k, v1, v2)` on all values `v1` and `v2` that share a key `k`.
    /// This is a right join : if `c1` has no elements for a key `k` in `c2`, then an object is emplaced in `c1` at this key and constructed with `(key, args...)`
    /// \param c1 an associative container.
    /// \param c2 an associative container.
    /// \param out output iterator
    /// \param transform function called on each joined values
    /// \param args extra arguments passed to construct `Container1::mapped_type(k,args...)` when a key `k` in `c2` is not present in `c1`
    ///
    /// \tparam Container1 a model of an `AssociativeContainer` that supports unique keys
    /// \tparam Container2 a model of an `AssociativeContainer`, `Container1` and `Container2` share a common `key_type`
    /// \tparam OutputIterator an output iterator used by `Transform`
    /// \tparam Transform a model of `Callable` with `key_type`, `Container1::mapped_type`, `Container2::mapped_type` and `OutputIterator` as argument types,
    ///         and `OutputIterator` as return type.
    /// \tparam Args types for the extra constructor arguments
    /// \return the output iterator `out`.
    template<typename Container1,
             typename Container2,
             typename OutputIterator,
             typename Transform,
             typename... Args>
    OutputIterator
    join_transform_reduce(Container1& c1, Container2& c2, OutputIterator out, Transform transform, Args&&... args)
    { return out; }

    /// \brief repeatedly joins by key two associative containers `c1` and `c2`.
    ///        More precisely, iteratively pops values `(k,v2)` from `c2` and looks the value `(k,v1)` in `c1`.
    ///        If no such pair `(k,v1)` exists in `c1`, it is first inserted in `c1` with `v1=Container1::mapped_type(k,args...)`.
    ///        Then `transform(k,v1,v2,out2))` is called, which may add elements in `c2` through `out2`.
    ///        Iteration ends when `c2.empty()` is `true`.
    /// \param c1 an associative container.
    /// \param c2 an associative container.
    /// \param out2 output iterator
    /// \param transform function called on each joined values and returns the output iterator
    /// \param args extra arguments passed to construct `Container1::mapped_type(k,args...)` when a key `k` in `c2` is not present in `c1`
    ///
    /// \tparam Container1 a model of an `AssociativeContainer` that supports unique keys
    /// \tparam Container2 a model of an `AssociativeContainer`, `Container1` and `Container2` share a common `key_type`
    /// \tparam OutputIterator an output iterator used by `Transform`
    /// \tparam Transform a model of `Callable` with `key_type`, `Container1::mapped_type`, `Container2::mapped_type` and `OutputIterator` as argument types,
    ///         and `OutputIterator` as return type.
    /// \tparam Args types for the extra constructor arguments
    template<typename Container1,
             typename Container2,
             typename OutputIterator2,
             typename Transform,
             typename... Args>
    void join_transform_reduce_loop(Container1& c1, Container2& c2, OutputIterator2 out2, Transform transform, Args&&... args) { }
/// @}

};
/* end Scheduler */
