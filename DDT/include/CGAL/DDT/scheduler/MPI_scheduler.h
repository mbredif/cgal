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

/// @todo make the point, count and Tile_index (de)serialization fully generic.
template<typename T> char * save_value_1(char * buf, T t) {
    *buf++ = (char)(t);
    return buf;
}

template<typename T> char * load_value_1(char * buf, T& t) {
    t = T(*buf++);
    return buf;
}


template<typename T> char * save_value_4(char * buf, T t) {
    *buf++ = (t      ) & 0xff;
    *buf++ = (t >>  8) & 0xff;
    *buf++ = (t >> 16) & 0xff;
    *buf++ = (t >> 24) & 0xff;
    return buf;
}

template<typename T> char * load_value_4(char * buf, T& t) {
    t = *buf++;
    t += (*buf++) <<  8;
    t += (*buf++) << 16;
    t += (*buf++) << 24;
    return buf;
}

/// Points are supposed to be represented exactly with the 2 doubles p[0] and p[1]
template<typename T> char * save_point(char * buf, T t) {
    double coords[2] = { t[0], t[1] };
    memcpy(buf, (char *)coords, 16);
    return buf + 16;
}

template<typename T> char * load_point(char * buf, T& t) {
    double coords[2];
    memcpy((char *)coords, buf, 16);
    t = T(coords[0], coords[1]);
    return buf + 16;
}

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

        pid = getpid();

#if CGAL_DDT_TRACING
        std::ostringstream oss;
        oss << "perf_mpi." << world_rank << ".json";
        trace.open(oss.str());
#endif

        // Print off a hello world message
        printf("Processor %s:%d [ %d / %d ]\n", processor_name, pid, world_rank+1, world_size);
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
        std::string s(oss.str());

        int sendcount = s.size()-1;
        std::vector<int> recvcounts(world_size, 0);
        MPI_Gather(&sendcount, 1, MPI_INT, recvcounts.data(), 1, MPI_INT, root_rank, MPI_COMM_WORLD );

        std::vector<int> displs(world_size+1, 0);
        for(int i=0; i<world_size; ++i) displs[i+1] = displs[i] + recvcounts[i];
        int recv_size = displs[world_size];
        std::vector<char> text(recv_size+2, 0);
        MPI_Gatherv(s.c_str()+1, sendcount, MPI_CHAR, text.data()+1, recvcounts.data(), displs.data(), MPI_CHAR, root_rank, MPI_COMM_WORLD );

        if (world_rank == root_rank) {
            std::ofstream out("perf_mpi.json");
            text[0] = '[';
            text[recv_size-1] = '\n';
            text[recv_size  ] = ']';
            out << text.data();
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
        // Surely not the most optimal repartition of tile ids across processing elements
        //assert(id>=0);
        return id % world_size;
    }

    template<typename Tile_index> inline bool is_local(Tile_index id) const
    {
        return rank(id) == world_rank;
    }

    template<typename OutputValue,
             typename Container,
             typename OutputIterator,
             typename Transform>
    OutputIterator ranges_transform(Container& c, Transform transform, OutputIterator out)
    {
        CGAL_DDT_TRACE0(*this, "PERF", "transform", "generic_work", "B");
        std::cerr << "todo : implement ranges_transform 1" << std::endl;
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
                if (is_local(first->first)) {
                    CGAL_DDT_TRACE2(*this, "PERF", "transform", 0, "B", k, to_string(first->first), in, to_summary(first, last));
                    V val = transform(first, last);
                    CGAL_DDT_TRACE1(*this, "PERF", "transform", 0, "E", value, val);
                    value = reduce(value, val);
                }
                first = last;
            }
        }
        std::cerr << "todo : aggregate the values of all processors" << std::endl;

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
        std::cerr << "todo : implement ranges_transform_reduce" << std::endl;
        CGAL_DDT_TRACE1(*this, "PERF", "transform_reduce", "generic_work", "E", value, value);
        return { value, out };
    }

    template<typename OutputValue3,
             typename Container1,
             typename Container2,
             typename Transform,
             typename OutputIterator3,
             typename... Args2>
    OutputIterator3 ranges_transform(Container1& c1, Container2& c2, Transform transform, OutputIterator3 out3, Args2&&... args2)
    {
        CGAL_DDT_TRACE0(*this, "PERF", "transform", "generic_work", "B");
        std::cerr << "todo : implement ranges_transform 2" << std::endl;
        CGAL_DDT_TRACE0(*this, "PERF", "transform", "generic_work", "E");
        return out3;
    }

    template<typename Container1,
             typename Container2,
             typename Transform,
             typename... Args2>
    void ranges_for_each(Container1& c1, Container2& c2, Transform transform, Args2&&... args2)
    {
        CGAL_DDT_TRACE0(*this, "PERF", "for_each", "generic_work", "B");
        /*
        // for now, let's assume that c2 is replicated on all processes, so that its non-local keys may be ignored
        // @todo : copartitioning of c1 and c2 ?
        for( auto it = c2.begin(); it != c2.end(); ) {
            if(!is_local(it->first)) it = c2.erase(it);
            else ++it;
        }

        for(iterator2 it2 = c2.begin(); it2 != c2.end(); ++it2) {
            key_type k = it2->first;
            if (!is_local(k)) continue;
            ...
        */
        std::cerr << "todo : implement ranges_for_each" << std::endl;
        CGAL_DDT_TRACE0(*this, "PERF", "for_each", "generic_work", "E");
    }

    template<typename Point_id>
    char *load_points(char *bytes, std::vector<Point_id>& points) const
    {
        int count;
        bytes = load_value_4(bytes, count);
        for(int i = 0; i< count; ++i) {
            Point_id p;
            bytes = load_point(bytes, p.second);
            bytes = load_value_4(bytes, p.first);
            points.push_back(p);
        }
        return bytes;
    }

    template<typename Point_id>
    char *save_points(char *bytes, const std::vector<Point_id>& points) const
    {
        int count = points.size();
        bytes = save_value_4(bytes, count);
        for(const auto& [id, p] : points) {
            bytes = save_point(bytes, p);
            bytes = save_value_4(bytes, id);
        }
        return bytes;
    }

    /// send/recv broadcast points to the tiles of other ranks/processes.
    /// send/recv points to the tiles that are local in other ranks/processes.
    /// @return whether any points were communicated.
    template<typename PointSetContainer>
    bool send_all_to_all(PointSetContainer& point_sets)
    {
        typedef typename PointSetContainer::key_type Tile_index;
        typedef typename PointSetContainer::Points Points;

        int id_size = 4; /// @todo serialized size of tile id
        int count_size = 4; /// @todo serialized size of point count
        int point_size = 16; /// @todo serialized size of a Point
        int data_size = point_size + id_size; /// @todo serialized size of a Point_id

        int broadcast_recv_size = 0;

        // one to all
        {
            int broadcast_send_size = 0;
            if (!point_sets.extreme_points().empty())
                broadcast_send_size = count_size + point_sets.extreme_points().size() * data_size;
            std::vector<char> sendbuf(broadcast_send_size, 0);
            if (broadcast_send_size>0) save_points(sendbuf.data(), point_sets.extreme_points());
            point_sets.extreme_points().clear();
            std::vector<int> recvcounts(world_size, 0);
            MPI_Allgather(
                &broadcast_send_size, 1, MPI_INT,
                recvcounts.data(), 1, MPI_INT,
                MPI_COMM_WORLD
            );

            std::vector<int> displs(world_size+1, 0);
            for(int i=0; i<world_size; ++i) displs[i+1] = displs[i] + recvcounts[i];
            broadcast_recv_size = displs[world_size];

            std::cout << processor_name <<":"<< pid << ":" << world_rank << " broadcast_send_size " << broadcast_send_size << std::endl;
            std::cout << processor_name <<":"<< pid << ":" << world_rank << " broadcast_recv_size " << broadcast_recv_size << std::endl;
            if(broadcast_recv_size > 0) // if any broadcast is occuring
            {
                std::vector<char> recvbuf(displs[world_size], 0);
                MPI_Allgatherv(
                    sendbuf.data(), broadcast_send_size, MPI_CHAR,
                    recvbuf.data(), recvcounts.data(), displs.data(), MPI_CHAR, MPI_COMM_WORLD);

                // load all points except the one sent by this process
                Points received_points;

                // ranks < world_rank
                char *bytes = recvbuf.data();
                char *end   = bytes + displs[world_rank];
                while( bytes < end ) bytes = load_points(bytes, received_points);

                // ranks > world_rank
                bytes = recvbuf.data() + displs[world_rank+1];
                end   = recvbuf.data() + displs[world_size];
                while( bytes < end ) bytes = load_points(bytes, received_points);

                for(auto& [id, point_set] : point_sets) {
                    if (!is_local(id) || received_points.empty()) continue;
                    Points& p = point_set.points()[id];
                    p.insert(p.end(), received_points.begin(), received_points.end());
                }
            }
        }

        int max_alltoall_send_size = 0;
        // one to one
        {
            std::vector<int> sendcounts(world_size, 0);
            for(const auto& [source, msg] : point_sets)
            {
                for(const auto& [target, points] : msg.points())
                {
                    int r = rank(target);
                    if(r == world_rank || points.empty()) continue; // local, no need to communicate !
                    sendcounts[r] += id_size + count_size + points.size() * data_size;
                }
            }

            // compute sdispls
            std::vector<int> sdispls(world_size+1, 0);
            std::vector<int> offset(world_size, 0);
            for(int i=0; i<world_size; ++i) {
                sdispls[i+1] = sdispls[i] + sendcounts[i];
                offset[i] = sdispls[i];
            }

            int alltoall_send_size = sdispls[world_size];
            std::cout << processor_name <<":"<< pid << ":" << world_rank << " alltoall_send_size " << alltoall_send_size << std::endl;

            int max_alltoall_send_size = 0;
            MPI_Allreduce(&alltoall_send_size, &max_alltoall_send_size,
                1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

            if (max_alltoall_send_size > 0)
            {

                // serialize to sendbuf and empty inbox
                std::vector<char> sendbuf(alltoall_send_size, 0);
                for(auto& [source, msg] : point_sets)
                {
                    for(auto& [target, points] : msg.points())
                    {
                        int r = rank(target);
                        if(r == world_rank || points.empty()) continue; // empty or local, no need to communicate !
                        char *bytes = sendbuf.data() + offset[r];
                        bytes = save_value_4(bytes, target);
                        bytes = save_points(bytes, points);
                        offset[r] = bytes - sendbuf.data();
                        points.clear();
                    }
                }

                // communicate sendcounts to know recvcounts
                std::vector<int> recvcounts(world_size, 0);
                MPI_Alltoall(
                    sendcounts.data(), 1, MPI_INT,
                    recvcounts.data(), 1, MPI_INT,
                    MPI_COMM_WORLD);

                // compute rdispls
                std::vector<int> rdispls(world_size+1, 0);
                for(int i=0; i<world_size; ++i) rdispls[i+1] = rdispls[i] + recvcounts[i];

                std::cout << processor_name <<":"<< pid << ":" << world_rank << " alltoall_recv_size " << rdispls[world_size] << std::endl;

                // send sendbuf to recvbuf
                std::vector<char> recvbuf(rdispls[world_size], 0);
                MPI_Alltoallv(
                    sendbuf.data(), sendcounts.data(), sdispls.data(), MPI_CHAR,
                    recvbuf.data(), recvcounts.data(), rdispls.data(), MPI_CHAR,
                    MPI_COMM_WORLD);

                // deserialize recvbuf to point_sets
                char *bytes = recvbuf.data();
                char *end = bytes + recvbuf.size();
                while( bytes < end) {
                    Tile_index target;
                    bytes = load_value_4(bytes, target);
                    Points& points = point_sets[target].points()[target];
                    std::size_t s = points.size();
                    bytes = load_points(bytes, points);
                }
            }
        }

        for(auto& [source, msg] : point_sets)
            for(auto& [target, points] : msg.points())
                std::cout << processor_name <<":"<< pid << ":" << world_rank << " points:"<< source << "->" << target << " = " << points.size() << std::endl;

        return (broadcast_recv_size > 0) || (max_alltoall_send_size > 0);
    }

private:
    int world_size;
    int world_rank, root_rank;
    int pid;
    char processor_name[MPI_MAX_PROCESSOR_NAME];

#ifdef CGAL_DDT_TRACING
public:
    typedef std::chrono::time_point<std::chrono::high_resolution_clock> clock_type;
    int process_index() const { return pid; }
    int thread_index() const { return world_rank; }
    std::size_t clock_microsec() const { return std::chrono::duration<double, std::micro>(clock_now() - trace.t0).count(); }
    clock_type clock_now() const { return std::chrono::high_resolution_clock::now(); }
    trace_logger<clock_type> trace = {"", clock_now()};
#endif
};

}
}

#endif // CGAL_DDT_SCHEDULER_MPI_SCHEDULER_H
