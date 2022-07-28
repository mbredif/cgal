#ifndef DDT_DDT_HPP
#define DDT_DDT_HPP

#include <CGAL/DDT/iterator/Vertex_const_iterator.h>
#include <CGAL/DDT/iterator/Facet_const_iterator.h>
#include <CGAL/DDT/iterator/Cell_const_iterator.h>
#include <CGAL/DDT/tile.h>

#include <string>
#include <unordered_map>


namespace ddt
{

template<typename Map_const_iterator>
class Key_const_iterator : public Map_const_iterator
{
public:
    typedef typename Map_const_iterator::value_type::first_type value_type;
    typedef const value_type& reference;
    typedef const value_type* pointer;

    Key_const_iterator ( ) : Map_const_iterator ( ) { }
    Key_const_iterator ( Map_const_iterator it_ ) : Map_const_iterator ( it_ ) { }

    pointer operator -> ( ) const { return &(Map_const_iterator::operator->()->first); }
    reference operator * ( ) const { return Map_const_iterator::operator*().first; }
    Key_const_iterator operator++() { return Map_const_iterator::operator++(); }
    Key_const_iterator operator++(int) { return Map_const_iterator::operator++(0); }
};

template<typename Map_const_iterator>
class Mapped_const_iterator : public Map_const_iterator
{
public:
    typedef typename Map_const_iterator::value_type::second_type value_type;
    typedef const value_type& reference;
    typedef const value_type* pointer;

    Mapped_const_iterator ( ) : Map_const_iterator ( ) { }
    Mapped_const_iterator ( Map_const_iterator it_ ) : Map_const_iterator ( it_ ) { }

    pointer operator -> ( ) const { return &(Map_const_iterator::operator->()->second); }
    reference operator * ( ) const { return Map_const_iterator::operator*().second; }
    Mapped_const_iterator operator++() { return Map_const_iterator::operator++(); }
    Mapped_const_iterator operator++(int) { return Map_const_iterator::operator++(0); }
};

template<typename Map_iterator>
class Mapped_iterator : public Map_iterator
{
public:
    typedef typename Map_iterator::value_type::second_type value_type;
    typedef value_type& reference;
    typedef value_type* pointer;

    Mapped_iterator ( ) : Map_iterator ( ) { }
    Mapped_iterator ( Map_iterator it_ ) : Map_iterator ( it_ ) { }

    pointer operator -> ( ) { return &(Map_iterator::operator->()->second); }
    reference operator * ( ) { return Map_iterator::operator*().second; }
    Mapped_iterator operator++() { return Map_iterator::operator++(); }
    Mapped_iterator operator++(int) { return Map_iterator::operator++(0); }
};

template<typename _Traits, typename Scheduler, typename _Tile = ddt::Tile<_Traits>>
class DDT
{
public:
    typedef _Traits                                  Traits;
    typedef _Tile                                    Tile;

    typedef typename Traits::Point                   Point;
    typedef typename Traits::Id                      Id;
    typedef typename Traits::Delaunay_triangulation  DT;
    typedef typename Traits::Vertex_handle           Tile_vertex_handle;
    typedef typename Traits::Vertex_iterator         Tile_vertex_iterator;
    typedef typename Traits::Vertex_const_handle     Tile_vertex_const_handle;
    typedef typename Traits::Vertex_const_iterator   Tile_vertex_const_iterator;
    typedef typename Traits::Cell_handle             Tile_cell_handle;
    typedef typename Traits::Cell_const_handle       Tile_cell_const_handle;
    typedef typename Traits::Cell_const_iterator     Tile_cell_const_iterator;
    typedef typename Traits::Facet_handle            Tile_facet_handle;
    typedef typename Traits::Facet_const_handle      Tile_facet_const_handle;
    typedef typename Traits::Facet_const_iterator    Tile_facet_const_iterator;

    typedef std::pair<Tile_cell_const_handle,Id>     Tile_cell_const_handle_and_id;
    typedef std::pair<Tile_vertex_const_handle,Id>   Tile_vertex_const_handle_and_id;
    typedef std::tuple<Point,Id,Id>                  Point_id_source;

    typedef ddt::Vertex_const_iterator<DDT>          Vertex_const_iterator;
    typedef ddt::Facet_const_iterator <DDT>          Facet_const_iterator;
    typedef ddt::Cell_const_iterator  <DDT>          Cell_const_iterator;

    typedef std::map<Id, Tile>                                              Tile_container;
    typedef Mapped_const_iterator<typename Tile_container::const_iterator>  Tile_const_iterator ;
    typedef Mapped_iterator<typename Tile_container::iterator>              Tile_iterator ;
    typedef Key_const_iterator<typename Tile_container::const_iterator>     Tile_id_const_iterator ;

    enum { D = Traits::D };

    inline int maximal_dimension() const
    {
        return D;
    }

    DDT(int n_threads=0) :
        tiles(),
        sch(n_threads),
        number_of_vertices_(0),
        number_of_facets_  (0),
        number_of_cells_   (0)
    {
    }

    DDT(const DDT& ddt) :
        tiles(ddt.tiles),
        sch(ddt.sch.number_of_threads()),
        number_of_vertices_(ddt.number_of_vertices_),
        number_of_facets_  (ddt.number_of_facets_  ),
        number_of_cells_   (ddt.number_of_cells_   )
    {
    }

    inline size_t number_of_cells   () const { return number_of_cells_;    }
    inline size_t number_of_vertices() const { return number_of_vertices_; }
    inline size_t number_of_facets  () const { return number_of_facets_;   }
    inline size_t number_of_tiles   () const { return tiles.size();   }

    Vertex_const_iterator vertices_begin() const { return Vertex_const_iterator(tiles_begin(), tiles_end()); }
    Vertex_const_iterator vertices_end  () const { return Vertex_const_iterator(tiles_begin(), tiles_end(), tiles_end()); }

    Cell_const_iterator cells_begin() const { return Cell_const_iterator(tiles_begin(), tiles_end()); }
    Cell_const_iterator cells_end  () const { return Cell_const_iterator(tiles_begin(), tiles_end(), tiles_end()); }

    Facet_const_iterator facets_begin() const { return Facet_const_iterator(tiles_begin(), tiles_end()); }
    Facet_const_iterator facets_end  () const { return Facet_const_iterator(tiles_begin(), tiles_end(), tiles_end()); }

    Tile_id_const_iterator tile_ids_begin() const { return tiles.begin(); }
    Tile_id_const_iterator tile_ids_end  () const { return tiles.end  (); }

    Tile_const_iterator tiles_begin  () const { return tiles.begin (); }
    Tile_const_iterator tiles_end    () const { return tiles.end   (); }
    Tile_const_iterator get_tile(Id id) const { return tiles.find(id); }

    Tile_iterator tiles_begin  () { return tiles.begin (); }
    Tile_iterator tiles_end    () { return tiles.end   (); }
    Tile_iterator get_tile(Id id) { return tiles.find(id); }

    int vertex_id(Vertex_const_iterator v) const
    {
        if (v->is_infinite()) return -1;
        return std::distance(vertices_begin(), v->main());
    }

    int cell_id(Cell_const_iterator c) const
    {
        return std::distance(cells_begin(), c->main());
    }

    int insert_received_points(bool do_simplify=true) { return sch.for_each(tiles_begin(), tiles_end(), sch.insert_func(do_simplify)); }
    int send_all_bbox_points()       { return sch.for_each(tiles_begin(), tiles_end(), sch.send_all_func(tile_ids_begin(), tile_ids_end(), &Tile::get_bbox_points)); }
    int splay_stars()       { return sch.for_each_rec(tiles_begin(), tiles_end(), sch.splay_func(&Tile::get_neighbors)); }

    void init(int id)
    {
        tiles.emplace(id, id);
    }

    void clear(int NT)
    {
        tiles.clear();
        for(int id=0; id<NT; ++id)
        {
            init(id);
        }
    }

    template<typename Iterator, typename Partitioner>
    void send_points(Iterator it, int count, Partitioner& part)
    {
        for(; count; --count, ++it)
        {
            Point p(*it);
            int id = part(p);
            if (tiles.find(id) == tiles.end())
                init(id);
            sch.send(p,id);
        }
    }

    void get_adjacency_graph(std::unordered_multimap<Id,Id>& edges) const
    {
        for(auto tile = tiles_begin(); tile != tiles_end(); ++tile)
        {
            std::set<Id> out_edges;
            tile->get_adjacency_graph_edges(out_edges);
            Id source = tile->id();
            for(Id target : out_edges)
                edges.emplace(source, target);
        }
    }

    bool is_adjacency_graph_symmetric() const
    {
        std::unordered_multimap<Id,Id> edges;
        std::unordered_multimap<Id,Id> reversed;
        get_adjacency_graph(edges);
        for(auto& edge : edges)
            reversed.emplace(edge.second, edge.first);
        return edges == reversed;
    }

    void get_ring(Cell_const_iterator c, int deg, std::set<Cell_const_iterator>& cset) const
    {
        std::set<Cell_const_iterator> seeds;
        c = c->main();
        cset.insert(c);
        seeds.insert(c);
        for(int i=0; i<deg; ++i)
        {
            std::set<Cell_const_iterator> next;
            next_ring(seeds, next);
            cset.insert(next.begin(), next.end());
            seeds.swap(next);
        }
    }

    void next_ring(const std::set<Cell_const_iterator>& seeds, std::set<Cell_const_iterator>& next) const
    {
        for(auto seed : seeds)
        {
            for(int d = 0; d <= D; d++)
            {
                auto c = seed->neighbor(d)->main();
                if(seeds.find(c) == seeds.end())
                    next.insert(c);
            }
        }
    }

    void finalize()
    {
        number_of_vertices_ = 0;
        number_of_facets_ = 0;
        number_of_cells_ = 0;
        for(auto tile = tiles_begin(); tile != tiles_end(); ++tile)
        {
            tile->finalize();
            number_of_vertices_ += tile->number_of_main_vertices();
            number_of_facets_ += tile->number_of_main_facets();
            number_of_cells_ += tile->number_of_main_cells();
        }
    }


    bool is_valid() const
    {
        size_t number_of_vertices = 0;
        size_t number_of_facets = 0;
        size_t number_of_cells = 0;
        for(auto tile = tiles_begin(); tile != tiles_end(); ++tile)
        {
            if(!tile->is_valid())
            {
                std::cerr << "Tile " << int(tile->id()) << " is invalid" << std::endl;
                return false;
            }
            number_of_vertices += tile->number_of_main_vertices();
            number_of_facets += tile->number_of_main_facets();
            number_of_cells += tile->number_of_main_cells();

            for(auto v = tile->vertices_begin(); v != tile->vertices_end(); ++v)
            {
                assert(tile->vertex_is_infinite(v) || (tile->vertex_is_local(v) + tile->vertex_is_foreign(v) == 1));
                if(tile->vertex_is_infinite(v)) continue;
                Id tid = tile->id(v);
                if(tid == tile->id()) continue;
                auto t = get_tile(tid);
                if(t->locate_vertex(*tile, v) == t->vertices_end())
                {
                    std::cerr << "locate_vertex failed" << std::endl;
                    return false;
                }
            }
            for(auto f = tile->facets_begin(); f != tile->facets_end(); ++f)
            {
                assert(tile->facet_is_local(f) + tile->facet_is_mixed(f) + tile->facet_is_foreign(f) == 1);
                if(!tile->facet_is_mixed(f)) continue;
                std::set<Id> tids;
                for(int d = 0; d <= tile->current_dimension(); ++d)
                {
                    if(d==tile->index_of_covertex(f)) continue;
                    auto c = tile->full_cell(f);
                    auto v = tile->vertex(c, d);
                    if(tile->vertex_is_infinite(v)) continue;
                    Id tid = tile->id(v);
                    if(tid == tile->id()) continue;
                    tids.insert(tid);
                }
                for(auto tid : tids)
                {
                    auto t = get_tile(tid);
                    if(t->locate_facet(*tile, f) == t->facets_end())
                    {
                        std::cerr << "locate_facet failed" << std::endl;
                        return false;
                    }
                }

            }
            for(auto c = tile->cells_begin(); c != tile->cells_end(); ++c)
            {
                assert(tile->cell_is_local(c) + tile->cell_is_mixed(c) + tile->cell_is_foreign(c) == 1);
                if(!tile->cell_is_mixed(c)) continue;
                std::set<Id> tids;
                for(int d = 0; d <= tile->current_dimension(); ++d)
                {
                    auto v = tile->vertex(c, d);
                    if(tile->vertex_is_infinite(v)) continue;
                    Id tid = tile->id(v);
                    if(tid == tile->id()) continue;
                    tids.insert(tid);
                }
                for(auto tid : tids)
                {
                    auto t = get_tile(tid);
                    if(t->locate_cell(*tile, c) == t->cells_end())
                    {
                        std::cerr << "locate_cell failed" << std::endl;
                        return false;
                    }
                }
            }
        }
        if (number_of_vertices != number_of_vertices_) { std::cerr << "incorrect number_of_vertices" << std::endl; return false; }
        if (number_of_facets != number_of_facets_) { std::cerr << "incorrect number_of_facets" << std::endl; return false; }
        if (number_of_cells != number_of_cells_) { std::cerr << "incorrect number_of_cells" << std::endl; return false; }
        return true;
    }


private:

    Tile_container tiles;
    Scheduler sch;
    size_t number_of_vertices_;
    size_t number_of_facets_;
    size_t number_of_cells_;

};

}

#endif // DDT_DDT_HPP
