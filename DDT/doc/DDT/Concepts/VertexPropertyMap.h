#ifndef CGAL_DDT_CONCEPT_VERTEX_PROPERTY_MAP
#define CGAL_DDT_CONCEPT_VERTEX_PROPERTY_MAP

/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `VertexPropertyMap` describes the requirements of a property map from Vertex_indices to Tile_indices.

\cgalHasModel `CGAL::DDT::Vertex_info_property_map`
\cgalHasModel `CGAL::DDT::Vertex_data_property_map`
\cgalHasModel `CGAL::DDT::Vertex_info_id_property_map`
\cgalHasModel `CGAL::DDT::Vertex_data_id_property_map`

*/
#include <boost/property_map/property_map.hpp>

struct VertexPropertyMap
{
/// \cond SKIP_IN_MANUAL
  typedef VertexPropertyMap Self;
  typedef ::VertexIndex key_type;
  typedef ::TileIndex value_type;
  typedef value_type& reference;
  typedef boost::lvalue_property_map_tag category;

  value_type& operator[](key_type& k) const { return Self::value; }

  friend value_type& get(const Self&, key_type& k) { return Self::value; }
  friend const value_type& get(const Self&, const key_type& k) { return Self::value; }
  friend void put(const Self&, key_type& k, const value_type& v) { }
/// \endcond

private:
  static value_type value;
};

VertexPropertyMap::value_type VertexPropertyMap::value = {};

#endif // CGAL_DDT_CONCEPT_VERTEX_PROPERTY_MAP
