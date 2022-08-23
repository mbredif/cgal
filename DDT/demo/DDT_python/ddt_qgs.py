##DDT=group
##Distributed Delaunay Triangulation=name
##insert_input_points=boolean False
##input_points=vector point
##number_of_random_points=number 10000
##output_triangulation=output vector
##output_points=output vector
##number_of_tiles_per_side=number 2
##number_of_threads=number 0

from qgis.core import *
from PyQt4.QtCore import *
from processing.tools.vector import VectorWriter
from pyddt import *

verbosity = 1
extent = 1
bbox = Bbox(-extent,extent)
rand_points = Random_points(extent)
npoints = number_of_random_points


number_of_tiles = number_of_tiles_per_side*number_of_tiles_per_side
tri = DDT(verbosity, number_of_threads)


if number_of_random_points:
    tri.send_points(rand_points, 10000, Grid_partitioner(bbox,4))


layer = processing.getObject(input_points)
if insert_input_points and input_points:
    points = []
    features = processing.features(layer)
    for feature in features:
        geom = feature.geometry()
        if geom.type() == QGis.Point:
            points.append(geom.asPoint().x())
            points.append(geom.asPoint().y())

    tri.send_points(points, len(points), Grid_partitioner(bbox,4))
    npoints += len(points)/2

print("Triangulating %d points" % npoints)
print(tri.insert_received_points())
print(tri.send_all_bbox_points())
print(tri.splay_stars())


print("Exporting to QGIS")
fields = [QgsField("tid", QVariant.Int),QgsField("local",QVariant.Int)]
writer = processing.VectorWriter(output_triangulation, None, fields, QGis.WKBPolygon, layer.crs())

for tile in tri.tiles():
    tid = tile.id()
    progress.setPercentage(int(100*tid/number_of_tiles))
    for cell in tile.cells():
        if not cell.is_infinite():
            feat = QgsFeature()
            local = 0
            p = []
            for i in range(3):
                x, y, vid = cell.point(i)
                p.append(QgsPoint(x, y))
                if vid == tid:
                    local = local + 1
            geom = QgsGeometry.fromPolygon([p])
            feat.setGeometry(geom)
            feat.setAttributes([tile.id(),local])
            writer.addFeature(feat)

del writer


fields = [QgsField("tid", QVariant.Int), QgsField("vid", QVariant.Int)]
writer = processing.VectorWriter(output_points, None, fields, QGis.WKBPoint, layer.crs())
for tile in tri.tiles():
    tid = tile.id()
    progress.setPercentage(int(100*tid/number_of_tiles))
    for vertex in tile.vertices():
        feat = QgsFeature()
        x, y, vid = vertex.point()
        geom = QgsGeometry.fromPoint(QgsPoint(x, y))
        feat.setGeometry(geom)
        feat.setAttributes([tid,vid])
        writer.addFeature(feat)

del writer

