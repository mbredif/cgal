#!/usr/bin/env python

from itertools import islice, count
from pyddt import *
import random
import os
import sys



print("\n\n======= Start computation ======")
# Params

nb_threads=1
extent = 1
nb_tile_side=5

Dim=2
bbox = Bbox(-extent,extent)
rand_points = Random_points(2,extent)
tri1 = DDT(nb_threads)
number_of_vertex=3000
test_io = False

## usefull function
def dump_vrt(output_path_vrt,tt) :
    if(not os.path.isdir(output_path_vrt)):
        os.mkdir(output_path_vrt)
    tt.write_vrt_cells(output_path_vrt)
    tt.write_vrt_verts(output_path_vrt)
    tt.write_vrt_bbox(output_path_vrt + "/bbox.vrt")
    tt.write_vrt_bbox_vert(output_path_vrt +  "/bbox_vert.vrt")
    tt.write_vrt_vert(output_path_vrt + "/out_full_vert.vrt")
    tt.write_vrt_cell(output_path_vrt +  "/out_full_cell.vrt")
    tt.write_vrt_facet(output_path_vrt +  "/out_full_facet.vrt")
    #tt.write_json_tri(output_path_vrt +  "/full_tri.json")

# Insertion
tri1.send_points(rand_points, number_of_vertex, Grid_partitioner(bbox,nb_tile_side))
print(tri1.insert_received_points(True))
print(tri1.send_all_bbox_points())
print(tri1.splay_stars())
tri1.finalize()

print("Is tri1 valid? " + str(tri1.is_valid()))

#### Output ########
nbc=tri1.number_of_cells()
nbv=tri1.number_of_vertices()
nbf=tri1.number_of_facets()
ee=nbv-nbf+nbc
print("number of cells     :" + str(nbc))
print("number of vertices  :" + str(nbv))
print("number of facets    :" + str(nbf))
print("Euler : Vertices - facets + Cells = " + str(ee))


#### Dump vrt for visualization
print("=== Dump vrt / json ===")
dump_vrt("./python/",tri1)

### Dump tri1angulation
if test_io :  
    print("=== Dump CGAL ===")
    output_path_cgal="./cgal/"
    if(not os.path.isdir(output_path_cgal)):
        os.mkdir(output_path_cgal)
    tri1.write_cgal(output_path_cgal)
    ### Load triangulation
    print("=== Load triangulation === ")
    # Load with file dir
    tri2 = DDT(nb_threads)
    print("=== read cgal === ")
    tri2.read_cgal(output_path_cgal)
    print("=== read cgal done === ")
    dump_vrt("./vrt_2/",tri2);
    print("=== Load triangulation done === ")
    print("Is tri1 valid? " + tri2.is_valid())



# c = 0
# for tile in tri1.tiles():
#     tid = tile.id()
#     for cell in tile.cells():
#         p = cell.point(0)
#         c = c + 1



# c = 0
# for tile in tri1.tiles():
#     tid = tile.id()
#     for cell in tile.cells():
#         if not cell.is_infinite():
#             p = cell.point(0)
#             c = c+1

        
tri1.write_adjacency_graph_dot("./python/quotient_u.dot", False)
tri1.write_adjacency_graph_dot("./python/quotient_d.dot", True)
