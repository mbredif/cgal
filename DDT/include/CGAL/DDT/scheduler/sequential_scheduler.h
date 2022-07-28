#ifndef DDT_SEQUENTIAL_SCHEDULER_HPP
#define DDT_SEQUENTIAL_SCHEDULER_HPP

#include <map>
#include <vector>

namespace ddt
{

template<typename T>
struct sequential_scheduler
{
    typedef T Tile;
    typedef typename Tile::Vertex_const_handle_and_id Vertex_const_handle_and_id;
    typedef typename Tile::Vertex_const_handle Vertex_const_handle;
    typedef typename Tile::Point_id_source Point_id_source;
    typedef typename Tile::Point Point;
    typedef typename Tile::Id Id;

    sequential_scheduler(int /*unused*/) {}
    inline int number_of_threads() const
    {
        return 0;
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
        int count = 0;
        for(Tile_iterator it = begin; it != end; ++it)
            count += func(*it, skip_tiles_receiving_no_points);
        return count;
    }
    /*
        // stops only after processing entire all tiles an equal amount of iterations
        template<typename Tile_iterator>
        int for_each_rec(Tile_iterator begin, Tile_iterator end, const std::function<int(Tile&, bool)>& func)
        {
            int count = for_each(begin, end, func, false), c;
            while((c = for_each(begin, end, func, true)))
                count += c;
            return count;
        }
    */
    // cycles indefinitely, and stops when the last N tiles reported a count of 0
    template<typename Tile_iterator>
    int for_each_rec(Tile_iterator begin, Tile_iterator end, const std::function<int(Tile&, bool)>& func)
    {
        int count = for_each(begin, end, func, false), c;
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
        return count;
    }

    void send(const Point& p, Id id, Id source, Id target)
    {
        inbox[target].emplace_back(p,id,source);
    }

    void send(const Point& p, Id id)
    {
        send(p,id,id,id);
    }
private:
    std::map<Id, std::vector<Point_id_source>> inbox;
};

}

#endif // DDT_SEQUENTIAL_SCHEDULER_HPP
