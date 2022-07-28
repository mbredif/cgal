#ifndef algo_ddt_algo_ddt_HPP
#define algo_ddt_algo_ddt_HPP

#include <CGAL/DDT/IO/logging.hpp>

#include <string>
#include <unordered_map>

namespace ddt
{

template<typename DDT, typename Scheduler>
class algo_ddt
{
public:

    typedef typename DDT::Traits    Traits;
    typedef typename DDT::Tile      Tile;



    algo_ddt(DDT & ddt_, Scheduler & sch_) : ddt(ddt_),sch(sch_)
    {
        log(0, sch.number_of_threads(), " thread(s)\n");
    }

    int for_each(const std::string& step, const std::string& type, const std::function<int(Tile&, bool)>& func)
    {
        log.step(step,type);
        return sch.for_each(ddt.tiles_begin(), ddt.tiles_end(), func);
    }
    int insert_received_points()
    {
        log.step("Splay", "Rcv   ");
        return sch.for_each(ddt.tiles_begin(), ddt.tiles_end(), sch.insert_func());
    }
    int send_all_bbox_points()
    {
        log.step("Send", "Loc+BB");
        return sch.for_each(ddt.tiles_begin(), ddt.tiles_end(), sch.send_all_func(&Tile::get_bbox_points, ddt.tile_ids_begin(), ddt.tile_ids_end()));
    }
    int splay_stars()
    {
        log.step("Splay", "Star  ");
        return sch.for_each_rec(ddt.tiles_begin(), ddt.tiles_end(), sch.splay_func(&Tile::get_neighbors));
    }

private:

  Scheduler sch;
    logging log;
    DDT ddt;

};

}

#endif // algo_ddt_DDT_HPP
