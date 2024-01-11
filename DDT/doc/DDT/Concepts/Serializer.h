/*!
\ingroup PkgDDTConcepts
\cgalConcept

The concept `Serializer` describes the requirements for the class that serializes and deserializes tiles to and from off-memory storage.
File serializers are enabling out of core processing, by streaming points in and out of memory.

\todo it is not clear if the serializer is used for buffering triangulation or even for writing the final result (for future usage for example).
A better scope should allow you to answer your question about `Distributed_triangulatioT::clear()`


\cgalHasModelsBegin
\cgalHasModels{CGAL::DDT::No_serializer}
\cgalHasModels{CGAL::DDT::File_serializer}
\cgalHasModels{CGAL::DDT::File_points_serializer}
\cgalHasModelsEnd

*/

class Serializer
{
public:
  /// checks if a tile id is available for deserialization
  template <typename TileIndex>
  bool is_readable(TileIndex) const { return false; }
  /// deserializes a tile, given its id, overwriting its triangulation on success.
  /// returns whether deserialization succeeded.
  /// \tparam `TileTriangulation` the tile triangulation.
  template<typename TileTriangulation> bool read(TileTriangulation&) const { return false; }
  /// serializes a tile.
  /// returns whether serialization succeeded.
  /// \tparam `TileTriangulation` the tile triangulation.
  template<typename TileTriangulation> bool write(const TileTriangulation& ) const { return false; }
};

/// serializes the serializer itself to the output stream
std::ostream& operator<<(std::ostream& out, const Serializer& serializer);
