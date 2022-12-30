#ifndef ALLIANCEDB_INCLUDE_COMMON_STREAM_HPP_
#define ALLIANCEDB_INCLUDE_COMMON_STREAM_HPP_

#include "Common/Param.hpp"
#include "Common/Tuple.hpp"

#include <fstream>
#include <memory>
#include <vector>

namespace AllianceDB
{
typedef std::shared_ptr<class Stream> StreamPtr;
class Stream
{
private:
    Param param;
    std::vector<TuplePtr> tuples;
    std::string filename;
    StreamType st;
    std::fstream fs;
    size_t cnt        = 0;
    size_t num_tuples = 0;

public:
    Stream(const Param &param, StreamType st);
    void Load();
    void Open();
    TuplePtr Next();
    bool HasNext();
    const std::vector<TuplePtr> &Tuples();
    size_t NumTuples();
};

}  // namespace AllianceDB
#endif  // ALLIANCEDB_INCLUDE_COMMON_STREAM_HPP_
