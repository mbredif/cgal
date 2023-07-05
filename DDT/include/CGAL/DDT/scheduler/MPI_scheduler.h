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
#include <CGAL/DDT/Tile.h>

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
    MPI_scheduler(int max_concurrency = 0) {
        // Initialize the MPI environment
        MPI_Init(NULL, NULL);

        // Get the number of processes
        MPI_Comm_size(MPI_COMM_WORLD, &world_size);

        // Get the rank of the process
        MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

        // Get the name of the processor
        int name_len;
        MPI_Get_processor_name(processor_name, &name_len);

        pid = getpid();

        // Print off a hello world message
        printf("Processor %s:%d [ %d / %d ]\n", processor_name, pid, world_rank+1, world_size);
    }

    ~MPI_scheduler() {
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

    template<typename Container,
             typename V,
             typename Transform,
             typename Reduce = std::plus<>>
    V transform_reduce(Container& c, V init, Transform transform, Reduce reduce = {})
    {
        V value = init;
        for(auto& [k, v] : c) {
            if (!is_local(k)) continue;
            v.locked = true;
            if (c.load(k, v)) value = reduce(value, transform(k, v));
            v.locked = true;
        }

        // @todo : aggregate the values of all processors
        return value;
    }

    template<typename Container1,
             typename Container2,
             typename V,
             typename Transform,
             typename Reduce = std::plus<>,
             typename... Args>
    V join_transform_reduce(Container1& c1, Container2& c2, V init, Transform transform, Reduce reduce = {}, Args... args)
    {
        // for now, let's assume that c2 is replicated on all processes, so that its non-local keys may be ignored
        // @todo : copartitioning of c1 and c2 ?
        for( auto it = c2.begin(); it != c2.end(); ) {
            if(!is_local(it->first)) it = c2.erase(it);
            else ++it;
        }
        V value = init;
        for(auto& [k, v2] : c2) {
            if (!is_local(k)) continue;
            typedef typename Container1::iterator iterator1;
            typedef typename Container1::mapped_type mapped_type1;
            iterator1 it = c1.try_emplace(k, k, std::forward<Args>(args)...).first;
            it->second.locked = true;

            mapped_type1& v1 = it->second;
            if (c1.load(k, it->second)) value = reduce(value, transform(k, v1, v2));

            c2.send_points(k);
            it->second.locked = false;
        }
        // @todo : aggregate the values of all processors
        return value;
    }

    template<typename Container1,
             typename Container2,
             typename V,
             typename Transform,
             typename Reduce = std::plus<>,
             typename... Args>
    V join_transform_reduce_loop(Container1& c1, Container2& c2, V init, Transform transform, Reduce reduce = {}, Args... args)
    {
        // for now, let's assume that c2 is replicated on all processes, so that its non-local keys may be ignored
        // @todo : copartitioning of c1 and c2 ?
        for( auto it = c2.begin(); it != c2.end(); ) {
            if(!is_local(it->first)) it = c2.erase(it);
            else ++it;
        }
        V value = init, v;
        do {
            do {
                v = join_transform_reduce(c1, c2, init, transform, reduce, args...);
                value = reduce(value, v);
            } while (v != init);
        } while (send_all_to_all(c2));
        return value;
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

<<<<<<< HEAD
        // communicate sendcounts to know recvcounts
        std::vector<int> recvcounts(world_size, 0);
        MPI_Alltoall(
            &sendcounts[0], 1, MPI_INT, 
            &recvcounts[0], 1, MPI_INT, 
            MPI_COMM_WORLD);

        // compute rdispls
        std::vector<int> rdispls(world_size+1, 0);
        for(int i=0; i<world_size; ++i) rdispls[i+1] = rdispls[i] + recvcounts[i];

        // send sendbuf to recvbuf
        std::vector<char> recvbuf(rdispls[world_size], 0);
        MPI_Alltoallv(
            &sendbuf[0], &sendcounts[0], &sdispls[0], MPI_CHAR,
            &recvbuf[0], &recvcounts[0], &rdispls[0], MPI_CHAR,
            MPI_COMM_WORLD);

        // deserialize recvbuf to inbox
        char *bytes = &recvbuf[0];
        char *end = bytes + recvbuf.size();
        while( bytes < end) bytes = load_points(bytes);

        // debug: write sent msg to file
        if (false) {
            std::string filename = "./";
            std::ostringstream oss;
            oss << "./send_" << world_rank << ".bin";
            std::ofstream out(oss.str().c_str(), std::ios::binary);
            out.write((const char*)(&sendbuf[0]), sendbuf.size());
            out.close();
        }

        // debug: write received msg to file
        if (false) {
            std::string filename = "./";
            std::ostringstream oss;
            oss << "./recv_" << world_rank << ".bin";
            std::ofstream out(oss.str().c_str(), std::ios::binary);
            out.write((const char*)(&recvbuf[0]), recvbuf.size());
            out.close();
        }
        for(const auto& it:inbox) {
            std::cout << int(it.first) << ":" << it.second.size() << " ";
        }
        std::cout << std::endl;

        return max_count;
    }

private:
    int world_size;
    int world_rank;
    int pid;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
};

}
}

#endif // CGAL_DDT_SCHEDULER_MPI_SCHEDULER_H
