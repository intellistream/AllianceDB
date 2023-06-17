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

#include "Join/SplitJoinOrigin.hpp"

#include <cassert>
#include <cmath>
#include <memory>
#include <thread>

using namespace std;
using namespace AllianceDB;

SplitJoinOrigin::SplitJoinOrigin(const Param &param, size_t wid) : param(param), tuples(20000) {
  auto num_threads = param.num_threads;
  uint32 sub_window = param.window_length / num_threads;
  for (int i = 0; i < num_threads; ++i) {
    JCs.push_back(std::make_shared<JoinCore>(param));
    JCs[i]->sub_window = sub_window;
    JCs[i]->JC_id = i;
    JCs[i]->window_id = wid;
  }
}

void SplitJoinOrigin::Feed(TuplePtr tuple) { tuples.push(tuple); }

void SplitJoinOrigin::Start(Context &ctx) {
  for (auto &jc : JCs) jc->Start(ctx);
  auto func = [this, &ctx]() { this->Run(ctx); };
  t = make_shared<thread>(func);
  assert(t);
}

void SplitJoinOrigin::Process(Context &ctx) {
  while (!tuples.empty()) {
    TuplePtr tuple;
    tuples.pop(tuple);
    int idx = int(tuple->ts) % int(param.num_threads);
    // JCs[idx]->inputs_store.push(tuple);
    JCs[idx]->Store(tuple);
    for (auto i = 0; i < param.num_threads; ++i) {
      // JCs[i]->inputs_find.push(tuple);
      JCs[i]->Find(ctx, tuple);
    }
  }
}

void SplitJoinOrigin::Run(Context &ctx) {
  while (true) {
    if (!status) {
      Process(ctx);
      break;
    }
    Process(ctx);
  }
}

void SplitJoinOrigin::Wait() {
  status = false;
  if (t) t->join();
  for (int i = 0; i < param.num_threads; ++i) {
    JCs[i]->Wait();
  }
}

SplitJoinOrigin::JoinCore::JoinCore(const Param &param)
    : param(param),
      inputs_store(param.window_length / param.num_threads),
      inputs_find(param.window_length),
      status(true),
      sub_window(param.window_length / param.num_threads) {}

void SplitJoinOrigin::JoinCore::Run(Context &ctx) {
  while (true) {
    if (!this->status) {
      break;
    }
    if (!inputs_store.empty()) {
      TuplePtr tuple;
      inputs_store.pop(tuple);
      Store(tuple);
    }
    if (!inputs_find.empty()) {
      TuplePtr tuple;
      inputs_find.pop(tuple);
      Find(ctx, tuple);
    }
  }
  while (!inputs_store.empty()) {
    TuplePtr tuple;
    inputs_store.pop(tuple);
    Store(tuple);
  }
  while (!inputs_find.empty()) {
    TuplePtr tuple;
    inputs_find.pop(tuple);
    Find(ctx, tuple);
  }
}

void SplitJoinOrigin::JoinCore::Start(Context &ctx) {
  auto func = [this, &ctx]() { this->Run(ctx); };
  t = make_shared<thread>(func);
}

void SplitJoinOrigin::JoinCore::Store(TuplePtr tuple) {
  // we currently make every JoinCore will only receive same mount tuples as sub_window, so not
  // necessary to check the size of store region, but we reserve this for future work;
  if (tuple->st == StreamType::R) {
    if (right_region.size() == sub_window) {
      auto t = right_region.front();
      right_region.pop();
      auto &set_v = map_idx_right[t->key];
      set_v.erase(t);
      if (set_v.empty()) {
        map_idx_right.erase(t->key);
      }
    }
    right_region.push(tuple);
    map_idx_right[tuple->key].insert(tuple);
  } else {
    if (left_region.size() == sub_window) {
      auto t = left_region.front();
      left_region.pop();
      auto &set_v = map_idx_left[t->key];
      set_v.erase(t);
      if (set_v.empty()) {
        map_idx_left.erase(t->key);
      }
    }
    left_region.push(tuple);
    map_idx_left[tuple->key].insert(tuple);
  }
}

void SplitJoinOrigin::JoinCore::Find(Context &ctx, TuplePtr tuple) {
  if (tuple->st == StreamType::R) {
    auto result = map_idx_left.find(tuple->key);
    if (result == map_idx_left.end()) {
      return;
    }
    for (auto i : result->second) {
      ctx.joinResults->EmitAllWindow(i, tuple);
      // DEBUG("window %d has found one matched tuple", window_id);
    }
  } else {
    auto result = map_idx_right.find(tuple->key);
    if (result == map_idx_right.end()) {
      return;
    }
    for (auto i : result->second) {
      ctx.joinResults->EmitAllWindow(tuple, i);
    }
  }
}

void SplitJoinOrigin::JoinCore::Wait() {
  this->status = false;
  if (t) t->join();
}