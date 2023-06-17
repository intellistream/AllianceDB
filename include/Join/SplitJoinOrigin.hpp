/*
 * Copyright 2022 IntelliStream team (https://github.com/intellistream)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ALLIANCEDB_INCLUDE_JOIN_SPLITJOINORIGIN_HPP_
#define ALLIANCEDB_INCLUDE_JOIN_SPLITJOINORIGIN_HPP_

#include "Common/Context.hpp"
#include "Common/Tuple.hpp"
#include "Join/Join.hpp"
#include "Utils/Queue.hpp"

#include <queue>
#include <set>
#include <unordered_map>
#include <vector>

namespace AllianceDB {
class SplitJoinOrigin : public JoinAlgo {
 public:
  SplitJoinOrigin(const Param &param, size_t wid);
  void Feed(TuplePtr tuple);
  void Wait();
  void Run(Context &ctx);
  void Start(Context &ctx);
  void Process(Context &ctx);
  struct JoinCore {
    const Param &param;
    // diff from SplitJoin
    std::queue<TuplePtr> right_region, left_region;
    std::unordered_map<KeyType, std::set<TuplePtr>> map_idx_right, map_idx_left;
    bool status;
    size_t sub_window;
    ThreadPtr t;
    int64_t JC_id, window_id;
    spsc_queue<TuplePtr> inputs_find, inputs_store;
    JoinCore(const Param &param);
    void Run(Context &ctx);
    void Start(Context &ctx);
    void Store(TuplePtr tuple);
    void Find(Context &ctx, TuplePtr tuple);
    void Wait();
  };
  using JoinCorePtr = std::shared_ptr<JoinCore>;

 private:
  const Param &param;
  int64_t window;
  std::vector<int> record_r, record_l;
  spsc_queue<TuplePtr> tuples;
  ThreadPtr t;
  std::vector<JoinCorePtr> JCs;
  bool status = true;
};
}  // namespace AllianceDB

#endif