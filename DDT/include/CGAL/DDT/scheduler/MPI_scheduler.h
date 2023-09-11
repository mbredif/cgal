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
    MPI_scheduler(int max_concurrency = 0)
    {
        // Initialize the MPI environment
        MPI_Init(NULL, NULL);

        // Get the number of processes
        MPI_Comm_size(MPI_COMM_WORLD, &world_size);

        // Get the rank of the process
        MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
        root_rank = 0;

        // Get the name of the processor
        int name_len;
        MPI_Get_processor_name(processor_name, &name_len);

#if CGAL_DDT_TRACING
        std::vector<char> processor_names;
        std::vector<int> displs;
        all_gather(processor_name, name_len, processor_names, displs);
        std::set<std::string> processor_nameset;
        int core_id = 0;
        for(int i = 0; i < world_size; ++i) {
            processor_nameset.insert(std::string(processor_names.data()+displs[i], processor_names.data()+displs[i+1]));
            if (i < world_rank && strncmp(processor_name, processor_names.data()+displs[i], name_len)==0) {
                core_id++;
            }
        }
        pid = std::distance(processor_nameset.begin(), processor_nameset.find(processor_name));

        std::ostringstream oss;
        oss << "perf_mpi." << world_rank << ".json";
        trace.open(oss.str());

        // poor man's clock sync
        MPI_Barrier(MPI_COMM_WORLD);
        trace.t0 = clock_now();
        CGAL_DDT_TRACE1(*this, "", "thread_name", 0, "M", name, "\"" << processor_name << "[" << core_id << "]\"");
        CGAL_DDT_TRACE1(*this, "", "process_name", 0, "M", name, "\"" << processor_name << "\"");
#endif
    }

    ~MPI_scheduler() {
#if CGAL_DDT_TRACING
        trace.out.close();
        // collect all traces "perf_mpi.*.json" to "perf_mpi.json"
        std::ostringstream filename;
        filename << "perf_mpi." << world_rank << ".json";
        std::ifstream f(filename.str());
        std::ostringstream oss;
        oss << f.rdbuf();
        std::string sendbuf(oss.str());
        std::vector<char> recvbuf;
        std::vector<int> displs;
        gather(sendbuf.c_str() + 1, sendbuf.size() - 1, recvbuf, displs, root_rank);

        if (world_rank == root_rank) {
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
        return world_size;
    }

    template<typename Tile_index> inline int rank(Tile_index id) const
    {
        // Surely not the most optimal repartition of tile ids across processing elements (lacks locality)
        return std::hash<Tile_index>{}(id) % world_size;
    }

    template<typename Tile_index> inline bool is_local(Tile_index id) const
    {
        return world_rank == rank(id);
    }

    inline bool is_root() const
    {
        return world_rank == root_rank;
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
        typedef typename Container1::value_type value_type1;
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
        out3 = all_to_all<value_type>(v3.begin(), v3.end(), out3);
        CGAL_DDT_TRACE0(*this, "MPI", "all_to_all", "generic_work", "E");
        return out3;
    }

    template<typename Container1,
             typename Container2,
             typename Transform,
             typename... Args2>
    void ranges_for_each(Container1& c1, Container2& c2, Transform transform, Args2&&... args2)
    {
        CGAL_DDT_TRACE0(*this, "PERF", "for_each", "generic_work", "B");
        typedef typename Container1::key_type    key_type;
        typedef typename Container1::mapped_type mapped_type1;
        typedef typename Container1::value_type  value_type1;
        std::multimap<key_type, mapped_type1> m1[2];
        ranges_transform<value_type1>(c1, c2, transform, std::inserter(m1[0], m1[0].begin()), std::forward<Args2>(args2)...);
        for(int i = 0, j = 1; all_reduce(!m1[i].empty(), std::logical_or<>()); i = j, j = 1-i) {
            ranges_transform<value_type1>(m1[i], c2, transform, std::inserter(m1[j], m1[j].begin()), std::forward<Args2>(args2)...);
            m1[i].clear();
        }
        CGAL_DDT_TRACE0(*this, "PERF", "for_each", "generic_work", "E");
    }
    int thread_index() const { return world_rank; }

private:
    int world_size;
    int world_rank, root_rank;
    int pid;
    char processor_name[MPI_MAX_PROCESSOR_NAME];

#ifdef CGAL_DDT_TRACING
public:
    typedef double clock_type;
    std::size_t clock_microsec() const { return 1e6*(clock_now() - trace.t0); }
    clock_type clock_now() const { return MPI_Wtime(); }
    trace_logger<clock_type> trace = {"", clock_now()};
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
        out << indent << std::distance(first, last) << "\n";
        for(Iterator it = first; it != last; ++it)
            write(out, *it, indent+" ") << "\n";
        out << indent << "\n";
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
    OutputIterator read(std::istream& in, OutputIterator out) {
       int n;
       in >> n;
       if(!in) return out;
       for(int i=0; i<n; ++i) {
           OutputValue v;
           read(in, v);
           CGAL_assertion(!!in);
           *out++ = std::move(v);
       }
       return out;
    }

    template<typename OutputValue, typename OutputIterator>
    OutputIterator deserialize(std::vector<char>& recvbuf, const std::vector<int>& rdispls, OutputIterator out) {
        CGAL_DDT_TRACE0(*this, "MPI", "deserialize", "generic_work", "B");
        //CGAL_DDT_TRACE1(*this, "MPI", "deserialize", "generic_work", "B", in, CGAL::DDT::to_summary(recvbuf.data()));
        for(int i = 0; i < world_size; ++i) {
            if (rdispls[i+1] == rdispls[i]) continue;
            char c = recvbuf[rdispls[i+1]];
            recvbuf[rdispls[i+1]] = 0;
            std::string s(recvbuf.data() + rdispls[i], rdispls[i+1] - rdispls[i]);
            std::istringstream iss(s);
            while (iss)
                out = read<OutputValue>(iss, out);
            recvbuf[rdispls[i+1]] = c;
        }
        CGAL_DDT_TRACE0(*this, "MPI", "deserialize", "generic_work", "E");
        return out;
    }

    void gather(const char *sendbuf, int sendcount, std::vector<char>& recvbuf, std::vector<int>& displs, int root) {
        CGAL_DDT_TRACE1(*this, "MPI", "MPI_Gather", "generic_work", "B", in, sendcount);
        std::vector<int> recvcounts(world_size, 0);
        int success1 = MPI_Gather(&sendcount, 1, MPI_INT, recvcounts.data(), 1, MPI_INT, root, MPI_COMM_WORLD );
        CGAL_assertion(success1 == MPI_SUCCESS);

        displs.resize(world_size+1, 0);
        for(int i=0; i<world_size; ++i) displs[i+1] = displs[i] + recvcounts[i];
        recvbuf.resize(displs[world_size]+1, 0); // null termination
        int success2 = MPI_Gatherv(sendbuf, sendcount, MPI_CHAR, recvbuf.data(), recvcounts.data(), displs.data(), MPI_CHAR, root, MPI_COMM_WORLD );
        CGAL_assertion(success2 == MPI_SUCCESS);
        CGAL_DDT_TRACE1(*this, "MPI", "MPI_Gather", "generic_work", "E", out, (is_root() ? displs[world_size] : 0));
    }

    void all_gather(const char *sendbuf, int sendcount, std::vector<char>& recvbuf, std::vector<int>& displs) {
        CGAL_DDT_TRACE1(*this, "MPI", "MPI_Allgather", "generic_work", "B", in, sendcount);
        std::vector<int> recvcounts(world_size, 0);
        int success1 = MPI_Allgather(&sendcount, 1, MPI_INT, recvcounts.data(), 1, MPI_INT, MPI_COMM_WORLD );
        CGAL_assertion(success1 == MPI_SUCCESS);

        displs.resize(world_size+1, 0);
        for(int i=0; i<world_size; ++i) displs[i+1] = displs[i] + recvcounts[i];
        recvbuf.resize(displs[world_size]+1, 0); // null termination
        int success2 = MPI_Allgatherv(sendbuf, sendcount, MPI_CHAR, recvbuf.data(), recvcounts.data(), displs.data(), MPI_CHAR, MPI_COMM_WORLD );
        CGAL_assertion(success2 == MPI_SUCCESS);
        CGAL_DDT_TRACE1(*this, "MPI", "MPI_Allgather", "generic_work", "E", out, displs[world_size]);
    }

    void all_to_all(const char *sendbuf, int *sendcounts, int *sdispls, std::vector<char>& recvbuf, std::vector<int>& rdispls) {
        CGAL_DDT_TRACE1(*this, "MPI", "MPI_Alltoall", "generic_work", "B", in, to_summary(sendcounts, sendcounts+world_size));
        // communicate sendcounts to know recvcounts
        std::vector<int> recvcounts(world_size, 0);
        int success1 = MPI_Alltoall(
            sendcounts, 1, MPI_INT,
            recvcounts.data(), 1, MPI_INT,
            MPI_COMM_WORLD);
        CGAL_assertion(success1 == MPI_SUCCESS);

        // compute rdispls
        rdispls.resize(world_size+1, 0);
        for(int i=0; i<world_size; ++i) rdispls[i+1] = rdispls[i] + recvcounts[i];

        // send sendbuf to recvbuf
        recvbuf.resize(rdispls[world_size]+1, 0); // null termination
        int success2 = MPI_Alltoallv(
            sendbuf, sendcounts, sdispls, MPI_CHAR,
            recvbuf.data(), recvcounts.data(), rdispls.data(), MPI_CHAR,
            MPI_COMM_WORLD);
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
            for(int i = 0; i < world_size; ++i) {
                if (i == world_rank) continue;
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
        int success = MPI_Allreduce(&value, &reduced, 1, MPI_CXX_BOOL, MPI_LOR, MPI_COMM_WORLD );
        CGAL_assertion(success == MPI_SUCCESS);
        CGAL_DDT_TRACE1(*this, "MPI", "MPI_Allreduce(BOOL,LOR)", "generic_work", "E", out, reduced);
        return reduced;
    }

    template<typename OutputValue, typename InputIterator, typename OutputIterator>
    OutputIterator all_gather(InputIterator begin, InputIterator end, OutputIterator out) {
        CGAL_DDT_TRACE1(*this, "MPI", "serialize", "generic_work", "B", in, to_summary(begin, end));
        std::ostringstream oss;
        write(oss, begin, end);
        CGAL_DDT_TRACE0(*this, "MPI", "serialize", "generic_work", "E");
        // CGAL_DDT_TRACE1(*this, "MPI", "serialize", "generic_work", "E", out, CGAL::DDT::to_summary(oss.str().c_str()));

        std::vector<char> recvbuf;
        std::vector<int > rdispls;
        all_gather(oss.str().c_str(), oss.str().size(), recvbuf, rdispls);

        return deserialize<OutputValue>(recvbuf, rdispls, out);
    }

    template<typename OutputValue, typename InputIterator, typename OutputIterator>
    OutputIterator all_to_all(InputIterator begin, InputIterator end, OutputIterator out) {
        CGAL_DDT_TRACE1(*this, "MPI", "serialize", "generic_work", "B", in, to_summary(begin, end));
        std::vector<std::ostringstream> oss(world_size);
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
        std::vector<int> sendcounts(world_size, 0);
        std::vector<int> sdispls   (world_size+1, 0);
        for(int i = 0; i < world_size; ++i) {
            int count = oss[i].str().size();
            sendcounts[i] = count;
            sdispls[i+1] = sdispls[i] + count;
        }
        std::vector<char> sendbuf(sdispls[world_size]+1, 0);
        for(int i = 0; i < world_size; ++i)
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
