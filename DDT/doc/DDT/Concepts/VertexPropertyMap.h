#ifndef CGAL_DDT_CONCEPT_VERTEX_PROPERTY_MAP
#define CGAL_DDT_CONCEPT_VERTEX_PROPERTY_MAP

/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `VertexPropertyMap` describes the requirements of a readable property map from (Triangulation, Vertex_index) pairs to Tile_indices.
If the mapped value does not only depend on the point embedding of the vertex, then it should also be writable.

\cgalHasModelsBegin
\cgalHasModels{CGAL::DDT::Partitioner_property_map}
\cgalHasModels{CGAL::DDT::Vertex_info_property_map}
\cgalHasModels{CGAL::DDT::Vertex_data_property_map}
\cgalHasModels{CGAL::DDT::Vertex_info_id_property_map}
\cgalHasModels{CGAL::DDT::Vertex_data_id_property_map}
\cgalHasModelsEnd

*/
#include <boost/property_map/property_map.hpp>

struct VertexPropertyMap
{
/// \cond SKIP_IN_MANUAL
  typedef VertexPropertyMap Self;
  typedef std::pair<const ::Triangulation&, ::VertexIndex> key_type;
  typedef ::TileIndex value_type;
  typedef boost::readable_property_map_tag category;

  friend value_type get(const Self&, const key_type& k) { return Self::value; }
  friend void put(const Self&, const key_type& k, const value_type& v) { }
/// \endcond

private:
  static value_type value;
};

typename  VertexPropertyMap::value_type VertexPropertyMap::value = {};

#endif // CGAL_DDT_CONCEPT_VERTEX_PROPERTY_MAP
