/*
 * Queue class (template) for exchanging data items among threads.
 *
 * Copyright (c) 2022 Man Hung-Coeng <udc577@126.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#ifndef __THREAD_QUEUE_HPP__
#define __THREAD_QUEUE_HPP__

#if __cplusplus < 201103L
#error C++11 or above required!
#endif

#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>

#define THREAD_QUEUE_INNER_LOCK()       std::unique_lock<lock_t> lock(*lock_ptr_)

template <typename T>
class thread_queue_c
{
public: // Types.
    typedef std::mutex                  lock_t;

    typedef std::condition_variable     notifier_t;

    enum
    {
        TIMEOUT_FOREVER = -1
    };

    enum notify_flag_e
    {
        NOTIFY_NONE     = 0
        , NOTIFY_ONE    = 1
        , NOTIFY_ALL    = 2
    };

public: // Constructors, destructor and assignment operator(s).
    thread_queue_c()
        : lock_ptr_(std::make_shared<lock_t>())
        , notifier_ptr_(std::make_shared<notifier_t>())
        , item_count_(0)
    {
    }

    thread_queue_c(const thread_queue_c&) = delete;

    thread_queue_c& operator=(const thread_queue_c&) = delete;

    // TODO: Need movement constructor and assignment operator?

    ~thread_queue_c(){}

public: // Status functions.
    inline bool empty(void) const
    {
        return (0 == item_count_);
    }

    inline size_t size(void) const
    {
        return item_count_;
    }

public: // Abilities.
    size_t push_one(T &&item, notify_flag_e flag = NOTIFY_ONE)
    {
        THREAD_QUEUE_INNER_LOCK();

        data_items_.push(std::move(item));
        ++item_count_;

        notify(flag);

        return 1;
    }

    size_t push_many(std::vector<T> &&items, notify_flag_e flag = NOTIFY_ONE)
    {
        size_t count = items.size();

        if (0 == count)
            return 0;

        THREAD_QUEUE_INNER_LOCK();

        for (size_t i = 0; i < count; ++i)
        {
            data_items_.push(std::move(items[i]));
        }
        item_count_ += count;

        notify(flag);

        return count;
    }

    std::vector<T> pop_some(size_t count, notify_flag_e flag = NOTIFY_NONE)
    {
        std::vector<T> items;

        if (0 == count || 0 == item_count_/* Have a fast glimpse of item_count_ before slower locking. */)
            return items;

        THREAD_QUEUE_INNER_LOCK();

        if (item_count_ > 0) /* Must confirm again after locking whether the queue is empty. */
        {
            for (size_t i = 0; i < count && item_count_ > 0; ++i)
            {
                items.push_back(std::move(data_items_.front()));
                data_items_.pop();
                --item_count_;
            }

            notify(flag);
        }

        return items;
    }

    std::vector<T> pop_all(notify_flag_e flag = NOTIFY_NONE)
    {
        std::vector<T> items;

        if (0 == item_count_) /* Have a fast glimpse of item_count_ before slower locking. */
            return items;

        THREAD_QUEUE_INNER_LOCK();

        if (item_count_ > 0) /* Must confirm again after locking whether the queue is empty. */
        {
            for (size_t i = 0; i < item_count_; ++i)
            {
                items.push_back(std::move(data_items_.front()));
                data_items_.pop();
            }
            item_count_ = 0;

            notify(flag);
        }

        return items;
    }

    inline void wait(int timeout_usecs = TIMEOUT_FOREVER)
    {
        THREAD_QUEUE_INNER_LOCK();

        if (timeout_usecs < 0)
            notifier_ptr_->wait(lock);
        else
            notifier_ptr_->wait_for(lock, std::chrono::microseconds(timeout_usecs));
    }

    inline void notify(notify_flag_e flag)
    {
        if (NOTIFY_NONE == flag)
            return;

        if (NOTIFY_ONE == flag)
            notifier_ptr_->notify_one();
        else
            notifier_ptr_->notify_all();
    }

private: // Data for implementation.
    std::shared_ptr<lock_t>         lock_ptr_;
    std::shared_ptr<notifier_t>     notifier_ptr_;
    std::queue<T>                   data_items_;
    std::atomic_size_t              item_count_;
};

template <typename T> using threaque_c = thread_queue_c<T>;

#endif /* #ifndef __THREAD_QUEUE_HPP__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2022-03-15, Man Hung-Coeng:
 *  01. Create.
 */

