#ifndef ALLIANCEDB_INCLUDE_COMMON_STREAM_HPP_
#define ALLIANCEDB_INCLUDE_COMMON_STREAM_HPP_

#include "Common/Param.hpp"
#include "Common/Tuple.hpp"

#include <memory>
#include <vector>
#include <fstream>

namespace AllianceDB {
typedef std::shared_ptr<class Stream> StreamPtr;
class Stream {
 private:
  Param param;
  std::vector<AllianceDB::TuplePtr> Tuples;
  std::string filename;
  StreamType st;
  std::fstream fs;
 public:
  Stream(const Param &param, const std::string& name, StreamType st);
  void Load();
};

}
#endif //ALLIANCEDB_INCLUDE_COMMON_STREAM_HPP_
