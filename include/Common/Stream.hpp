#ifndef ALLIANCEDB_INCLUDE_COMMON_STREAM_HPP_
#define ALLIANCEDB_INCLUDE_COMMON_STREAM_HPP_

#include "Common/Param.hpp"
#include "Common/Tuple.hpp"

#include <fstream>
#include <memory>
#include <vector>

namespace AllianceDB {
typedef std::shared_ptr<class Stream> StreamPtr;
class Stream {
private:
  Param param;
  std::vector<TuplePtr> tuples;
  std::string filename;
  StreamType st;
  std::fstream fs;
  int cnt = 0;
  int num_tuples = 0;

public:
  Stream(const Param &param, const std::string &name, StreamType st);
  void Load();
  TuplePtr Next();
  bool End();
  const std::vector<TuplePtr> &Tuples();
};

} // namespace AllianceDB
#endif // ALLIANCEDB_INCLUDE_COMMON_STREAM_HPP_
