#ifndef DDT_TILE_HPP
#define DDT_TILE_HPP

#include #include <CGAL/DDT/bbox.hpp>

#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <assert.h>


namespace ddt
{

template<class T>
class Tile
{
public:
    typedef T                                        Traits;
    typedef typename Traits::Id                      Id;
    typedef typename Traits::Flag                    Flag;
    typedef typename Traits::Point                   Point;
    typedef typename Traits::Delaunay_triangulation  DT;
    typedef typename Traits::Vertex_handle           Vertex_handle;
    typedef typename Traits::Vertex_iterator         Vertex_iterator;
    typedef typename Traits::Vertex_const_handle     Vertex_const_handle;
    typedef typename Traits::Vertex_const_iterator   Vertex_const_iterator;
    typedef std::pair<Vertex_const_handle,Id>        Vertex_const_handle_and_id;
    typedef std::tuple<Point,Id,Id>                  Point_id_source;
    typedef typename Traits::Cell_handle             Cell_handle;
    typedef typename Traits::Cell_const_handle       Cell_const_handle;
    typedef typename Traits::Cell_const_iterator     Cell_const_iterator;
    typedef typename Traits::Facet_const_iterator    Facet_const_iterator;
    typedef typename Traits::Facet_const_iterator    Facet_const_handle;
    typedef std::pair<Cell_const_handle,Id>          Cell_const_handle_and_id;
    enum { D = Traits::D };

    Tile(Id id, int dimension = D)
        : id_(id),
          dt_(traits.triangulation(dimension)),
          number_of_main_vertices_(0),
          number_of_main_facets_(0),
          number_of_main_cells_(0)
    {}

    inline DT& triangulation() { return dt_; }
    inline const DT& triangulation() const { return dt_; }

    inline Id id() const { return id_; }
    inline void set_id(Id i) { id_ = i; }

    inline int maximal_dimension() const { return traits.maximal_dimension(dt_); }
    inline int current_dimension() const { return traits.current_dimension(dt_); }

    inline Cell_const_iterator cells_begin() const { return traits.cells_begin(dt_); }
    inline Cell_const_iterator cells_end  () const { return traits.cells_end  (dt_); }

    inline Vertex_const_iterator vertices_begin() const { return traits.vertices_begin(dt_); }
    inline Vertex_const_iterator vertices_end  () const { return traits.vertices_end  (dt_); }

    inline Vertex_iterator vertices_begin() { return traits.vertices_begin(dt_); }
    inline Vertex_iterator vertices_end  () { return traits.vertices_end  (dt_); }

    inline Facet_const_iterator  facets_begin()  const { return traits.facets_begin(dt_); }
    inline Facet_const_iterator  facets_end  ()  const { return traits.facets_end  (dt_); }

    inline size_t number_of_vertices() const { return traits.number_of_vertices(dt_); }
    inline size_t number_of_cells   () const { return traits.number_of_cells   (dt_); }

    inline size_t number_of_main_vertices() const { return number_of_main_vertices_; }
    inline size_t number_of_main_facets  () const { return number_of_main_facets_;   }
    inline size_t number_of_main_cells   () const { return number_of_main_cells_;    }

    inline Id    id  (Vertex_const_handle v) const { assert(!vertex_is_infinite(v)); return traits.id  (v); }
    inline Flag& flag(Vertex_const_handle v) const { assert(!vertex_is_infinite(v)); return traits.flag(v); }

    inline void clear() { traits.clear(dt_); }
    template<class It> inline void insert(It begin, It end) { traits.insert(dt_, begin, end); }
    template<class It> inline void remove(It begin, It end) { traits.remove(dt_, begin, end); }

    inline Vertex_handle infinite_vertex() const { return traits.infinite_vertex(dt_); }
    inline const Point& point(Vertex_const_handle v) const { return traits.point(dt_, v); }
    inline double coord(const Point& p, int i) const { return traits.coord(dt_, p, i); }

    inline bool vertex_is_infinite(Vertex_const_handle v) const { return traits.vertex_is_infinite(dt_, v); }
    inline bool facet_is_infinite (Facet_const_handle  f) const { return traits.facet_is_infinite (dt_, f); }
    inline bool cell_is_infinite  (Cell_const_handle   c) const { return traits.cell_is_infinite  (dt_, c); }

    // Facet functions
    inline int index_of_covertex(Facet_const_handle f) const { return traits.index_of_covertex(dt_, f); }
    inline Cell_const_handle full_cell(Facet_const_handle f) const { return traits.full_cell(dt_, f); }

    // Cell functions
    inline Vertex_const_handle vertex(Cell_const_handle c, int i) const { return traits.vertex(dt_, c, i); }
    inline Facet_const_iterator facet(Cell_const_handle c, int i) const { return traits.facet(dt_, c, i); }
    inline int mirror_index(Cell_const_handle c, int i) const { return traits.mirror_index(dt_, c, i); }
    inline Cell_const_handle neighbor(Cell_const_handle c, int i) const { return traits.neighbor(dt_, c, i); }

    // read/write (TODO: move away?)
    void write_cgal(std::ostream & ofile) const { traits.write_cgal(ofile,dt_); }
    void read_cgal(std::istream & ifile) { traits.read_cgal(ifile,dt_); }

    // Local => all vertex ids are equal to the tile id
    // Mixed  => some vertex ids are equal to the tile id
    // Foreign => no vertex ids are equal to the tile id
    inline bool vertex_is_local(Vertex_const_handle v) const { assert(!vertex_is_infinite(v)); return id(v) == id(); }
    inline bool vertex_is_local_in_tile(Vertex_const_handle v, int tid) const { assert(!vertex_is_infinite(v)); return id(v) == tid; }
    inline bool vertex_is_foreign(Vertex_const_handle v) const { return !vertex_is_local(v); }

    template<typename F>
    bool facet_is_local(F f) const
    {
        int icv = index_of_covertex(f);
        auto c = full_cell(f);
        for(int i=0; i<=current_dimension(); ++i)
        {
            if (i == icv) continue;
            auto v = vertex(c,i);
            if ( vertex_is_infinite(v) ) continue;
            if ( vertex_is_foreign(v) ) return false;
        }
        return true;
    }

    template<typename F>
    bool facet_is_mixed(F f) const
    {
        int icv = index_of_covertex(f);
        auto c = full_cell(f);
        bool local_found = false;
        bool foreign_found = false;
        for(int i=0; i <= current_dimension(); ++i)
        {
            if ( i == icv ) continue;
            auto v = vertex(c,i);
            if ( vertex_is_infinite(v) ) continue;
            if ( vertex_is_local(v) )
            {
                if (foreign_found) return true;
                local_found = true;
            }
            else
            {
                if (local_found) return true;
                foreign_found = true;
            }
        }
        return false;
    }

    template<typename F>
    bool facet_is_foreign(F f) const
    {
        int icv = index_of_covertex(f);
        auto c = full_cell(f);
        for(int i=0; i<=current_dimension(); ++i)
        {
            if ( i == icv ) continue;
            auto v = vertex(c,i);
            if ( vertex_is_infinite(v) ) continue;
            if ( vertex_is_local(v) ) return false;
        }
        return true;
    }

    template<typename C>
    bool cell_is_local(C c) const
    {
        for(int i=0; i<=current_dimension(); ++i)
        {
            auto v = vertex(c,i);
            if ( vertex_is_infinite(v) ) continue;
            if ( vertex_is_foreign(v) ) return false;
        }
        return true;
    }

    template<typename C>
    bool cell_is_mixed(C c) const
    {
        bool local_found = false;
        bool foreign_found = false;
        for(int i=0; i <= current_dimension(); ++i)
        {
            auto v = vertex(c,i);
            if ( vertex_is_infinite(v) ) continue;
            if ( vertex_is_local(v) )
            {
                if (foreign_found) return true;
                local_found = true;
            }
            else
            {
                if (local_found) return true;
                foreign_found = true;
            }
        }
        return false;
    }

    template<typename C>
    bool cell_is_foreign(C c) const
    {
        for(int i=0; i<=current_dimension(); ++i)
        {
            auto v = vertex(c,i);
            if ( vertex_is_infinite(v) ) continue;
            if ( vertex_is_local(v) ) return false;
        }

        return true;
    }


    template<typename C>                                                                                                                                                                                               bool cell_is_foreign_in_tile(C c, int tid) const
    {
        for(int i=0; i<=current_dimension(); ++i)
        {
            auto v = vertex(c,i);
            if ( vertex_is_infinite(v) ) continue;
            if ( vertex_is_local_in_tile(v,tid) ) return false;
        }

        return true;
    }



    // Main
    template<typename V>
    bool vertex_is_main(V v) const
    {
        // TODO: define somehow the main infinite vertex
        return !vertex_is_infinite(v) && vertex_is_local(v) ;
    }

    template<typename F>
    bool facet_is_main(F f) const
    {
        return facet_is_main_c2(f);
    }

    template<typename F>
    bool facet_is_main_c2(F f) const
    {
        int icv = index_of_covertex(f);
        auto c1 = full_cell(f);
        auto c2 = neighbor(c1,icv);
        std::vector<int> lid;
        for(int i=0; i<=current_dimension(); ++i)
        {
            if (i == icv) continue;
            auto v = vertex(c1,i);
            if (vertex_is_infinite(v)) continue;
            Id vid = id(v);
            lid.push_back(vid);
        }
        std::sort(lid.begin(),lid.end());
        for(uint i=0; i < lid.size(); ++i)
        {
            int vid = lid[i];
            if(!cell_is_foreign_in_tile(c1,vid) && !cell_is_foreign_in_tile(c2,vid))
                return vid == id();
        }
        return false;
    }


    template<typename F>
    bool facet_is_main_c1(F f) const
    {
        int icv = index_of_covertex(f);
        auto c = full_cell(f);
        bool foreign = true;
        for(int i=0; i<=current_dimension(); ++i)
        {
            if (i == icv) continue;
            auto v = vertex(c,i);
            if (vertex_is_infinite(v)) continue;
            Id vid = id(v);
            if ( vid < id() )
                return false;
            else if (vid == id())
                foreign = false;
        }
        return !foreign;
    }

    template<typename C>
    bool cell_is_main(C c) const
    {
        bool foreign = true;
        for(int i=0; i<=current_dimension(); ++i)
        {
            auto v = vertex(c,i);
            if (vertex_is_infinite(v)) continue;
            Id vid = id(v);
            if ( vid < id() )
                return false;
            else if (vid == id())
                foreign = false;
        }
        return !foreign;
    }

    int simplify()
    {
        // initialize flags to 1
        for(auto vit = vertices_begin(); vit != vertices_end(); ++vit)
            if(!vertex_is_infinite(vit))
                flag(vit) = 1;

        // set flags of vertices incident to non-foreign cells to 0
        for(auto cit = cells_begin(); cit != cells_end(); ++cit)
        {
            if(cell_is_foreign(cit)) continue;
            for(int i=0; i<=current_dimension(); ++i)
            {
                Vertex_const_handle v = vertex(cit, i);
                if(!vertex_is_infinite(v))
                {
                    flag(v) = 0;
                }
            }
        }

        // gather vertices that are to be removed
        std::vector<Vertex_handle> todo;
        for(auto vit = vertices_begin(); vit != vertices_end(); ++vit)
            if(!vertex_is_infinite(vit) && flag(vit))
                todo.push_back(vit);
        // remove these vertices
        remove(todo.begin(), todo.end());
        return todo.size();
    }

    void get_bbox_points(std::vector<Vertex_const_handle>& out) const
    {
        Vertex_const_handle v[2*D];
        auto vit = vertices_begin();
        // first local point
        for(; vit != vertices_end(); ++vit)
        {
            if (!vertex_is_infinite(vit) && vertex_is_local(vit))
            {
                for(int i=0; i<2*D; ++i) v[i] = vit;
                break;
            }
        }
        if(vit == vertices_end()) return; // no local points
        // other local points
        for(; vit != vertices_end(); ++vit)
        {
            if (!vertex_is_infinite(vit) && vertex_is_local(vit))
            {
                const Point& p = point(vit);
                for(int i=0; i<D; ++i)
                {
                    if(p[i] < point(v[i  ])[i]) v[i  ] = vit;
                    if(p[i] > point(v[i+D])[i]) v[i+D] = vit;
                }
            }
        }
        // remove duplicates (O(D^2) implementation, should we bother ?)
        for(int i=0; i<2*D; ++i)
        {
            int j = 0;
            for(; j<i; ++j) if(v[j]==v[i]) break;
            if(i==j) out.push_back(v[i]);
        }
    }

    void get_local_neighbors(std::vector<Vertex_const_handle_and_id>& out) const
    {
        std::map<Id, std::set<Vertex_const_handle>> outbox;
        for(Cell_const_iterator cit = cells_begin(); cit != cells_end(); ++cit)
            for(int i=0; i<=current_dimension(); ++i)
            {
                Vertex_const_handle v = vertex(cit, i);
                if(vertex_is_infinite(v)) break;
                Id idv = id(v);
                if(idv != id())
                    for(int j=0; j<=current_dimension(); ++j)
                        if(vertex_is_local(vertex(cit, j))) // implies i!=j
                            outbox[idv].insert(vertex(cit, j));
            }
        for(auto&& pair : outbox)
            for(auto vh : pair.second)
                out.push_back(std::make_pair(vh, pair.first));
    }

    void get_neighbors(std::vector<Vertex_const_handle_and_id>& out) const
    {
        std::map<Id, std::set<Vertex_const_handle>> outbox;
        for(auto cit = cells_begin(); cit != cells_end(); ++cit)
            for(int i=0; i<=current_dimension(); ++i)
            {
                Vertex_const_handle v = vertex(cit, i);
                if(vertex_is_infinite(v)) break;
                Id idv = id(v);
                if(idv != id())
                    for(int j=0; j<=current_dimension(); ++j)
                    {
                        Vertex_const_handle w = vertex(cit, j);
                        if(vertex_is_infinite(w) || id(w) != idv) // implies i!=j
                            outbox[idv].insert(w);
                    }
            }
        for(auto&& pair : outbox)
            for(auto vh : pair.second)
                out.push_back(std::make_pair(vh, pair.first));
    }

    void get_neighbors_pid(std::map<Id, std::vector<Point_id_source>> & outbox)
    {

        for(auto cit = cells_begin(); cit != cells_end(); ++cit)
        {
            for(int i=0; i<=D; ++i)
            {
                Vertex_const_handle v = vertex(cit, i);
                if(vertex_is_infinite(v)) break;
                Id idv = id(v);
                if(idv != id())
                    for(int j=0; j<=D; ++j)
                    {
                        Vertex_const_handle w = vertex(cit, j);
                        if(vertex_is_infinite(w) || id(w) != idv)
                        {
                            outbox[idv].push_back(std::make_tuple(w->point(),id(w),id()));
                        }
                    }
            }
        }

    }

    int send_one(
        std::map<Id, std::vector<Point_id_source>>& inbox,
        const std::vector<Vertex_const_handle_and_id>& outbox
    )
    {
        int count = 0;
        for(auto& vi : outbox)
            count += send_vertex(inbox, vi.first, id(), vi.second);
        return count;
    }

    template<typename Id_iterator>
    int send_all(
        std::map<Id, std::vector<Point_id_source>>& inbox,
        const std::vector<Vertex_const_handle>& outbox,
        Id_iterator begin,
        Id_iterator end
    )
    {
        int count = 0;
        for(auto v : outbox)
            for(Id_iterator target = begin; target != end; ++target)
                count += send_vertex(inbox, v, id(), *target);
        return count;
    }

    inline bool send_vertex(
        std::map<Id, std::vector<Point_id_source>>& inbox, Vertex_const_handle vh, Id source, Id target)
    {
        if(vertex_is_infinite(vh))
            return false;
        Id vid = id(vh);
        if(target==vid || target == source || !sent_[target].insert(vh).second)
            return false;
        inbox[target].emplace_back(point(vh),vid,source);
        return true;
    }

    int insert(const std::vector<Point_id_source>& received, bool do_simplify = true)
    {
        if(received.empty()) return 0;
        std::vector<std::pair<Point,Id>> points;
        points.reserve(received.size());
        for(auto& v : received)
        {
            Id vid = std::get<1>(v);
            points.emplace_back(std::get<0>(v),vid); // source is unused here
            bbox_[vid] += std::get<0>(v);
        }
        insert(points.begin(), points.end());
        int s = 0;
        if(do_simplify)
            s = simplify();
        return points.size() - s;
    }

    void get_mixed_cells(std::vector<Cell_const_handle_and_id>& out) const
    {
        for(auto cit = cells_begin(); cit != cells_end(); ++cit)
            if(cell_is_mixed(cit))
                for(int i=0; i<=current_dimension(); ++i)
                    if(!vertex_is_infinite(vertex(cit,i)))
                        out.emplace_back(cit, id(vertex(cit,i)));
    }

    void get_adjacency_graph_edges(std::set<Id>& out_edges) const
    {
        for(auto cit = cells_begin(); cit != cells_end(); ++cit)
            if(cell_is_mixed(cit))
                for(int i=0; i<=current_dimension(); ++i)
                {
                    Vertex_const_handle v = vertex(cit,i);
                    if(!vertex_is_infinite(v) && id(v) != id())
                        out_edges.insert(id(v));
                }
    }

    void get_merge_graph_edges(std::set<Id>& out_edges, std::vector<Cell_const_handle>& finalized) const
    {
        for(auto cit = cells_begin(); cit != cells_end(); ++cit)
        {
            if(cell_is_foreign(cit)) continue;
            bool active = false;
            for(auto pair : bbox_)
                if((active || out_edges.find(pair.first)==out_edges.end()) && cell_is_active(pair.second, cit) )
                {
                    active = true;
                    out_edges.insert(pair.first);
                }
            if(!active)
                finalized.push_back(cit);
        }
    }


    bool cell_is_finalized(Cell_const_handle c) const
    {
        if(cell_is_foreign(c)) return false;
        // TODO: acceleration data structure !!!
        for(auto pair : bbox_)
            if(pair.first != id() && cell_is_active(pair.second, c)) return false;
        return true;
    }

    const std::map<Id, Bbox<D>>& const_bbox() const
    {
        return bbox_;
    }

    std::map<Id, Bbox<D>>& bbox()
    {
        return bbox_;
    }


    Vertex_const_handle locate_vertex(const Tile& t, Vertex_const_handle v) const
    {
        for(auto vit = vertices_begin(); vit != vertices_end(); ++vit )
        {
            if(traits.are_vertices_equal(t.dt_, v, dt_, vit))
                return vit;
        }
        assert(false);
        return vertices_end();
    }

    Facet_const_handle locate_facet(const Tile& t, Facet_const_handle f) const
    {
        assert(!t.cell_is_foreign(t.full_cell(f)));
        Cell_const_handle c = full_cell(f);
        Cell_const_handle d = locate_cell(t, c);
        Vertex_const_handle v = vertex(c, index_of_covertex(f));
        for(int i=0; i<=current_dimension(); ++i)
        {
            if(traits.are_vertices_equal(t.dt_, v, dt_, vertex(d, i)))
                return facet(d, i);
        }
        assert(false);
        return facets_end();
    }

    Cell_const_handle locate_cell(const Tile& t, Cell_const_handle c) const
    {
        assert(!t.cell_is_foreign(c));
        for(auto cit = cells_begin(); cit != cells_end(); ++cit )
        {
            if(traits.are_cells_equal(t.dt_, c, dt_, cit))
                return cit;
        }
        assert(false);
        return cells_end();
    }

    void finalize()
    {

        number_of_main_vertices_ = 0;
        for(auto vit = vertices_begin(); vit != vertices_end(); ++vit)
        {
            if(vertex_is_main(vit))
                ++number_of_main_vertices_;
        }
        number_of_main_facets_ = 0;
        for(auto fit = facets_begin(); fit != facets_end(); ++fit )
        {
            if(facet_is_main(fit))
                ++number_of_main_facets_;
        }

        number_of_main_cells_ = 0;
        for(auto cit = cells_begin(); cit != cells_end(); ++cit)
        {
            if(cell_is_main(cit))
                ++number_of_main_cells_;
        }
    }

    void read_class(std::istream& is)
    {
        is >> id_;
        is >> dt_;
    }

    bool is_valid() const
    {
        return dt_.is_valid();
    }



    std::map<Id, std::set<Point>> points_sent_;
private:
    Traits traits;
    Id id_;
    DT dt_;

    size_t number_of_main_vertices_;
    size_t number_of_main_facets_;
    size_t number_of_main_cells_;

    std::map<Id, Bbox<D>> bbox_;
    std::map<Id, std::set<Vertex_const_handle>> sent_;
};

template<class T>
std::istream& operator>> (std::istream& is,Tile<T> & tt)
{
    
  typedef typename Tile<T>::Id Id;
  Id ii;
  is >> ii;
  is >> tt;
  tt.set_id(ii);
  return is;
}

template<class T>
std::ostream& operator<< (std::ostream& os,const Tile<T> & tt)
{
    os << tt.id() << " ";
    os << tt.triangulation();
    return os;
}


}

#endif // DDT_TILE_HPP
