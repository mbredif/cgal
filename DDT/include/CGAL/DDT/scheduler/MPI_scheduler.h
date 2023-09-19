// Copyright (c) 2022 Institut Géographique National - IGN (France)
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Mathieu Brédif and Laurent Caraffa

#ifndef CGAL_DDT_SCHEDULER_MPI_SCHEDULER_H
#define CGAL_DDT_SCHEDULER_MPI_SCHEDULER_H

#ifndef CGAL_LINKED_WITH_MPI
#error MPI not properly setup with CGAL
#endif

#include <mpi.h>
#include <map>
#include <vector>
#include <functional>
#include <CGAL/DDT/IO/trace_logger.h>

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTSchedulerClasses
/// \cgalModels Scheduler
struct MPI_scheduler
{
    /// constructor
    MPI_scheduler(int max_concurrency = 0, MPI_Comm comm = MPI_COMM_WORLD) : comm(comm)
    {
        // Initialize the MPI environment
        MPI_Init(NULL, NULL);

        // Get the number of processes
        MPI_Comm_size(comm, &comm_size);

        // Get the rank of the process
        MPI_Comm_rank(comm, &comm_rank);

        // Get the name of the processor
        int name_len;
        MPI_Get_processor_name(processor_name, &name_len);

#if CGAL_DDT_TRACING
        std::vector<char> processor_names;
        std::vector<int> displs;
        all_gather(processor_name, name_len, processor_names, displs);
        std::set<std::string> processor_nameset;
        int core_id = 0;
        for(int i = 0; i < comm_size; ++i) {
            processor_nameset.insert(std::string(processor_names.data()+displs[i], processor_names.data()+displs[i+1]));
            if (i < comm_rank && strncmp(processor_name, processor_names.data()+displs[i], name_len)==0) {
                core_id++;
            }
        }
        pid = std::distance(processor_nameset.begin(), processor_nameset.find(processor_name));

        std::ostringstream oss;
        oss << "perf_mpi." << comm_rank << ".json";
        trace.open(oss.str());

        // poor man's clock sync
        MPI_Barrier(comm);
        trace.t0 = MPI_Wtime();
        CGAL_DDT_TRACE1(*this, "", "thread_name", 0, "M", name, "\"" << processor_name << "[" << core_id << "]\"");
        CGAL_DDT_TRACE1(*this, "", "process_name", 0, "M", name, "\"" << processor_name << "\"");
#endif
    }

    ~MPI_scheduler() {
#if CGAL_DDT_TRACING
        trace.out.close();
        // collect all traces "perf_mpi.*.json" to "perf_mpi.json"
        std::ostringstream filename;
        filename << "perf_mpi." << comm_rank << ".json";
        std::ifstream f(filename.str());
        std::ostringstream oss;
        oss << f.rdbuf();
        std::string sendbuf(oss.str());
        std::vector<char> recvbuf;
        std::vector<int> displs;
        gather(sendbuf.c_str() + 1, sendbuf.size() - 1, recvbuf, displs, root_rank);

        if (comm_rank == root_rank) {
            std::ofstream out("perf_mpi.json");
            recvbuf[recvbuf.size() - 3] = 0; // do not use the 2 trailing characters ",\n"
            out << "[" << recvbuf.data() << "]";
        }
#endif

        // Finalize the MPI environment.
        MPI_Finalize();
    }

    inline int max_concurrency() const
    {
        return comm_size;
    }

    template<typename Tile_index> inline int rank(Tile_index id) const
    {
        // Surely not the most optimal repartition of tile ids across processing elements (lacks locality)
        return std::hash<Tile_index>{}(id) % comm_size;
    }

    template<typename Tile_index> inline bool is_local(Tile_index id) const
    {
        return comm_rank == rank(id);
    }

    template<typename OutputValue,
             typename Container,
             typename OutputIterator,
             typename Transform>
    OutputIterator ranges_transform(Container& c, Transform transform, OutputIterator out)
    {
        CGAL_DDT_TRACE0(*this, "PERF", "transform", "generic_work", "B");
        auto first = std::begin(c), end = std::end(c), last = first;
        while(first != end) {
            if (++last == end || first->first != last->first) {
                CGAL_DDT_TRACE2(*this, "PERF", "transform", 0, "B", k, to_string(first->first), in, to_summary(first, last));
                out = transform(first, last, out);
                CGAL_DDT_TRACE0(*this, "PERF", "transform", 0, "E");
                first = last;
            }
        }
        CGAL_DDT_TRACE0(*this, "PERF", "transform", "generic_work", "E");
        return out;
    }

    template<typename Container,
             typename V,
             typename Reduce,
             typename Transform>
    V ranges_reduce(Container& c, Transform transform, V value, Reduce reduce)
    {
        CGAL_DDT_TRACE0(*this, "PERF", "reduce", "generic_work", "B");
        auto first = std::begin(c), end = std::end(c), last = first;
        while(first != end) {
            if (++last == end || first->first != last->first) {
                CGAL_DDT_TRACE1(*this, "PERF", "transform", 0, "B", k, to_string(first->first));
                V val = transform(first, last);
                CGAL_DDT_TRACE2(*this, "PERF", "transform", 0, "E", value, val, in, to_summary(first, last));
                value = reduce(value, val);
                first = last;
            }
        }
        value = all_reduce(value, reduce);
        CGAL_DDT_TRACE1(*this, "PERF", "reduce", "generic_work", "E", value, value);
        return value;
    }


    template<typename OutputValue,
             typename Container,
             typename Transform,
             typename V,
             typename Reduce,
             typename OutputIterator>
    std::pair<V,OutputIterator>
    ranges_transform_reduce(Container& c, Transform transform, V value, Reduce reduce, OutputIterator out)
    {
        CGAL_DDT_TRACE0(*this, "PERF", "transform_reduce", "generic_work", "B");
        auto first = std::begin(c), end = std::end(c), last = first;
        std::vector<OutputValue> values;
        while(first != end) {
            if (++last == end || first->first != last->first) {
                std::vector<OutputValue> val;
                CGAL_DDT_TRACE2(*this, "PERF", "transform", 0, "B", k, to_string(first->first), in, to_summary(first, last));
                auto res = transform(first, last, std::back_inserter(val));
                CGAL_DDT_TRACE1(*this, "PERF", "transform", 0, "E", value, to_string(res.first));
                value = reduce(value, res.first);
                first = last;
            }
        }
        value = all_reduce(value, reduce);
        CGAL_DDT_TRACE1(*this, "PERF", "transform_reduce", "generic_work", "E", value, value);
        return { value, out };
    }

    template<typename InputIterator1,
             typename Container2,
             typename Transform,
             typename OutputIterator3,
             typename... Args2>
    OutputIterator3 transform_range(InputIterator1 first1, InputIterator1 last1, Container2& c2, Transform transform, OutputIterator3 out3, Args2&&... args2)
    {
        typedef typename Container2::key_type key_type;
        key_type k = first1->first;
        CGAL_assertion(is_local(k));
        auto it2 = c2.emplace(std::piecewise_construct,
            std::forward_as_tuple(k),
            std::forward_as_tuple(k, std::forward<Args2>(args2)...)).first;
        CGAL_DDT_TRACE2(*this, "PERF", "transform", 0, "B", k, to_string(k), in, to_summary(first1, last1));
        out3 = transform(first1, last1, it2->second, out3);
        CGAL_DDT_TRACE1(*this, "PERF", "transform", 0, "E", inout, to_summary(first1, last1));
        return out3;
    }

    template<typename OutputValue3,
             typename Container1,
             typename Container2,
             typename Transform,
             typename OutputIterator3,
             typename... Args2>
    OutputIterator3 ranges_transform(Container1& c1, Container2& c2, Transform transform, OutputIterator3 out3, Args2&&... args2)
    {
        std::vector<OutputValue3> v3;
        auto first1 = std::begin(c1), end1 = std::end(c1), last1 = first1;
        CGAL_DDT_TRACE1(*this, "PERF", "transform", "generic_work", "B", in, to_summary(first1, last1));
        while(first1 != end1) {
            if (++last1 == end1 || first1->first != last1->first) {
                if(is_local(first1->first))
                    transform_range(first1, last1, c2, transform, std::back_inserter(v3), std::forward<Args2>(args2)...);
                first1 = last1;
            }
        }
        CGAL_DDT_TRACE2(*this, "PERF", "transform", "generic_work", "E", inout, to_summary(first1, last1), out, to_summary(v3.begin(), v3.end()));
        CGAL_DDT_TRACE1(*this, "MPI", "all_to_all", "generic_work", "B", in, to_summary(v3.begin(), v3.end()));
        typedef std::remove_const_t<typename OutputValue3::first_type > first_type;
        typedef std::remove_const_t<typename OutputValue3::second_type> second_type;
        typedef std::pair<first_type,second_type> value_type;
        out3 = all_to_all<OutputValue3>(v3.begin(), v3.end(), out3);
        CGAL_DDT_TRACE0(*this, "MPI", "all_to_all", "generic_work", "E");
        return out3;
    }

    template<typename InputIterator1,
             typename Container2,
             typename Transform,
             typename OutputIterator3,
             typename... Args2>
    OutputIterator3 range_transform(InputIterator1 first1, InputIterator1 last1, Container2& c2, Transform transform, OutputIterator3 out3, Args2&&... args2)
    {
        typedef typename std::iterator_traits<InputIterator1>::value_type value_type1;
        typedef typename value_type1::first_type    key_type;

        key_type k = first1->first;
        int r1 = rank(k);
        if(r1 != comm_rank) {
            std::vector<value_type1> v1;
            std::move(first1, last1, std::back_inserter(v1));
            if (!v1.empty())
                issend(r1, v1);
            return out3;
        }

        typename Container2::iterator it2 = c2.emplace(std::piecewise_construct,
            std::forward_as_tuple(k),
            std::forward_as_tuple(k, std::forward<Args2>(args2)...)).first;

        CGAL_DDT_TRACE2(*this, "PERF", "transform", 0, "B", k, to_string(k), in, to_summary(first1, last1));
        out3 = transform(first1, last1, it2->second, out3);
        CGAL_DDT_TRACE1(*this, "PERF", "transform", 0, "E", inout, to_summary(first1, last1));
        return out3;
    }



    template<typename T>
    void issend(int dest, const T& value) {
        CGAL_DDT_TRACE1(*this, "PERF", "issend", 0, "B", dest, dest);
        std::ostringstream oss;
        write(oss, value);
        int sendcount = oss.str().size();
        MPI_Request request;
        int success1 = MPI_Issend(&sendcount, 1, MPI_INT, dest, size_tag, comm, &request);
        CGAL_assertion(success1 == MPI_SUCCESS);
        std::string& s = req_buf.emplace_back(std::move(oss.str()));
        int success2 = MPI_Issend(s.data(), sendcount, MPI_CHAR, dest, value_tag, comm, &request);
        CGAL_assertion(success2 == MPI_SUCCESS);
        req_value.push_back(request);
        ++req_send[dest];
        CGAL_DDT_TRACE2(*this, "PERF", "issend", 0, "E", bytes, sendcount, value, to_summary(value.begin(), value.end()));
    }

    template<typename OutputValue, typename OutputIterator>
    OutputIterator recv(int source, OutputIterator out) {
        CGAL_DDT_TRACE1(*this, "PERF", "recv", 0, "B", source, source);
        int recvcount;
        int success1 = MPI_Recv(&recvcount, 1, MPI_INT, source, size_tag, comm, MPI_STATUSES_IGNORE);
        CGAL_assertion(success1 == MPI_SUCCESS);
        std::vector<char> buf(recvcount+1, 0);
        int success2 = MPI_Recv(buf.data(), recvcount, MPI_CHAR, source, value_tag, comm, MPI_STATUSES_IGNORE);
        CGAL_assertion(success2 == MPI_SUCCESS);
        CGAL_DDT_TRACE1(*this, "PERF", "recv", 0, "E", bytes, recvcount);
        CGAL_DDT_TRACE1(*this, "MPI", "deserialize", "generic_work", "B", bytes, recvcount);
        out = deserialize<OutputValue>(buf.data(), out);
        CGAL_DDT_TRACE0(*this, "MPI", "deserialize", "generic_work", "E");
        return out;
    }


    template<typename OutputValue, typename OutputIterator>
    OutputIterator poll(int source, OutputIterator out)
    {
        int flag;
        MPI_Status status;
        int success = MPI_Iprobe(source, value_tag, comm, &flag, &status);
        CGAL_assertion(success == MPI_SUCCESS);
        while(flag)
        {
            out = recv<OutputValue>(status.MPI_SOURCE, out);
            ++req_recv;
            MPI_Iprobe(source, value_tag, comm, &flag, &status);
        }
        return out;
    }

    void test_some()
    {
        int outcount;
        std::vector<int> indices(req_value.size());
        int success = MPI_Testsome(req_value.size(), req_value.data(), &outcount, indices.data(), MPI_STATUSES_IGNORE);
        CGAL_assertion(success == MPI_SUCCESS);
        int pending = req_value.size();
        int i = outcount;
        while(i > 0) {
            --i;
            --pending;
            if(i==pending) continue;
            std::iter_swap(req_value.begin()+indices[i], req_value.begin()+pending);
            std::iter_swap(req_buf  .begin()+indices[i], req_buf  .begin()+pending);
        }
        req_value.erase(req_value.begin()+pending, req_value.end());
        req_buf.erase(req_buf.begin()+pending, req_buf.end());
        CGAL_DDT_TRACE1(*this, "PERF", "requests\", \"id\":\"" << comm_rank, 0, "C", pending, pending);
    }

#ifdef CGAL_DDT_MPI_FOR_EACH_DEFAULT
    template<typename Container1,
             typename Container2,
             typename Container3,
             typename Transform,
             typename... Args2>
    void ranges_for_each(Container1& c1, Container2& c2, Container3& c3, Transform transform, Args2&&... args2)
    {
        CGAL_DDT_TRACE0(*this, "PERF", "for_each", "generic_work", "B");
        typedef typename Container3::value_type  value_type3;
        ranges_transform<value_type3>(c1, c2, transform, std::inserter(c3, c3.end()), std::forward<Args2>(args2)...);
        while(!all_reduce(c3.empty(), std::logical_or<>())) {
            Container3 tmp;
            ranges_transform<value_type3>(c3, c2, transform, std::inserter(tmp, tmp.end()), std::forward<Args2>(args2)...);
            std::swap(tmp, c3);
        }
        CGAL_DDT_TRACE0(*this, "PERF", "for_each", "generic_work", "E");
    }
#else
    template<typename Container1,
             typename Container2,
             typename Container3,
             typename Transform,
             typename... Args2>
    void ranges_for_each(Container1& c1, Container2& c2, Container3& c3, Transform transform, Args2&&... args2)
    {
        CGAL_DDT_TRACE0(*this, "PERF", "for_each", "generic_work", "B");
        typedef typename Container1::iterator         iterator1;
        typedef typename Container1::key_type         key_type1;
        typedef typename Container3::key_type         key_type3;
        typedef typename Container3::iterator         iterator3;
        typedef std::insert_iterator<Container3>      inserter3;
        typedef typename Container3::value_type       value_type3;

        std::set<key_type1> keys1;
        for(iterator1 it1 = std::begin(c1); it1 != std::end(c1); ++it1)
            if(is_local(it1->first))
                keys1.insert(it1->first);

        inserter3 out3 = std::inserter(c3, c3.begin());

        req_send.assign(comm_size, 0);
        std::vector<int> req_send_(comm_size, 0);
        req_recv = 0;
        int req_recv_ = 0;
        MPI_Request req_recv_request = MPI_REQUEST_NULL;
        MPI_Request all_done_request = MPI_REQUEST_NULL;
        bool all_done = false, done = false;
        do {
            out3 = poll<value_type3>(MPI_ANY_SOURCE, out3);
            while(!(c3.empty() && keys1.empty())) {
                if (c3.empty()) {
                    typename std::set<key_type1>::iterator it = keys1.begin();
                    key_type1 k = *it;
                    std::pair<iterator1,iterator1> range1 = c1.equal_range(k);
                    keys1.erase(it);
                    out3 = range_transform(range1.first, range1.second, c2, transform, out3, std::forward<Args2>(args2)...);

                } else {
                    iterator3 it = c3.begin();
                    key_type3 k = it->first;
                    std::pair<iterator3,iterator3> range3 = c3.equal_range(k);

                    // move range3 from c3 to v3
                    std::vector<value_type3> v3;
                    std::move(range3.first, range3.second, std::back_inserter(v3));
                    c3.erase(range3.first, range3.second);
                    out3 = std::inserter(c3, c3.begin());

                    out3 = range_transform(v3.begin(), v3.end(), c2, transform, out3, std::forward<Args2>(args2)...);
                }
                test_some();
                out3 = poll<value_type3>(MPI_ANY_SOURCE, out3);

            } // while(!(c3.empty() && keys1.empty()))

            int flag, success;
            if (all_done_request == MPI_REQUEST_NULL) {
                if (req_recv_request == MPI_REQUEST_NULL) {
                    // prevent overflow : req_send stores the counts since the start of the last MPI_Ireduce_scatter_block
                    std::swap(req_send, req_send_);
                    req_send.assign(comm_size, 0);
                    CGAL_DDT_TRACE1(*this, "MPI", "MPI_Ireduce_scatter_block", "generic_work", "B", send_, to_summary(req_send_.begin(), req_send_.end()));
                    MPI_Ireduce_scatter_block(req_send_.data(), &req_recv_, 1, MPI_INT, MPI_SUM, comm, &req_recv_request);
                }

                success = MPI_Test(&req_recv_request, &flag, MPI_STATUSES_IGNORE);
                CGAL_assertion(success == MPI_SUCCESS);
                if(!flag) continue;
                CGAL_DDT_TRACE3(*this, "MPI", "MPI_Ireduce_scatter_block", "generic_work", "E",
                    send, to_summary(req_send.begin(), req_send.end()),
                    recv, req_recv,
                    recv_, req_recv_);

                req_recv -= req_recv_;
                done = req_recv == 0;
                for(int r = 0; done && r < comm_size; ++r) {
                    done = (req_send[r] == 0);
                }
                CGAL_DDT_TRACE1(*this, "MPI", "MPI_Iallreduce", "generic_work", "B", in, done);
                success = MPI_Iallreduce(&done, &all_done, 1, MPI_CXX_BOOL, MPI_LAND, comm, &all_done_request);
                CGAL_assertion(success == MPI_SUCCESS);
            }

            success = MPI_Test(&all_done_request, &flag, MPI_STATUSES_IGNORE);
            CGAL_assertion(success == MPI_SUCCESS);
            if(!flag) continue;
            CGAL_DDT_TRACE1(*this, "MPI", "MPI_Iallreduce", "generic_work", "E", out, all_done);

            if (all_done) break;
        } while (true);
        CGAL_DDT_TRACE0(*this, "PERF", "for_each", "generic_work", "E");
    }
#endif
    int thread_index() const { return comm_rank; }

private:
    MPI_Comm comm;
    int comm_size;
    int comm_rank;
    int pid;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    std::vector<MPI_Request> req_value;
    std::vector<std::string> req_buf;
    std::vector<int> req_send;
    int req_recv;
    static const int root_rank = 0;
    static const int size_tag = 1;
    static const int value_tag = 2;

#ifdef CGAL_DDT_TRACING
public:
    std::size_t clock_microsec() const { return 1e6*(MPI_Wtime() - trace.t0); }
    trace_logger<double> trace = {"", {}};
    int process_index() const { return pid; }
#endif

private:

    template<typename T>
    std::ostream& write(std::ostream& out, const T& t, const std::string& indent = "") {
        return out << indent << t;
    }

    template<typename T, typename A>
    std::ostream& write(std::ostream& out, const std::vector<T,A>& v, const std::string& indent = "") {
        return write(out, v.begin(), v.end(), indent);
    }

    template<typename T, typename U>
    std::ostream& write(std::ostream& out, const std::pair<T,U>& p, const std::string& indent = "") {
        return write(write(out, p.first, indent) << " ", p.second, indent);
    }

    template<typename Iterator>
    std::ostream& write(std::ostream& out, Iterator first, Iterator last, const std::string& indent = "") {
        out << indent << std::distance(first, last) << " ";
        for(Iterator it = first; it != last; ++it)
            write(out, *it, indent+" ") << " ";
        out << indent << " ";
        return out;
    }

    template<typename T>
    std::istream& read(std::istream& in, T& t) {
        return in >> t;
    }

    template<typename T, typename A>
    std::istream& read(std::istream& in, std::vector<T,A>& v) {
        int n;
        in >> n;
        v.reserve(n);
        for(int i=0; i<n; ++i) {
            T t;
            read(in, t);
            CGAL_assertion(!!in);
            v.push_back(std::move(t));
        }
        return in;
    }

    template<typename T, typename U>
    std::istream& read(std::istream& in, std::pair<T,U>& p) {
        return read(read(in, p.first), p.second);
    }

    template<typename OutputValue, typename OutputIterator>
    OutputIterator read_all(std::istream& in, OutputIterator out) {
       typedef typename std::remove_cv_t<typename OutputValue::first_type > first_type;
       typedef typename std::remove_cv_t<typename OutputValue::second_type> second_type;
       int n;
       in >> n;
       if(!in) return out;
       for(int i=0; i<n; ++i) {
           first_type v1;
           second_type v2;
           read(read(in, v1), v2);
           CGAL_assertion(!!in);
           *out++ = {std::move(v1), std::move(v2)};
       }
       return out;
    }

    template<typename OutputValue, typename OutputIterator>
    OutputIterator deserialize(char *buf, OutputIterator out) {
        std::istringstream iss(buf);
        while (iss)
            out = read_all<OutputValue>(iss, out);
        return out;
    }

    template<typename OutputValue, typename OutputIterator>
    OutputIterator deserialize(std::vector<char>& recvbuf, const std::vector<int>& rdispls, OutputIterator out) {
        CGAL_DDT_TRACE1(*this, "MPI", "deserialize", "generic_work", "B", bytes, recvbuf.size());
        for(int i = 0; i < comm_size; ++i) {
            if (rdispls[i+1] == rdispls[i]) continue;
            char c = recvbuf[rdispls[i+1]];
            recvbuf[rdispls[i+1]] = 0;
            out = deserialize<OutputValue>(recvbuf.data() + rdispls[i], out);
            recvbuf[rdispls[i+1]] = c;
        }
        CGAL_DDT_TRACE0(*this, "MPI", "deserialize", "generic_work", "E");
        return out;
    }

    void gather(const char *sendbuf, int sendcount, std::vector<char>& recvbuf, std::vector<int>& displs, int root) {
        CGAL_DDT_TRACE1(*this, "MPI", "MPI_Gather", "generic_work", "B", in, sendcount);
        std::vector<int> recvcounts(comm_size, 0);
        int success1 = MPI_Gather(&sendcount, 1, MPI_INT, recvcounts.data(), 1, MPI_INT, root, comm );
        CGAL_assertion(success1 == MPI_SUCCESS);

        displs.resize(comm_size+1, 0);
        for(int i=0; i<comm_size; ++i) displs[i+1] = displs[i] + recvcounts[i];
        recvbuf.resize(displs[comm_size]+1, 0); // null termination
        int success2 = MPI_Gatherv(sendbuf, sendcount, MPI_CHAR, recvbuf.data(), recvcounts.data(), displs.data(), MPI_CHAR, root, comm );
        CGAL_assertion(success2 == MPI_SUCCESS);
        CGAL_DDT_TRACE1(*this, "MPI", "MPI_Gather", "generic_work", "E", out, (comm_rank == root_rank ? displs[comm_size] : 0));
    }

    void all_gather(const char *sendbuf, int sendcount, std::vector<char>& recvbuf, std::vector<int>& displs) {
        CGAL_DDT_TRACE1(*this, "MPI", "MPI_Allgather", "generic_work", "B", in, sendcount);
        std::vector<int> recvcounts(comm_size, 0);
        int success1 = MPI_Allgather(&sendcount, 1, MPI_INT, recvcounts.data(), 1, MPI_INT, comm );
        CGAL_assertion(success1 == MPI_SUCCESS);

        displs.resize(comm_size+1, 0);
        for(int i=0; i<comm_size; ++i) displs[i+1] = displs[i] + recvcounts[i];
        recvbuf.resize(displs[comm_size]+1, 0); // null termination
        int success2 = MPI_Allgatherv(sendbuf, sendcount, MPI_CHAR, recvbuf.data(), recvcounts.data(), displs.data(), MPI_CHAR, comm );
        CGAL_assertion(success2 == MPI_SUCCESS);
        CGAL_DDT_TRACE1(*this, "MPI", "MPI_Allgather", "generic_work", "E", out, displs[comm_size]);
    }

    void all_to_all(const char *sendbuf, int *sendcounts, int *sdispls, std::vector<char>& recvbuf, std::vector<int>& rdispls) {
        CGAL_DDT_TRACE1(*this, "MPI", "MPI_Alltoall", "generic_work", "B", in, to_summary(sendcounts, sendcounts+comm_size));
        // communicate sendcounts to know recvcounts
        std::vector<int> recvcounts(comm_size, 0);
        int success1 = MPI_Alltoall(
            sendcounts, 1, MPI_INT,
            recvcounts.data(), 1, MPI_INT,
            comm);
        CGAL_assertion(success1 == MPI_SUCCESS);

        // compute rdispls
        rdispls.resize(comm_size+1, 0);
        for(int i=0; i<comm_size; ++i) rdispls[i+1] = rdispls[i] + recvcounts[i];

        // send sendbuf to recvbuf
        recvbuf.resize(rdispls[comm_size]+1, 0); // null termination
        int success2 = MPI_Alltoallv(
            sendbuf, sendcounts, sdispls, MPI_CHAR,
            recvbuf.data(), recvcounts.data(), rdispls.data(), MPI_CHAR,
            comm);
        CGAL_assertion(success2 == MPI_SUCCESS);
        CGAL_DDT_TRACE1(*this, "MPI", "MPI_Alltoall", "generic_work", "E", out, to_summary(recvcounts.begin(), recvcounts.end()));
    }

    template<typename V, typename Reduce>
    V all_reduce(V value, Reduce reduce) {
        CGAL_DDT_TRACE1(*this, "MPI", "all_reduce", "generic_work", "B", in, value);
        std::vector<char> recvbuf;
        std::vector<int> displs;
        {
            std::ostringstream oss;
            oss << value;
            all_gather(oss.str().c_str(), oss.str().size(), recvbuf, displs);
        }
        int sendcount = 0;
        {
            std::ostringstream oss;
            for(int i = 0; i < comm_size; ++i) {
                if (i == comm_rank) continue;
                std::string s(displs[i+1] - displs[i], 0);
                std::copy(recvbuf.data() + displs[i], recvbuf.data() + displs[i+1], s.data());
                std::istringstream iss(s);
                V v;
                iss >> v;
                value = reduce(value, v);
            }
        }
        CGAL_DDT_TRACE1(*this, "MPI", "all_reduce", "generic_work", "E", out, value);
        return value;
    }

    bool all_reduce(bool value, std::logical_or<> reduce) {
        CGAL_DDT_TRACE1(*this, "MPI", "MPI_Allreduce(BOOL,LOR)", "generic_work", "B", in, value);
        bool reduced;
        int success = MPI_Allreduce(&value, &reduced, 1, MPI_CXX_BOOL, MPI_LOR, comm );
        CGAL_assertion(success == MPI_SUCCESS);
        CGAL_DDT_TRACE1(*this, "MPI", "MPI_Allreduce(BOOL,LOR)", "generic_work", "E", out, reduced);
        return reduced;
    }

    template<typename OutputValue, typename InputIterator, typename OutputIterator>
    OutputIterator all_gather(InputIterator begin, InputIterator end, OutputIterator out) {
        CGAL_DDT_TRACE1(*this, "MPI", "serialize", "generic_work", "B", in, to_summary(begin, end));
        std::ostringstream oss;
        write(oss, begin, end);
        CGAL_DDT_TRACE1(*this, "MPI", "serialize", "generic_work", "E", bytes, oss.str().size());

        std::vector<char> recvbuf;
        std::vector<int > rdispls;
        all_gather(oss.str().c_str(), oss.str().size(), recvbuf, rdispls);

        return deserialize<OutputValue>(recvbuf, rdispls, out);
    }

    template<typename OutputValue, typename InputIterator, typename OutputIterator>
    OutputIterator all_to_all(InputIterator begin, InputIterator end, OutputIterator out) {
        CGAL_DDT_TRACE1(*this, "MPI", "serialize", "generic_work", "B", in, to_summary(begin, end));
        std::vector<std::ostringstream> oss(comm_size);
        auto first = begin, last = first;
        while(first != end) {
            if (++last == end || first->first != last->first) {
                if(is_local(first->first))
                    std::move(first, last, out);
                else {
                    write(oss[rank(first->first)], first, last);
                    // clear
                }
                first = last;
            }
        }
        std::vector<int> sendcounts(comm_size, 0);
        std::vector<int> sdispls   (comm_size+1, 0);
        for(int i = 0; i < comm_size; ++i) {
            int count = oss[i].str().size();
            sendcounts[i] = count;
            sdispls[i+1] = sdispls[i] + count;
        }
        std::vector<char> sendbuf(sdispls[comm_size]+1, 0);
        for(int i = 0; i < comm_size; ++i)
            std::copy_n(oss[i].str().c_str(), sendcounts[i], sendbuf.data() + sdispls[i]);
        CGAL_DDT_TRACE1(*this, "MPI", "serialize", "generic_work", "E", inout, to_summary(begin, end));

        std::vector<char> recvbuf;
        std::vector<int > rdispls;
        all_to_all(sendbuf.data(), sendcounts.data(), sdispls.data(), recvbuf, rdispls);

        return deserialize<OutputValue>(recvbuf, rdispls, out);
    }
};

}
}

#endif // CGAL_DDT_SCHEDULER_MPI_SCHEDULER_H
