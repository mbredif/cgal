
/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `Scheduler` describes the requirements for the scheduling and execution of tasks that operate on
key/value pairs stored in 1 or 2 `AssociativeContainer`s.

These tasks are performed by applying a `transform` function to each range of equivalent keys in the container.
The ordering of the `transform` calls is however unspecified and may be performed in parrallel.

The member functions that take `Value` and `Reduce` arguments return the aggregation of the results of the `transform` calls
on all `(first,last)` ranges, as if by sequentially computing `value=reduce(value, transform(first,last, ...))`.

The member functions that take a `Transform` with an `OutputIterator` argument enable function calls to return an arbitrary number of elements.

\cgalHasModel `CGAL::DDT::Sequential_scheduler`
\cgalHasModel `CGAL::DDT::TBB_scheduler`
\cgalHasModel `CGAL::DDT::MPI_scheduler` (work in progress)

*/
#include <utility> // pair

class Scheduler {
public:

/// \name Functions
/// @{

    /// \brief executes a function on all ranges of values with equivalent keys in the associative container `c`, calling `transform(first,last,out)`
    ///        on each such range `(first,last)` in `c`.
    ///        If `Container` is non-const, then non-const iterators are passed to `transform` and its values may be modified.
    /// \param c an associative container
    /// \param transform function called on each range
    /// \param out output iterator
    /// \tparam OutputValue the type that is assigned to `*out++` by `transform`
    /// \tparam Container a model of an `AssociativeContainer` that supports equivalent keys
    /// \tparam OutputIterator a model of `OutputIterator` used by `transform`
    /// \tparam Transform a model of `Callable` with 2 `Container::(const_)iterator`s and 1 `OutputIterator` as argument types,
    ///         and `OutputIterator` as return type.
    /// \return `out` after all updates by `transform`.
    template<typename OutputValue,
             typename Container,
             typename Transform,
             typename OutputIterator>
    OutputIterator ranges_transform(Container& c, Transform transform, OutputIterator out) { return out; }

    /// \brief executes a function on all ranges of values with equivalent keys in the associative container `c`, calling `transform(first,last,out)`
    ///        on each such range `(first,last)` in `c`.
    ///        If `Container` is non-const, then non-const iterators are passed to `transform` and its values may be modified.
    /// \param c an associative container
    /// \param transform function called on each range
    /// \param value initial value of the reduction
    /// \param reduce function called to produce the reduced result
    /// \param out output iterator
    /// \tparam OutputValue the type that is assigned to `*out++` by `transform`
    /// \tparam Container a model of an `AssociativeContainer` that supports equivalent keys
    /// \tparam OutputIterator a model of `OutputIterator` used by `transform`
    /// \tparam V the return type of the `reduce` function
    /// \tparam Reduce a model of `Callable` with `V` and a type `U` as argument types and `V` as its return type
    /// \tparam Transform a model of `Callable` with 2 `Container::(const_)iterator`s and 1 `OutputIterator` as argument types,
    ///         and `std::pair<U, OutputIterator>` as return type.
    /// \return `(value, out)` the reduced value and the output iterator after all updates by `transform`.
    ///         The reduced value is computed by applying `value=reduce(value, transform(first,last,out))` on all aforementioned ranges.
    template<typename OutputValue,
             typename Container,
             typename Transform,
             typename V,
             typename Reduce,
             typename OutputIterator>
    std::pair<V,OutputIterator>
    ranges_transform_reduce(Container& c, Transform transform, V value, Reduce reduce, OutputIterator out) { return { value, out }; }

    /// \brief executes a function on all ranges of values with equivalent keys in the associative container `c`,
    ///        calling `transform(first,last)` on each such range `(first,last)` in `c`.
    ///        If `Container` is non-const, then non-const iterators are passed to `transform` and its values may be modified.
    /// \param c an associative container
    /// \param transform function called on each range
    /// \param value initial value of the reduction
    /// \param reduce function called to produce the reduced result
    /// \tparam Container a model of an `AssociativeContainer` that supports equivalent keys
    /// \tparam V the return type of the `reduce` function
    /// \tparam Reduce a model of `Callable` with `V` and a type `U` as argument types and `V` as its return type
    /// \tparam Transform a model of `Callable` with 2 `Container::(const_)iterator`s as argument types and `U` as return type.
    /// \return `value` the reduced value after all updates by `transform`.
    ///         The reduced value is computed by applying `value=reduce(value, transform(first,last))` on all aforementioned ranges.
    template<typename Container,
             typename Transform,
             typename V,
             typename Reduce>
    V ranges_reduce(Container& c, Transform transform, V value, Reduce reduce) { return value; }

    /// \brief executes a function on all ranges of values with equivalent keys in the associative container `c1`,
    ///        calling `transform(first1,last1,v2,out3)` on each such range `(first1,last1)` in `c1`,
    ///        on the value `v2` associated in `c2` with the key `k=first1->first` and on the output iterator `out3`.
    ///        This is a left join : if `c2` has no elements for an equivalent key `k` in `c1`, then an object is `emplace`d
    ///        in `c2` at this key and constructed with `(k, args2...)`.
    ///        If `Container1` is non-const, then non-const iterators are passed to `transform` and its values may be modified.
    ///        If `Container2` is non-const, then a non-const reference is passed to `transform` and its values may be modified.
    /// \param c1 an associative container
    /// \param c2 an associative container
    /// \param out3 output iterator
    /// \param transform function called on each joined values: `transform(first1,last1,v2,out3)`
    /// \param args2 extra arguments passed to const.ruct `Container2::mapped_type(k,args2...)` when a key `k` in `c1` is not found in `c2`
    ///
    /// \tparam OutputValue3 a value type that may be output to `out3`
    /// \tparam Container1 a model of an `AssociativeContainer` that supports equivalent keys
    /// \tparam Container2 a model of an `AssociativeContainer` that supports unique keys, `Container1` and `Container2` share a common `key_type`
    /// \tparam Transform a model of `Callable` with 2 `Container1::(const_)iterator`s as argument types `Container2::mapped_type`
    ///         and `OutputIterator3` as argument types, and `OutputIterator3` as return type.
    /// \tparam OutputIterator3 a model of `OutputIterator` used by `transform`
    /// \tparam Args2 types for the extra constructor arguments
    /// \return the output iterator `out3`.
    template<typename OutputValue3,
             typename Container1,
             typename Container2,
             typename Transform,
             typename OutputIterator3,
             typename... Args2>
    OutputIterator3
    ranges_transform(Container1& c1, Container2& c2, Transform transform, OutputIterator3 out3, Args2&&... args2)
    { return out3; }

    /// \brief executes a function on all ranges of values with equivalent keys in the associative container `c1`,
    ///        calling `transform(first1,last1,v2,out)` on each such range `(first1,last1)` in `c1`,
    ///        on the value `v2` associated in `c2` with the key `k=first1->first` and a model of `OutputIterator`.
    ///        This is a left join : if `c2` has no elements for an equivalent key `k` in `c1`, then an object is `emplace`d
    ///        in `c2` at this key and constructed with `(k, args2...)`.
    ///        The output iterator `out` accepts assignments of `Container1::value_type` objects, which enables the scheduling
    ///        of further calls to the `transform` function.
    ///        The aggregation into ranges of values from `c1` or scheduled using `out` is also unspecified. It is only
    ///        specified that the ranges passed to `transform` have equivalent keys and that they form a partition of all the values from `c1` and `out`.
    ///        If `Container1` is non-const, then non-const iterators are passed to `transform` and its values may be modified.
    ///        If `Container2` is non-const, then a non-const reference is passed to `transform` and its values may be modified.
    /// \param c1 an associative container.
    /// \param c2 an associative container.
    /// \param transform function called on each joined values: `transform(first1,last1,v2,out)`
    /// \param args2 extra arguments passed to construct `Container2::mapped_type(k,args2...)` when a key `k` in `c1` is not found in `c2`
    ///
    /// \tparam Container1 a model of an `AssociativeContainer` that supports equivalent keys
    /// \tparam Container2 a model of an `AssociativeContainer` that supports unique keys, `Container1` and `Container2` share a common `key_type`
    /// \tparam Transform a model of `Callable` with 2 `Container1::(const_)iterator`s as argument types `Container2::mapped_type`
    ///         and a model of `OutputIterator` as argument types, and `OutputIterator` as return type.
    /// \tparam Args2 types for the extra constructor arguments
    template<typename Container1,
             typename Container2,
             typename Transform,
             typename... Args2>
    void ranges_for_each(Container1& c1, Container2& c2, Transform transform, Args2&&... args2) { }
/// @}

#ifdef CGAL_DDT_TRACING
    struct unspecified_type {};
    struct {
        struct {
            template<typename T> auto operator<<(const T&) { return *this; }
        } out;
    } trace;
    unspecified_type process_index() { return {}; }
    unspecified_type thread_index() { return {}; }
    unspecified_type clock_microsec() { return {}; }
#endif

};
/* end Scheduler */
