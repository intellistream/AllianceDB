#pragma  once
#ifndef _SAFEQUEUE_H_
#define _SAFEQUEUE_H_
#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <algorithm>
#include <vector>
#include <string>
#include <queue>
#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <algorithm>
#include <vector>
#include <string>
#include <queue>
#include <barrier>

using namespace std;
namespace std {
typedef std::shared_ptr<std::barrier<>> BarrierPtr;
template<typename T>
class SafeQueue {
 public:
  mutex m_mut;
  condition_variable m_cond;
  queue<T> m_queue;

 public:
  SafeQueue() {}
  SafeQueue(const SafeQueue &rhs) {
    lock_guard<mutex> lk(rhs.m_mut);
    m_queue = rhs.m_queue;
  }
  void push(T data) {
    lock_guard<mutex> lk(m_mut);  // 使用效率高的lock_guard
    m_queue.push(move(data));  // 移动构造函数，防止对象不能拷贝
    m_cond.notify_one(); // 通知唤醒阻塞的一个线程
  }
  void pushNoMove(T data) {
    lock_guard<mutex> lk(m_mut);  // 使用效率高的lock_guard
    m_queue.push(data);  // 移动构造函数，防止对象不能拷贝
    m_cond.notify_one(); // 通知唤醒阻塞的一个线程
  }

  void waitAndPop(T &res)  // 直到队列不为空
  {
    unique_lock<mutex> lk(m_mut);
    m_cond.wait(lk, [this] { return !m_queue.empty(); });
    res = move(m_queue.front());
    m_queue.pop();
  }
  auto pop() {
    unique_lock<mutex> lk(m_mut);
    m_cond.wait(lk, [this] { return !m_queue.empty(); });
    move(m_queue.front());
    return m_queue.pop();
  }
  auto size() {
    lock_guard<mutex> lk(m_mut);  // 使用效率高的lock_guard
    auto sz = m_queue.size();
    return sz;
  }
  auto empty() {
    lock_guard<mutex> lk(m_mut);  // 使用效率高的lock_guard
    auto sz = m_queue.empty();

    return sz;
  }
  auto front() {
    lock_guard<mutex> lk(m_mut);  // 使用效率高的lock_guard
    auto fr = m_queue.front();
    return fr;
  }
  void operator=(const SafeQueue &D) {
    m_queue = D.m_queue;
  }
  void operator=(const std::queue<T> &D) {
    m_queue = D;
  }
  void waitAndPopNoMOve(T &res)  // 直到队列不为空
  {
    unique_lock<mutex> lk(m_mut);
    m_cond.wait(lk, [this] { return !m_queue.empty(); });
    m_queue.pop();
  }
  bool tryPop(T &res)  //立即返回
  {
    lock_guard<mutex> lk(m_mut);
    if (m_queue.empty())
      return false;
    res = move(m_queue.front());
    return true;
  }
  /*
  下面这种是由返回值返回元素。
  */
  shared_ptr<T> waitAndPop() {
    unique_lock<mutex> lk(m_mut);
    m_cond.wait(lk, [this] { return !m_queue.empty(); });
    shared_ptr<T> res(make_shared<T>(move(m_queue.front())));
    m_queue.pop();
    return res;
  }

  shared_ptr<T> tryPop() {
    lock_guard<mutex> lk(m_mut);
    if (m_queue.empty())
      return NULL;
    shared_ptr<T> res(make_shared<T>(move(m_queue.front())));
    m_queue.pop();
    return res;
  }
};

typedef std::shared_ptr<SafeQueue<int>> SafeQueuePtr;
}

#endif