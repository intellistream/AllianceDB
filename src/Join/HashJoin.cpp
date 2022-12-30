#include "Join/HashJoin.hpp"

#include <unistd.h>
#include <iostream>

using namespace std;
using namespace AllianceDB;

// HashJoin::HashJoin(const Param &param, WindowPtr wr, WindowPtr ws) : param(param), wr(*wr),
// ws(*ws)
// {}

// void HashJoin::Run(size_t rbeg, size_t rend, size_t sbeg, size_t send, WindowJoinResult &result)
// {
//     std::unordered_map<KeyType, std::vector<TuplePtr>> r_map;
//     for (size_t i = rbeg; i < rend; ++i)
//     {
//         r_map[wr[i]->key].push_back(wr[i]);
//     }
//     for (size_t i = sbeg; i < send; ++i)
//     {
//         auto it = r_map.find(ws[i]->key);
//         if (it != r_map.end())
//         {
//             for (auto &r : it->second)
//             {
//                 result.push_back(ResultTuple(ws[i]->key, ws[i]->val, r->val));
//             }
//         }
//     }
//     // sleep(1);
// }

// void HashJoin::Feed(TuplePtr tuple)
// {
//     // TODO
// }