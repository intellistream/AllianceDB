#ifndef ALLIANCEDB_INCLUDE_COMMON_STREAM_HPP_
#define ALLIANCEDB_INCLUDE_COMMON_STREAM_HPP_

#include <memory>
#include <vector>
#include <Common/Tuple.hpp>

namespace AllianceDB {
typedef std::shared_ptr<class Stream> StreamPtr;
class Stream {
 private:
  std::vector<AllianceDB::TuplePtr> Tuples;
 public:
  void Load(const std::string &fileName);
};

}
#endif //ALLIANCEDB_INCLUDE_COMMON_STREAM_HPP_
