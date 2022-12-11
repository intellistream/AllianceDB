#include <Common/Stream.hpp>
#include <Utils/Logger.hpp>

#include <filesystem>
#include <fstream>

using namespace std;
using namespace AllianceDB;
using namespace std::filesystem;

Stream::Stream(const Param &param, const std::string &file, StreamType st)
    : param(param), filename(file), st(st) {
  static const path search_dirs[] = {path("./"), path("datasets/"), path("../datasets/")};
  path path_to_file(filename);
  if (path_to_file.filename() == filename) {
    for (const auto &dir : search_dirs) {
      if (exists(dir / filename)) {
        filename = (dir / filename).string();
        break;
      }
      if (exists(path(param.bin_dir) / dir / filename)) {
        filename = (path(param.bin_dir) / dir / filename).string();
        break;
      }
    }
  }
  fs.open(filename, ios::in);
  if (!fs.is_open()) {
    ERROR("cannot open file %s", filename.c_str());
  } else {
    INFO("find input %s", filename.c_str());
  }
}

void Stream::Load() {
  std::string buffer;
  while (getline(fs, buffer)) {
    TsType ts;
    KeyType key;
    ValType val;
    sscanf(buffer.data(), "%ld,%ld,%ld", &key, &val, &ts);
    // KeyType key = stoi(buffer);
    TuplePtr tuple = std::make_shared<Tuple>(key, val, st, ts);
    this->Tuples.push_back(tuple);
    TRACE("push tuple " + tuple->toString());
  }
  fs.close();
  INFO("load %ld tuples from %s", Tuples.size(), filename.c_str());
}
