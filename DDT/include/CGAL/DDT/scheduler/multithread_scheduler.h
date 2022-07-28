#ifndef DDT_MULTITHREAD_SCHEDULER_HPP
#define DDT_MULTITHREAD_SCHEDULER_HPP

#include <map>
#include <vector>
#include <chrono>
#include <CGAL/DDT/thread_pool.hpp>

namespace ddt
{

template<typename T>
struct multithread_scheduler
{
    typedef T Tile;
    typedef typename Tile::Vertex_const_handle_and_id Vertex_const_handle_and_id;
    typedef typename Tile::Vertex_const_handle Vertex_const_handle;
    typedef typename Tile::Point_id_source Point_id_source;
    typedef typename Tile::Point Point;
    typedef typename Tile::Id Id;
    multithread_scheduler(int n_threads) : pool(n_threads), timeout_(1)
    {
        pool.init();
    }
    template<class Duration>
    multithread_scheduler(int n_threads, Duration timeout) : pool(n_threads), timeout_(timeout)
    {
        pool.init();
    }
    inline int number_of_threads() const
    {
        return pool.number_of_threads();
    }
    ~multithread_scheduler()
    {
        pool.shutdown();
    }

    void send(const Point& p, Id id, Id source, Id target)
    {
        inbox[target].emplace_back(p,id,source);
    }

    void send(const Point& p, Id id)
    {
        send(p,id,id,id);
    }

    std::function<int(Tile&, bool)>
    insert_func(bool do_simplify)
    {
        return [this, do_simplify](Tile& tile, bool /*unused*/ )
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
            std::vector<Vertex_const_handle_and_id> vertices;
            (tile.*f)(vertices);
            std::map<Id, std::vector<Point_id_source>> outgoing;
            int count = tile.send_one(outgoing, vertices);
            for(auto& p : outgoing) inbox[p.first].append(p.second);
            return count;
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
            std::map<Id, std::vector<Point_id_source>> outgoing;
            int count = tile.send_all(outgoing, vertices, begin, end);
            for(auto& p : outgoing) inbox[p.first].append(p.second);
            return count;
        };
    }

    template<typename Tile_iterator>
    int for_each(Tile_iterator begin, Tile_iterator end, const std::function<int(Tile&, bool)>& func, bool skip_tiles_receiving_no_points=false)
    {
        std::vector<std::future<int>> futures;
        for(Tile_iterator it = begin; it != end; ++it)
            futures.push_back(pool.submit(func, std::ref(*it), skip_tiles_receiving_no_points));
        int count = 0;
        for(auto& f: futures) count += f.get();
        return count;
    }
    /*
        // barrier between each epoch
        template<typename Tile_iterator>
        int for_each_rec(Tile_iterator begin, Tile_iterator end, const std::function<int(Tile&, bool)>& func)
        {
            int count = for_each(begin, end, func), c;
            while((c = for_each(begin, end, func, true)))
                count += c;
            return count;
        }
    */

    // no barrier between each epoch, busy tiles are skipped
    template<typename Tile_iterator>
    int for_each_rec(Tile_iterator begin, Tile_iterator end, const std::function<int(Tile&, bool)>& func)
    {
        if (begin == end) return 0;
        int count = 0, tilecount, checkcount;
        do
        {
            std::map<Id, std::future<int>> futures;
            for(Tile_iterator it = begin; it != end; ++it)
            {
                futures[it->id()] = pool.submit(func, std::ref(*it), false);
            }
            bool loop = true;
            Tile_iterator it = begin, itend = begin;
            while(loop || it != itend)
            {
                loop = false;
                if (futures[it->id()].wait_for(timeout_) != std::future_status::ready)
                {
                    itend = it;
                    if (++itend == end) itend = begin;
                    loop = true;
                }
                else
                {
                    tilecount = futures[it->id()].get();
                    if (tilecount)
                    {
                        count += tilecount;
                        itend = it;
                        if (++itend == end) itend = begin;
                        loop = true;
                    }
                    futures[it->id()] = pool.submit(func, std::ref(*it), true);
                }
                if (++it == end) it = begin;
            }
            checkcount = 0;
            for(Tile_iterator it = begin; it != end; ++it)
            {
                checkcount += futures[it->id()].get();
            }
            count += checkcount;
        }
        while (checkcount);
        return count;
    }
private:
    std::map<Id, safe<std::vector<Point_id_source>>> inbox;
    thread_pool pool;
    std::chrono::milliseconds timeout_;
};

}

#endif // DDT_MULTITHREAD_SCHEDULER_HPP
