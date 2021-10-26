// Copyright (c) 2021 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef SRC_BASE_MCIL_THREAD_H_
#define SRC_BASE_MCIL_THREAD_H_

#include <condition_variable>
#include <list>
#include <functional>
#include <thread>

namespace mcil {

class Thread {
 public:
  Thread();
  Thread(const std::string& name);
  ~Thread();

  bool IsRunning() { return is_running_; }
  void Start();
  void Stop();

  void PostTask(std::function<void()>);

 private:
  void RunInternal();

  std::condition_variable condition_;
  std::list<std::function<void()>> task_queue_;
  std::mutex mutex_;
  std::thread thread_;
  bool is_running_ = false;
  std::string thread_name_;
};

}  // namespace mcil

#endif  // SRC_BASE_MCIL_THREAD_H_
