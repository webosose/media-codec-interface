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

#include "log.h"
#include "thread.h"

namespace mcil {

Thread::Thread() = default;

Thread::Thread(const std::string& name)
 : thread_name_(name) {
}

Thread::~Thread() {
  Stop();
}

void Thread::PostTask(std::function<void()> task) {
  MCIL_DEBUG_PRINT(": %s", thread_name_.c_str());

  {
    std::lock_guard<std::mutex> lock(mutex_);
    task_queue_.push_back(task);
  }

  condition_.notify_all();
}

void Thread::Start() {
  MCIL_DEBUG_PRINT(": %s running[%d]", thread_name_.c_str(), is_running_);

  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (is_running_)
      return;

    is_running_ = true;
  }

  thread_ = std::thread(&Thread::RunInternal, this);
}

void Thread::Stop() {
  MCIL_DEBUG_PRINT(": %s running[%d]", thread_name_.c_str(), is_running_);

  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!is_running_)
      return;

    is_running_ = false;
  }

  condition_.notify_all();
  thread_.join();
}

void Thread::RunInternal() {
  for (;;) {
    decltype(task_queue_) local_queue;
    {
      std::unique_lock<std::mutex> lock(mutex_);
      condition_.wait(lock, [&] {
        return !task_queue_.empty() + !is_running_;
      });

      if (!is_running_) {
        for (auto& task : task_queue_)
          task();

        task_queue_.clear();
        return;
      }

      std::swap(task_queue_, local_queue);
    }

    for (auto& task : local_queue)
      task();
  }
}

}  //  namespace mcil
