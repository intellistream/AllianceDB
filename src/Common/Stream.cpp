#include "Common/Stream.hpp"
#include "Utils/Logger.hpp"

#include <filesystem>
#include <fstream>

using namespace std;
using namespace AllianceDB;
using namespace std::filesystem;

Stream::Stream(const Param &param, StreamType st) : param(param), st(st)
{
    filename                        = (st == StreamType::R ? param.r : param.s);
    static const path search_dirs[] = {path("./"), path("datasets/"), path("../datasets/")};
    path path_to_file(filename);
    if (path_to_file.filename() == filename)
    {
        for (const auto &dir : search_dirs)
        {
            if (exists(dir / filename))
            {
                filename = (dir / filename).string();
                break;
            }
            if (exists(path(param.bin_dir) / dir / filename))
            {
                filename = (path(param.bin_dir) / dir / filename).string();
                break;
            }
        }
    }
    fs.open(filename, ios::in);
    if (!fs.is_open())
    {
        FATAL("cannot open file %s", filename.c_str());
    }
    else
    {
        INFO("find input %s", filename.c_str());
    }
}

void Stream::Load()
{
    std::string buffer;
    int count = 0;
    while (getline(fs, buffer))
    {
        TsType ts;
        KeyType key;
        ValType val;
        sscanf(buffer.data(), "%ld,%ld,%ld", &key, &val, &ts);
        ts             = count++;  // count-based window
        TuplePtr tuple = std::make_shared<Tuple>(key, val, st, ts);
        this->tuples.push_back(tuple);
        DEBUG("push tuple %s", tuple->toString().c_str());
    }
    fs.close();
    num_tuples = tuples.size();
    INFO("load %ld tuples from %s", num_tuples, filename.c_str());
}

void Stream::Open() { cnt = 0; }

TuplePtr Stream::Next()
{
    // TODO: thread-safe
    if (cnt < num_tuples)
    {
        return tuples[cnt++];
    }
    else
    {
        return nullptr;
    }
}

bool Stream::HasNext() { return cnt < num_tuples; }

const std::vector<TuplePtr> &Stream::Tuples() { return tuples; }

size_t Stream::NumTuples() { return num_tuples; }
