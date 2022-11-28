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

#ifndef CGAL_DDT_CGAL_TRAITS_D_H
#define CGAL_DDT_CGAL_TRAITS_D_H


#include <CGAL/DDT/Bbox.h>
#include <CGAL/DDT/traits/Facet_const_iterator_d.h>
#include <CGAL/DDT/traits/Data.h>

#include <CGAL/Epick_d.h>
#include <CGAL/Delaunay_triangulation.h>
#include <CGAL/Triangulation_ds_vertex.h>
#include <CGAL/point_generators_d.h>
#include <CGAL/Spatial_sort_traits_adapter_d.h>


#include <functional>
#include <limits>


typedef std::numeric_limits< double > dbl;

namespace CGAL {
namespace DDT {

/// \ingroup PkgDDTTraitsClasses
/// \cgalModels TriangulationTraits
template<typename I, typename F = No_info, typename Dim = CGAL::Dynamic_dimension_tag>
struct Cgal_traits_d
{
    typedef Dim                                                    Dim_tag;
    typedef I                                                      Id;
    typedef F                                                      Info;
    typedef CGAL::DDT::Data<Id, Info>                              Data;
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
    inline Info& info(Vertex_const_handle v) const
    {
        return v->data().info;
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

    void spatial_sort(const Delaunay_triangulation& dt, std::vector<std::size_t>& indices, const std::vector<Point>& points) const
    {
        using Geom_traits = K;
        typedef typename Pointer_property_map<Point>::const_type Pmap;
        typedef Spatial_sort_traits_adapter_d<Geom_traits,Pmap> Search_traits;

        CGAL::spatial_sort(indices.begin(), indices.end(),
                     Search_traits(make_property_map(points), dt.geom_traits()));
    }

    inline void incident_cells(const Delaunay_triangulation& dt, std::vector<Cell_const_handle>& cells, Vertex_const_handle v) const
    {
        cells.reserve(64);
        dt.incident_full_cells(v, std::back_inserter(cells));
    }

    inline void adjacent_vertices(const Delaunay_triangulation& dt, std::vector<Vertex_handle>& adj, Vertex_const_handle v) const
    {
        std::vector<Cell_const_handle> cells;
        incident_cells(dt, cells, v);

        std::set<Vertex_handle> vertices;
        for(Cell_const_handle c : cells)
        {
            for( int i = 0; i <= dt.current_dimension(); ++i )
                vertices.insert(c->vertex(i));
        }
        for( Vertex_handle w : vertices )
            if(w != v)
                adj.push_back(w);
    }

    inline std::pair<Vertex_handle, bool> insert(Delaunay_triangulation& dt, const Point& p, Id id, Vertex_handle hint = Vertex_handle()) const
    {
        typename Delaunay_triangulation::Locate_type lt;
        typename Delaunay_triangulation::Face f(dt.maximal_dimension());
        typename Delaunay_triangulation::Facet ft;
        if (hint == Vertex_handle()) hint = dt.infinite_vertex();
        Cell_handle c = dt.locate(p, lt, f, ft, hint);
        if(lt == Delaunay_triangulation::ON_VERTEX) {
            Vertex_handle v = c->vertex(f.index(0));
            v->set_point(p);
            assert(id == v->data().id);
            return std::make_pair(v, false);
        }
        Vertex_handle v = dt.insert(p, lt, f, ft, c);
        v->data().id = id;
        return std::make_pair(v, true);
    }


    inline void remove(Delaunay_triangulation& dt, Vertex_handle v) const
    {
        dt.remove(v);
    }

    inline Point circumcenter(const Delaunay_triangulation& dt, Cell_const_handle c) const
    {
        /// @todo
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

    bool are_vertices_equal(const Delaunay_triangulation& t1, Vertex_const_handle v1, const Delaunay_triangulation& t2, Vertex_const_handle v2) const
    {
        bool inf1 = vertex_is_infinite(t1, v1);
        bool inf2 = vertex_is_infinite(t2, v2);
        return (inf1 || inf2) ? (inf1 == inf2) : v1->point() == v2->point();
    }

    bool are_facets_equal(const Delaunay_triangulation& t1, Facet_const_handle f1, const Delaunay_triangulation& t2, Facet_const_handle f2) const
    {
        auto c1 = f1->first;
        auto c2 = f2->first;
        int icv1 = f1->second;
        int icv2 = f2->second;
        for(int i1 = 0; i1 < t1.current_dimension(); ++i1 )
        {
            if(i1 == icv1) continue;
            auto v1 = c1->vertex(i1);
            bool found = false;
            for(int i2 = 0; i2 < t2.current_dimension(); ++i2 )
            {
                if(i2 == icv2) continue;
                auto v2 = c2->vertex(i2);
                if(are_vertices_equal(t1, v1, t2, v2))
                {
                    found = true;
                    break;
                }
            }
            if(!found)
                return false;
        }
        return true;
    }

    bool are_cells_equal(const Delaunay_triangulation& t1, Cell_const_handle c1, const Delaunay_triangulation& t2, Cell_const_handle c2) const
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

    inline Cell_const_handle cell(const Delaunay_triangulation& dt, Facet_const_handle f) const
    {
        return f->first;
    }

    inline Cell_const_handle cell(const Delaunay_triangulation& dt, Vertex_const_handle v) const
    {
        return v->full_cell();
    }

    inline Vertex_const_handle covertex(const Delaunay_triangulation& dt, Facet_const_handle f) const
    {
        return vertex(dt, f->first, f->second);
    }

    inline Vertex_const_handle mirror_vertex(const Delaunay_triangulation& dt, Facet_const_handle f) const
    {
        Cell_const_iterator c = f->first;
        return vertex(dt, c->neighbor(f->second), c->mirror_index(f->second));
    }

    Facet_const_handle mirror_facet(const Delaunay_triangulation& dt, Facet_const_handle f) const
    {
        Cell_const_iterator c = f->first;
        Facet g(c->neighbor(f->second), c->mirror_index(f->second));
        return Facet_const_iterator(dt.tds(), g);
    }

    inline int mirror_index(const Delaunay_triangulation& dt, Facet_const_handle f) const
    {
        return f->first->mirror_index(f->second);
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

    Vertex_const_handle locate_vertex(const Delaunay_triangulation& dt, const Point& p) const
    {
        typename Delaunay_triangulation::Locate_type  lt;
        typename Delaunay_triangulation::Face f(dt.maximal_dimension());
        typename Delaunay_triangulation::Facet ft;
        Cell_handle c = dt.locate(p, lt, f, ft);
        if(lt!=Delaunay_triangulation::ON_VERTEX) return infinite_vertex(dt);
        return f.vertex(0);
    }

};

/// \ingroup PkgDDTTraitsClasses
/// \cgalModels TriangulationTraits
template<unsigned int N, typename I, typename F = No_info>
struct Cgal_traits : public Cgal_traits_d<I,F,CGAL::Dimension_tag<N>>
{
    enum { D = N };
};

}
}

#endif // CGAL_DDT_CGAL_TRAITS_D_H
