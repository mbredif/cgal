// Copyright (c) 2022 Institut Géographique National - IGN (France)
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Mathieu Brédif and Laurent Caraffa

#ifndef CGAL_DDT_SCHEDULER_THREAD_POOL_H
#define CGAL_DDT_SCHEDULER_THREAD_POOL_H

#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>
#include <queue>

#include <CGAL/DDT/scheduler/multithread_scheduler/safe.h>

namespace CGAL {
namespace DDT {

class thread_pool
{
private:
    class thread_worker
    {
    private:
        thread_pool * m_pool;
        int m_id;
    public:
        thread_worker(thread_pool * pool, const int id)
            : m_pool(pool), m_id(id)
        {
        }

        void operator()()
        {
            std::function<void()> func;
            bool dequeued;
            while (!m_pool->m_shutdown)
            {
                {
                    std::unique_lock<std::mutex> lock(m_pool->m_conditional_mutex);
                    if (m_pool->m_queue.empty())
                    {
                        m_pool->m_conditional_lock.wait(lock);
                    }
                    dequeued = m_pool->m_queue.dequeue(func);
                }
                if (dequeued)
                {
                    func();
                }
            }
        }
    };

    std::vector<std::thread> m_threads;
    std::map<std::thread::id, int> m_thread_ids;
    bool m_shutdown;
    safe<std::queue<std::function<void()>>> m_queue;
    std::mutex m_conditional_mutex;
    std::condition_variable m_conditional_lock;
public:
    thread_pool(const int max_concurrency)
        : m_threads(max_concurrency?max_concurrency:std::thread::hardware_concurrency()), m_shutdown(false)
    {
    }

    thread_pool(const thread_pool &) = delete;
    thread_pool(thread_pool &&) = delete;

    thread_pool & operator=(const thread_pool &) = delete;
    thread_pool & operator=(thread_pool &&) = delete;

    inline int max_concurrency() const
    {
        return m_threads.size();
    }

    int thread_index() const { return m_thread_ids.at(std::this_thread::get_id()); }

    // Inits thread pool
    void init()
    {
        m_thread_ids[std::this_thread::get_id()] = 0;
        for (std::size_t i = 0; i < m_threads.size(); ++i)
        {
            m_threads[i] = std::thread(thread_worker(this, i));
            m_thread_ids[m_threads[i].get_id()] = i+1;
        }
    }

    // Waits until threads finish their current task and shutdowns the pool
    void shutdown()
    {
        m_shutdown = true;
        m_conditional_lock.notify_all();

        for (std::size_t i = 0; i < m_threads.size(); ++i)
        {
            m_threads[i].join();
        }
    }

    // Submit a function to be executed asynchronously by the pool
    template<typename F, typename...Args>
    auto submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))>
    {
        // Create a function with bounded parameters ready to execute
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        // Encapsulate it into a shared ptr in order to be able to copy construct / assign
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

        // Wrap packaged task into void function
        std::function<void()> wrapper_func = [task_ptr]()
        {
            (*task_ptr)();
        };

        // Enqueue generic wrapper function
        m_queue.enqueue(wrapper_func);

        // Wake up one thread if its waiting
        m_conditional_lock.notify_one();

        // Return future from promise
        return task_ptr->get_future();
    }
};

}
}

#endif // CGAL_DDT_SCHEDULER_THREAD_POOL_H
