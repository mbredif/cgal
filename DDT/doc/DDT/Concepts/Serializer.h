/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `Serializer` describes the requirements for the class that serializes and deserializes tiles to and from off-memory storage.
File serializers are enabling out of core processing, by streaming points in and out of memory.
\cgalHasModel `CGAL::DDT::No_serializer`
\cgalHasModel `CGAL::DDT::File_serializer`
\cgalHasModel `CGAL::DDT::File_points_serializer`

*/

class Serializer
{
public:
  /// Checks if a tile id is available for deserialization
  template <typename TileIndex>
  bool is_readable(TileIndex) const { return false; }
  /// Deserialize a tile, given its id, overwriting its triangulation on success. Returns whether deserialization succeeded.
  template<typename TileTriangulation> bool read(TileTriangulation&) const { return false; }
  /// Serialize a tile. Returns whether serialization succeeded.
  template<typename TileTriangulation> bool write(const TileTriangulation& ) const { return false;}
};

/// Streaming
std::ostream& operator<<(std::ostream& out, const Serializer& serializer);
