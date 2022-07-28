#ifndef DDT_CGAL_TRAITS_D_HPP
#define DDT_CGAL_TRAITS_D_HPP


#include <CGAL/DDT/data.h>
#include <CGAL/DDT/bbox.h>
#include <CGAL/DDT/iterator/Facet_const_iterator_d.h>

#include <CGAL/Epick_d.h>
#include <CGAL/Delaunay_triangulation.h>
#include <CGAL/Triangulation_ds_vertex.h>
#include <CGAL/point_generators_d.h>


#include <functional>
#include <limits>


typedef std::numeric_limits< double > dbl;

namespace ddt
{

template<typename I, typename F, typename Dim = CGAL::Dynamic_dimension_tag>
struct Cgal_traits_d
{
    typedef Dim                                                    Dim_tag;
    typedef I                                                      Id;
    typedef F                                                      Flag;
    typedef ddt::Data<Id, Flag>                                    Data;
    typedef CGAL::Epick_d<Dim_tag>                                 K;
    typedef CGAL::Triangulation_vertex<K,Data>                     Vb;
    typedef CGAL::Triangulation_full_cell<K>                       Cb;
    typedef CGAL::Triangulation_data_structure<Dim_tag,Vb,Cb>      TDS;
    typedef typename K::Point_d                                    Point;

    typedef typename TDS::Vertex_const_iterator                    Vertex_const_iterator;
    typedef typename TDS::Vertex_const_handle                      Vertex_const_handle;
    typedef typename TDS::Vertex_iterator                          Vertex_iterator;
    typedef typename TDS::Vertex_handle                            Vertex_handle;

    typedef typename TDS::Full_cell_const_iterator                 Cell_const_iterator;
    typedef typename TDS::Full_cell_const_handle                   Cell_const_handle;
    typedef typename TDS::Full_cell_iterator                       Cell_iterator;
    typedef typename TDS::Full_cell_handle                         Cell_handle;

    typedef std::pair<Cell_const_handle, int>                      Facet;
    typedef Facet_const_iterator_d<TDS>                            Facet_const_iterator;
    typedef Facet_const_iterator                                   Facet_const_handle;
    typedef Facet_const_iterator                                   Facet_iterator;
    typedef Facet_const_iterator                                   Facet_handle;

    typedef CGAL::Delaunay_triangulation<K,TDS>                    Delaunay_triangulation;
    typedef CGAL::Random_points_in_cube_d<Point>                   Random_points_in_box;

    Delaunay_triangulation triangulation(int dimension) const
    {
        return Delaunay_triangulation(dimension);
    }

    inline Id    id  (Vertex_const_handle v) const
    {
        return v->data().id;
    }
    inline Flag& flag(Vertex_const_handle v) const
    {
        return v->data().flag;
    }
    inline int current_dimension(const Delaunay_triangulation& dt) const
    {
        return dt.current_dimension();
    }
    inline int maximal_dimension(const Delaunay_triangulation& dt) const
    {
        return dt.maximal_dimension();
    }
    inline size_t number_of_cells(const Delaunay_triangulation& dt) const
    {
        return dt.number_of_full_cells();
    }
    inline size_t number_of_vertices(const Delaunay_triangulation& dt) const
    {
        return dt.number_of_vertices();
    }
    inline Vertex_const_handle vertex(const Delaunay_triangulation& dt, Cell_const_handle c, int i) const
    {
        return c->vertex(i);
    }
    inline Vertex_const_iterator vertices_begin(const Delaunay_triangulation& dt) const
    {
        return dt.vertices_begin();
    }
    inline Vertex_const_iterator vertices_end(const Delaunay_triangulation& dt) const
    {
        return dt.vertices_end();
    }
    inline Vertex_iterator vertices_begin(Delaunay_triangulation& dt) const
    {
        return dt.vertices_begin();
    }
    inline Vertex_iterator vertices_end(Delaunay_triangulation& dt) const
    {
        return dt.vertices_end();
    }
    inline Facet_const_iterator facets_begin(const Delaunay_triangulation& dt) const
    {
        return Facet_const_iterator(dt.tds());
    }
    inline Facet_const_iterator facets_end(const Delaunay_triangulation& dt) const
    {
        return Facet_const_iterator();
    }
    inline Cell_const_iterator cells_begin(const Delaunay_triangulation& dt) const
    {
        return dt.full_cells_begin();
    }
    inline Cell_const_iterator cells_end(const Delaunay_triangulation& dt) const
    {
        return dt.full_cells_end();
    }

    inline Vertex_handle infinite_vertex(const Delaunay_triangulation& dt) const
    {
        return dt.infinite_vertex();
    }

    inline void clear(Delaunay_triangulation& dt) const
    {
        return dt.clear();
    }

    template<class It> inline void insert(Delaunay_triangulation& dt, It begin, It end) const
    {
        Vertex_handle hint;
        for(auto it=begin; it!=end; ++it)
        {
            if (hint != Vertex_handle())
            {
                hint = dt.insert(it->first, hint);
            }
            else
            {
                hint = dt.insert(it->first);
            }
            hint->data() = it->second;
        }
    }

    template<class It> inline void remove(Delaunay_triangulation& dt, It begin, It end) const
    {
        dt.remove(begin, end);
    }

    inline Point circumcenter(const Delaunay_triangulation& dt, Cell_const_handle c) const
    {
        // TODO
        return Point();
    }

    inline bool vertex_is_infinite(const Delaunay_triangulation& dt, Vertex_const_handle v) const
    {
        return dt.is_infinite(v);
    }

    inline bool facet_is_infinite(const Delaunay_triangulation& dt, Facet_const_handle f) const
    {
        for(int i = 0; i<=dt.current_dimension(); ++i)
            if(i!=f->second && dt.is_infinite(vertex(dt, f->first, i)))
                return true;
        return false;
    }

    inline bool cell_is_infinite(const Delaunay_triangulation& dt, Cell_const_handle c) const
    {
        for(int i = 0; i<=dt.current_dimension(); ++i)
            if(dt.is_infinite(c->vertex(i)))
                return true;
        return false;
    }

    inline const Point& point(const Delaunay_triangulation& dt, Vertex_const_handle v) const
    {
        return v->point();
    }

    inline double coord(const Delaunay_triangulation& dt, const Point& p, int i) const
    {
        return CGAL::to_double(p[i]);
    }

    template<typename V>
    bool are_vertices_equal(const Delaunay_triangulation& t1, V v1, const Delaunay_triangulation& t2, V v2) const
    {
        bool inf1 = vertex_is_infinite(t1, v1);
        bool inf2 = vertex_is_infinite(t2, v2);
        return (inf1 || inf2) ? (inf1 == inf2) : v1->point() == v2->point();
    }

    template<typename C>
    bool are_cells_equal(const Delaunay_triangulation& t1, C c1, const Delaunay_triangulation& t2, C c2) const
    {
        for(auto v1 = (c1)->vertices_begin(); v1 != (c1)->vertices_end(); ++v1 )
        {
            bool is_equal = false;
            for(auto v2 = (c2)->vertices_begin(); v2 != (c2)->vertices_end(); ++v2 )
            {
                if(are_vertices_equal(t1, *v1, t2, *v2))
                {
                    is_equal = true;
                    break;
                }
            }
            if(!is_equal)
                return false;
        }
        return true;
    }

    inline int index_of_covertex(const Delaunay_triangulation& dt, Facet_const_handle f) const
    {
        return f->second;
    }

    inline Cell_const_handle full_cell(const Delaunay_triangulation& dt, Facet_const_handle f) const
    {
        return f->first;
    }

    inline int mirror_index(const Delaunay_triangulation& dt, Cell_const_handle c, int i) const
    {
        return c->mirror_index(i);
    }

    inline Cell_const_iterator neighbor(const Delaunay_triangulation& dt, Cell_const_iterator c, int i) const
    {
        return c->neighbor(i);
    }

    Facet_const_iterator facet(const Delaunay_triangulation& dt, Cell_const_iterator c, int i) const
    {
        Facet f(c, i);
        return Facet_const_iterator(dt.tds(), f);
    }

    Point get_cell_barycenter(Cell_const_handle ch)
    {
        int D = ch->maximal_dimension();
        std::vector<double> coords(D);
        for(uint d = 0; d < D; d++)
            coords[d] = 0;
        for(auto vht = ch->vertices_begin() ;
                vht != ch->vertices_end() ;
                ++vht)
        {
            Vertex_handle v = *vht;
            for(uint d = 0; d < D; d++)
            {
                coords[d] += (v->point())[d];
            }
        }
        for(uint d = 0; d < D; d++)
            coords[d] /= ((double)D+1);
        return Point(D,coords.begin(),coords.end());
    }

    std::ostream & write_cgal(std::ostream & ofile, const Delaunay_triangulation& tri, bool do_data = true) const
    {

        int dim=tri.current_dimension();

        if(CGAL::is_ascii(ofile))
        {
            ofile << dim << " ";
        }
        else
        {
            ofile.write(reinterpret_cast<char *>(&dim), sizeof(dim));
        }
        // write the number of vertices
        uint n = tri.number_of_vertices();


        if(CGAL::is_ascii(ofile))
            ofile << n << " ";
        else
            ofile.write(reinterpret_cast<char *>(&n), sizeof(n));
        if(n == 0)
        {
            return ofile;
        }


        // write the vertices
        std::map<Vertex_const_handle, uint> vertex_map;
        vertex_map[tri.infinite_vertex()] = 0;
        uint i = 1;
        for(auto vit = tri.vertices_begin(); vit != tri.vertices_end(); ++vit)
        {
            if(tri.is_infinite(vit))
            {
                continue;
            }
            for(int d = 0; d < dim; d++)
            {
                //write_to_binary();
                //reinterpret_cast<char *>(&idx), sizeof(idx)
                double pv = vit->point()[d];
                if(CGAL::is_ascii(ofile))
                    ofile << pv << " ";
                else
                    ofile.write(reinterpret_cast<char *>(&pv), sizeof(pv));
            }
            if(do_data)
            {
                vit->data().write(ofile);
            }
            vertex_map[vit] = i;
            i++;
        }



        // write the number of cells
        n = tri.number_of_full_cells();

        if(CGAL::is_ascii(ofile))
            ofile << n << " ";
        else
            ofile.write(reinterpret_cast<char *>(&n), sizeof(n));

        // write the cells
        std::map<Cell_const_handle, uint> cell_map;
        i = 0;
        for(auto it = tri.full_cells_begin(); it != tri.full_cells_end(); ++it)
        {

            cell_map[it] = i;
            ++i;
            for(int d = 0; d < dim+1; d++)
            {
                if(CGAL::is_ascii(ofile))
                    ofile << vertex_map[it->vertex(d)] << " ";
                else
                    ofile.write(reinterpret_cast<char *>(&(vertex_map[it->vertex(d)])), sizeof((vertex_map[it->vertex(d)])));
            }
            if(do_data)
            {
                it->data().write(ofile);
            }
        }

        // write the neighbors of the cells
        for(auto it = tri.full_cells_begin(); it != tri.full_cells_end(); ++it)
            for(int j = 0; j < dim+1; j++)
            {
                if(CGAL::is_ascii(ofile))
                    ofile << cell_map[it->neighbor(j)] << " ";
                else
                    ofile.write(reinterpret_cast<char *>(&(cell_map[it->neighbor(j)])), sizeof((cell_map[it->neighbor(j)])));
            }
        return ofile;
    }


    std::istream& read_cgal(std::istream&  ifile, Delaunay_triangulation& tri, bool do_data = true)
    {

        uint ldim;
        if(CGAL::is_ascii(ifile))
            ifile >> ldim;
        else
            ifile.read(reinterpret_cast<char *>(&ldim), sizeof(ldim));

        Delaunay_triangulation & dt = tri;
        tri.clear();
        tri.set_current_dimension(ldim);

        auto cit = tri.full_cells_begin();
        Cell_handle inf_ch = cit;
        dt.tds().delete_full_cell(inf_ch);

        // 4) read the number of vertices
        uint num_v = 0;
        if(CGAL::is_ascii(ifile))
            ifile >> num_v;
        else
            ifile.read(reinterpret_cast<char *>(&num_v), sizeof(num_v));

        std::vector<Vertex_handle> vertex_map(num_v+1);
        vertex_map[0] = tri.infinite_vertex();

        // 5) read and create the vertices
        for(uint i = 1; i <= num_v; ++i)
        {
            std::vector<double> coords_v(ldim);
            for(uint d = 0; d < ldim; d++)
            {
                if(CGAL::is_ascii(ifile))
                    ifile >> coords_v[d];
                else
                    ifile.read(reinterpret_cast<char *>(&(coords_v[d])), sizeof(coords_v[d]));
            }
            Point p(ldim,coords_v.begin(),coords_v.end());
            vertex_map[i] = dt.new_vertex(p);
            // with_simplex_datastuff
            if(do_data)
            {
                vertex_map[i]->data().read(ifile);
            }
        }


        // 6) read the number of cells
        uint num_c = 0;
        if(CGAL::is_ascii(ifile))
            ifile >> num_c;
        else
            ifile.read(reinterpret_cast<char *>(&num_c), sizeof(num_c));

        // 7) read and create the cells
        std::vector<Cell_handle> cell_map(num_c);
        uint ik;


        for(uint i = 0; i < num_c; ++i)
        {
            Cell_handle ch = dt.new_full_cell();
            for (uint d = 0; d < ldim+1; d++)
            {

                if(CGAL::is_ascii(ifile))
                    ifile >> ik;
                else
                    ifile.read(reinterpret_cast<char *>(&ik), sizeof(ik));

                ch->set_vertex(d, vertex_map[ik]);
                vertex_map[ik]->set_full_cell(ch);
            }
            if(do_data)
            {
                ch->data().read(ifile);
            }

            cell_map[i] = ch;
        }

        // 8) read and construct neighbourhood relationships for cells
        for(uint j = 0; j < num_c; ++j)
        {
            Cell_handle ch  = cell_map[j];
            for(uint d = 0; d < ldim+1; d++)
            {
                if(CGAL::is_ascii(ifile))
                    ifile >> ik;
                else
                    ifile.read(reinterpret_cast<char *>(&ik), sizeof(ik));
                ch->set_neighbor(d, cell_map[ik]);
            }
        }


        // compute the mirror indices
        for(uint j = 0; j < num_c; ++j)
        {
            Cell_handle s  = cell_map[j];
            for( uint j = 0; j <= ldim; ++j )
            {
                if( -1 != s->mirror_index(j) )
                    continue;
                Cell_handle n = s->neighbor(j);
                int k = 0;
                Cell_handle nn = n->neighbor(k);
                while( s != nn )
                    nn = n->neighbor(++k);
                s->set_mirror_index(j,k);
                n->set_mirror_index(k,j);
            }
        }
        std::cout << "is_valid:" << dt.is_valid(true) << std::endl;
        assert(dt.is_valid(true));

        return ifile;
    }


};

template<unsigned int N, typename I, typename F>
struct Cgal_traits : public Cgal_traits_d<I,F,CGAL::Dimension_tag<N>>
{
    enum { D = N };
};

}

#endif // DDT_CGAL_TRAITS_D_HPP
