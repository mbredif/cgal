/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `Serializer` describes the requirements for the class that assigns tile identifiers to points.
\cgalHasModel `CGAL::DDT::No_serializer`
\cgalHasModel `CGAL::DDT::File_serializer`
\cgalHasModel `CGAL::DDT::File_points_serializer`

*/

class Serializer
{
public:
  /// The type of the tile identifier.
  typedef unspecified_type Id;
  /// The type of the tile.
  typedef unspecified_type Tile;
  /// Checks if a tile id is available for deserialization
  bool has_tile(Id) const { return false; }
  /// Deserialize a tile, given its id, overwriting its triangulation on success. Returns whether deserialization succeeded.
  bool load(Tile&) const { return false; }
  /// Serialize a tile. Returns whether deserialization succeeded.
  bool save(const Tile& ) const { return false;}
};

/// Streaming
std::ostream& operator<<(std::ostream& out, const Serializer& serializer);
