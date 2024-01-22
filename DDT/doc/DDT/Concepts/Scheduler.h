
/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `Scheduler` describes the requirements for the scheduling and execution of tasks that operate on
key/value pairs stored in one or two containers model of the concept `AssociativeContainer`.

These tasks are performed by applying a `transform` function to each range of equivalent keys in the container.
The ordering of the `transform` calls is however unspecified and may be performed in parallel.

The member functions that take `Value` and `Reduce` arguments return the aggregation of the results of the `transform` calls
on all `(first,last)` ranges, as if by sequentially computing `value=reduce(value, transform(first,last, ...))`.

The member functions that take a `Transform` with an `OutputIterator` argument enable function calls to return an arbitrary number of elements.

\cgalHasModelsBegin
\cgalHasModels{CGAL::DDT::Sequential_scheduler}
\cgalHasModels{CGAL::DDT::TBB_scheduler}
\cgalHasModels{CGAL::DDT::MPI_scheduler}
\cgalHasModelsEnd
*/
#include <utility> // pair

class Scheduler {
public:

/// \name Functions
/// @{

    /// \brief executes a function on all ranges of values with equivalent keys in the associative container `c`, calling `transform(first,last,out)`
    ///        on each such range `[first,last)` in `c`.
    ///        If `Container` is non-const, then non-const iterators are passed to `transform` and its values may be modified.
    /// \param c an associative container
    /// \param transform function called on each range of equivalent elements of `c`
    /// \param out output iterator
    /// \tparam OutputValue the type that is put in `out` by `transform`
    /// \tparam Container a model of `AssociativeContainer` that supports equivalent keys
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
    ///        on each such range `[first,last)` in `c`.
    ///        If `Container` is non-const, then non-const iterators are passed to `transform` and its values may be modified.
    /// \todo reduction is not mentionned in the brief/long description
    /// \todo maybe put tparam Transform before `Reduce`, that way it is easier to define what is `U`
    /// \param c an associative container
    /// \param transform function called on each range
    /// \param value initial value of the reduction
    /// \param reduce function called to produce the reduced result
    /// \param out output iterator
    /// \tparam OutputValue the type that is put in `out` by `transform`
    /// \tparam Container a model of `AssociativeContainer` that supports equivalent keys
    /// \tparam OutputIterator a model of `OutputIterator` used by `transform`
    /// \tparam V the return type of a call to `reduce`
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
    ///        calling `v=transform(first,last)` on each such range `[first,last)` in `c` and aggregating each returned value `v`
    ///        using `value=reduce(value,v)`.
    ///        If `Container` is non-const, then non-const iterators are passed to `transform` and its values may be modified.
    /// \todo reduction not mentioned in brief/long description MB: done
    /// \todo maybe put tparam Transform before `Reduce`, that way it is easier to define what is `U` MB: done
    /// \param c an associative container
    /// \param transform function called on each range
    /// \param value initial value of the reduction
    /// \param reduce function called to produce the reduced result
    /// \tparam Container a model of `AssociativeContainer` that supports equivalent keys
    /// \tparam Transform a model of `Callable` with 2 `Container::(const_)iterator`s as argument types and calling `U` its return type.
    /// \tparam V the type of the aggregated value.
    /// \tparam Reduce a model of `Callable` with `V` and `U` as argument types and `V` as its return type
    /// \return `value` the reduced value after all updates by `transform`.
    ///         The reduced value is computed by applying `value=reduce(value, transform(first,last))` on all aforementioned ranges.
    template<typename Container,
             typename Transform,
             typename V,
             typename Reduce>
    V ranges_reduce(Container& c, Transform transform, V value, Reduce reduce) { return value; }

    /// \brief executes a function on all ranges of values with equivalent keys in the associative container `c1`,
    ///        calling `transform(first1,last1,v2,out3)` on each such range `[first1,last1)` in `c1`,
    ///        with `v2` being the value in `c2` associated in to the key `k=first1->first`.
    ///        This is a left join: if `c2` has no elements for an equivalent key `k` in `c1`, then an object is put in
    ///        in `c2` using `emplace(k, args2...)`.
    ///        If `Container1` is non-const, then non-const iterators are passed to `transform` and its values may be modified.
    ///        If `Container2` is non-const, then a non-const reference is passed to `transform` and its values may be modified.
    /// \todo question valid for all functions: it should be clear that the range of elements with equivalent keys must be of size > 1
    /// \param c1 an associative container
    /// \param c2 an associative container
    /// \param out3 output iterator
    /// \param transform function called on each joined values: `transform(first1,last1,v2,out3)`
    /// \param args2 extra arguments passed to construct `Container2::mapped_type(k,args2...)` when a key `k` in `c1` is not found in `c2`
    ///
    /// \tparam OutputValue3 a value type that can be put in `out3`
    /// \tparam Container1 a model of `AssociativeContainer` that supports equivalent keys
    /// \tparam Container2 a model of `AssociativeContainer` that supports unique keys, `Container1` and `Container2` have the same `key_type`.
    /// \tparam Transform a model of `Callable` with two `Container1::(const_)iterator`s as argument types `Container2::mapped_type`
    ///         and `OutputIterator3` as argument types, and `OutputIterator3` as return type.
    /// \tparam OutputIterator3 a model of `OutputIterator` used by `transform`
    /// \tparam Args2 extra types for constructing elements in `Container2`.
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

    /// \brief executes a function on all ranges of values with equivalent keys in the associative containers `c1` and `c3`,
    ///        calling `transform(first,last,v2,out)` on each such range `[first,last)` in `c1` and `c3`,
    ///        with `v2` being the value in `c2` associated in to the key `k=first1->first`
    ///        This is a left join: if `c2` has no elements for an equivalent key `k` in `c1`, then an object is put in
    ///        in `c2` using `emplace(k, args2...)`.
    ///        The output iterator `out` is constructed using `std::inserter(c3, c3.end())`, which enables the scheduling
    ///        of further calls to the `transform` function.
    ///        The aggregation into ranges of values from `c1` or `c3` (scheduled using `out`) is also unspecified. It is only
    ///        specified that the ranges passed to `transform` have equivalent keys and that they form a partition of all the values from `c1` and `c3`.
    ///        If `Container1` is non-const, then non-const iterators are passed to `transform` and its values may be modified.
    ///        If `Container2` is non-const, then a non-const reference is passed to `transform` and its values may be modified.
    /// \param c1 an associative container.
    /// \param c2 an associative container.
    /// \param c3 an associative container.
    /// \param transform function called on each joined values: `transform(first,last,v2,out)` where `(first,last)` is a range of equivalent keys from `c1` or `c3`
    /// \param args2 extra arguments passed to construct `Container2::mapped_type(k,args2...)` when a key `k` in `c1` or `c3` is not found in `c2`
    ///
    /// \tparam Container1 a model of `AssociativeContainer` that supports equivalent keys
    /// \tparam Container2 a model of `AssociativeContainer` that supports unique keys, `Container1` and `Container2` share a common `key_type`
    /// \tparam Container3 a model of `AssociativeContainer` that supports equivalent keys, insertion and range erasure
    /// \tparam Transform a model of `Callable` with 2 `Container(1/3)::(const_)iterator`s as argument types, a reference to `Container2::mapped_type`
    ///         and a model of `OutputIterator` as argument types, and `OutputIterator` as return type.
    /// \tparam Args2 extra types for constructing elements in `Container2`.
    /// \todo the role of c3 is not clear here. I don't get if c1 and c3 should both contain the ranges and if there is a recursion since values are put in c3
    template<typename Container1,
             typename Container2,
             typename Container3,
             typename Transform,
             typename... Args2>
    void ranges_for_each(Container1& c1, Container2& c2, Container3& c3, Transform transform, Args2&&... args2) { }
/// @}

    int thread_index() { return 0; }

#ifdef CGAL_DDT_TRACING
    struct unspecified_type {};
    struct {
        struct {
            template<typename T> auto operator<<(const T&) { return *this; }
        } out;
    } trace;
    unspecified_type clock_microsec() { return {}; }
    int process_index() { return 0; }
#endif

};
/* end Scheduler */
