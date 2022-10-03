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

namespace ddt
{


/// @todo make the point, count and Id (de)serialization fully generic.
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

template<typename T>
struct mpi_scheduler
{
    typedef T Tile;
    typedef typename Tile::Vertex_const_handle_and_id Vertex_const_handle_and_id;
    typedef typename Tile::Vertex_const_handle Vertex_const_handle;
    typedef typename Tile::Point_id_source Point_id_source;
    typedef typename Tile::Point Point;
    typedef typename Tile::Id Id;

    mpi_scheduler(int /*unused*/) {
        // Initialize the MPI environment
        MPI_Init(NULL, NULL);

        // Get the number of processes
        MPI_Comm_size(MPI_COMM_WORLD, &world_size);

        // Get the rank of the process
        MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

        // Get the name of the processor
        int name_len;
        MPI_Get_processor_name(processor_name, &name_len);

        // Print off a hello world message
        printf("Processor %s [ %d / %d ]\n", processor_name, world_rank, world_size);
    }

    ~mpi_scheduler() {
        // Finalize the MPI environment.
        MPI_Finalize();
    }

    inline int number_of_threads() const
    {
        return world_size;
    }

    void send(const Point& p, Id id, Id source, Id target)
    {
        inbox[target].emplace_back(p,id,source);
    }

    void send(const Point& p, Id id)
    {
        send(p,id,id,id);
    }

    inline int rank(Id id) const
    {
        // Surely not the most optimal repartition of tile ids across processing elements
        assert(id>=0);
        return id % world_size;
    }

    inline bool is_local(Id id) const
    {
        return rank(id) == world_rank;
    }

    std::function<int(Tile&, bool)>
    insert_func(bool do_simplify)
    {
        return [this, do_simplify](Tile& tile, bool /*unused*/)
        {
            std::vector<Point_id_source> received;
            inbox[tile.id()].swap(received);
            return int(tile.insert(received, do_simplify));
        };
    }

    template<typename F>
    std::function<int(Tile&, bool)>
    splay_func(F&& f, bool skip_tiles_receiving_no_points = false)
    {
        return [this,f](Tile& tile, bool skip_tiles_receiving_no_points)
        {
            std::vector<Point_id_source> received;
            inbox[tile.id()].swap(received);
            if(!tile.insert(received) && skip_tiles_receiving_no_points) return 0;
            std::vector<Vertex_const_handle_and_id> outgoing;
            (tile.*f)(outgoing);
            return tile.send_one(inbox, outgoing);
        };
    }

    template<typename Id_iterator, typename F>
    std::function<int(Tile&, bool)>
    send_all_func(Id_iterator begin, Id_iterator end, F&& f)
    {
        return [this,f,begin,end](Tile& tile, bool /*unused*/)
        {
            std::vector<Vertex_const_handle> vertices;
            (tile.*f)(vertices);
            return tile.send_all(inbox, vertices, begin, end);
        };
    }

    template<typename Tile_iterator>
    int for_each(Tile_iterator begin, Tile_iterator end, const std::function<int(Tile&, bool)>& func, bool skip_tiles_receiving_no_points=false)
    {
        send_all_to_all();
        int count = 0;
        for(Tile_iterator it = begin; it != end; ++it)
            count += func(*it, skip_tiles_receiving_no_points);
        return count;
    }
    // cycles indefinitely, and stops when the last N tiles reported a count of 0
    template<typename Tile_iterator>
    int for_each_rec(Tile_iterator begin, Tile_iterator end, const std::function<int(Tile&, bool)>& func)
    {
        int count = for_each(begin, end, func, false), c;
        do {
            Tile_iterator itend = end;
            for(Tile_iterator it = begin; it != itend; ++it)
            {
                if (it == end) it = begin;
                if((c = func(*it, true)))
                {
                    count += c;
                    itend = it;
                }
            }
        } while (send_all_to_all() > 0);
        return count;
    }

    char *load_points(char *bytes)
    {
        Id id;
        int count;
        bytes = load_value_1(bytes, id);
        bytes = load_value_4(bytes, count);
        std::vector<Point_id_source>& box = inbox[id];
        for(int i = 0; i< count; ++i) {
            Point_id_source p;
            bytes = load_point(bytes, std::get<0>(p));
            bytes = load_value_1(bytes, std::get<1>(p)); // Id point id
            bytes = load_value_1(bytes, std::get<2>(p)); // Id source
            box.push_back(p);
        }
        // std::cout << processor_name << "[" << world_rank << "] " << count  << " points recieved by " << int(id) << std::endl;
        return bytes;
    }

    char *save_points(char *bytes, Id id, const std::vector<Point_id_source>& msg) const 
    {
        int count = msg.size();
        bytes = save_value_1(bytes, id);
        bytes = save_value_4(bytes, count);
        for(auto p : msg) {
            bytes = save_point(bytes, std::get<0>(p));
            bytes = save_value_1(bytes, std::get<1>(p)); // Id point id
            bytes = save_value_1(bytes, std::get<2>(p)); // Id source
        }
        std::cout << processor_name << "[" << world_rank << "] " << count  << " points sent to " << int(id) << std::endl;
        return bytes;
    }

    /// send the content of each non-local inbox to the relevant processing element.
    /// @return maximum number of points sent from any one of the processing elements.
    int send_all_to_all() 
    {
        int id_size = 1; /// @todo serialized size of tile id
        int count_size = 4; /// @todo serialized size of point count
        int point_size = 16; /// @todo serialized size of a Point
        int data_size = point_size + 2*id_size; /// @todo serialized size of a Point_id_source

        // compute sendcounts
        std::vector<int> sendcounts(world_size, 0);
        int max_count = 0;
        for(auto it : inbox)
        {
            int r = rank(it.first);
            if(r == world_rank) continue; // local, no need to communicate !
            sendcounts[r] += id_size + count_size + it.second.size() * data_size;
            if (max_count < it.second.size()) max_count = it.second.size();
        }

        // compute sdispls
        std::vector<int> sdispls(world_size+1, 0);
        for(int i=0; i<world_size; ++i) sdispls[i+1] = sdispls[i] + sendcounts[i];

        MPI_Allreduce(&max_count, &max_count, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
        if(max_count == 0) return 0;


        // serialize inbox to sendbuf and empty inbox
        std::vector<char> sendbuf(sdispls[world_size], 0);
        std::vector<int> offset(world_size, 0);
        for(auto it : inbox)
        {
            Id id = it.first;
            int r = rank(id);
            if(r == world_rank) continue; // local, no need to communicate !
            char *bytes = save_points(&sendbuf[offset[r]], id, it.second);
            offset[r] = bytes - &sendbuf[0];
            it.second.clear();
        }

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

        return max_count;
    }

private:
    std::map<Id, std::vector<Point_id_source>> inbox;
    int world_size;
    int world_rank;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
};

}

#endif // CGAL_DDT_SCHEDULER_MPI_SCHEDULER_H
