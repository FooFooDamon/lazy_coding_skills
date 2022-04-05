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
#include <list>
#include <vector>

#ifndef __FUNCTION__
#define __FUNCTION__                        __func__
#endif

#if defined(THREAD_QUEUE_DEBUG) || defined(TEST)
#define THREAD_QUEUE_DPRINT(format, ...)    printf("[DEBUG] " __FILE__ ":%d: %s(): " format, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define THREAD_QUEUE_DPRINT(format, ...)
#endif

#define THREAD_QUEUE_INNER_LOCK()           std::unique_lock<lock_t> lock(*lock_ptr_)

template<typename seq_container_t>
inline void __reserve_memory_if_needed(size_t item_count, seq_container_t &container)
{
}

template<typename T>
inline void __reserve_memory_if_needed(size_t item_count, std::vector<T> &container)
{
    size_t needed_size = container.size() + item_count;
    bool should_reserve = (needed_size > container.capacity() + 1);

    THREAD_QUEUE_DPRINT("Whether to reserve memory of %zu items for vector: %d\n", item_count, should_reserve);

    if (should_reserve)
        container.reserve(needed_size);
}

template<typename T1, typename T2>
inline void __general_push_back(size_t count, T1 &from, T2 &to)
{
    auto iter_begin = from.begin();
    auto iter_end = iter_begin;

    std::advance(iter_end, count);

    __reserve_memory_if_needed(count, to);

    for (auto iter = iter_begin; iter_end != iter; ++iter)
    {
        to.push_back(std::move(*iter));
    }

    from.erase(iter_begin, iter_end);

    THREAD_QUEUE_DPRINT("%s\n", "General and possibly slower push_back()s.");
}

template<typename T>
inline void __general_push_back(size_t count, std::list<T> &from, std::list<T> &to)
{
    auto iter_begin = from.begin();
    auto iter_end = iter_begin;

    std::advance(iter_end, count);

    to.splice(to.end(), from, iter_begin, iter_end);
}

template<typename seq_container_t>
inline void __splice_back(seq_container_t &&from, seq_container_t &to)
{
#if 0 /* TODO: Which is faster, insert() with iterators of a lvalue or push_back()s with rvalues? */
    to.insert(to.end(), from.begin(), from.end());
    THREAD_QUEUE_DPRINT("Used insert().\n");
    to.swap(seq_container_t()); // Totally release memory.
#else
    __general_push_back(from.size(), from, to);
    THREAD_QUEUE_DPRINT("%s\n", "Used push_back()s.");
#endif
}

template<typename T>
inline void __splice_back(std::list<T> &&from, std::list<T> &to)
{
    to.splice(to.end(), std::move(from));
}

template<typename T, typename seq_container_t = std::list<T>/* or std::deque<T>, std::vector<T> */>
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

        data_items_.push_back(std::move(item));

        ++item_count_;

        notify(flag);

        return 1;
    }

    size_t push_many(seq_container_t &&items, notify_flag_e flag = NOTIFY_ONE)
    {
        size_t count = items.size();

        if (0 == count)
            return 0;

        THREAD_QUEUE_INNER_LOCK();

        if (0 == item_count_)
        {
            data_items_.swap(items);
            THREAD_QUEUE_DPRINT("%s\n", "Called swap().");
        }
        else
        {
            __splice_back(std::move(items), data_items_);
        }

        item_count_ += count;

        notify(flag);

        return count;
    }

    template<typename diff_seq_container_t>
    size_t push_many_with(diff_seq_container_t &&items, notify_flag_e flag = NOTIFY_ONE)
    {
        size_t count = items.size();

        if (0 == count)
            return 0;

        THREAD_QUEUE_INNER_LOCK();

        __general_push_back(count, items, data_items_);

        item_count_ += count;

        notify(flag);

        return count;
    }

    seq_container_t pop_some(size_t count, notify_flag_e flag = NOTIFY_NONE)
    {
        seq_container_t items;

        if (0 == item_count_/* Have a fast glimpse of item_count_ before slower locking. */ || 0 == count)
            return items;

        THREAD_QUEUE_INNER_LOCK();

        if (item_count_ > 0) /* Must confirm again after locking whether the queue is empty. */
        {
            if (count == item_count_)
            {
                data_items_.swap(items);
                item_count_ = 0;
                THREAD_QUEUE_DPRINT("%s\n", "Called swap().");
            }
            else
            {
                if (count > item_count_)
                    count = item_count_;

                __general_push_back(count, data_items_, items);

                item_count_ -= count;
            }

            notify(flag);
        }

        return items;
    }

    template<typename diff_seq_container_t>
    diff_seq_container_t pop_some_as(size_t count, notify_flag_e flag = NOTIFY_NONE)
    {
        return this->__pop_as<diff_seq_container_t>(/* all = */false, count, flag);
    }

    seq_container_t pop_all(notify_flag_e flag = NOTIFY_NONE)
    {
        seq_container_t items;

        if (0 == item_count_) /* Have a fast glimpse of item_count_ before slower locking. */
            return items;

        THREAD_QUEUE_INNER_LOCK();

        if (item_count_ > 0) /* Must confirm again after locking whether the queue is empty. */
        {
            data_items_.swap(items);

            item_count_ = 0;

            notify(flag);
        }

        return items;
    }

    template<typename diff_seq_container_t>
    diff_seq_container_t pop_all_as(notify_flag_e flag = NOTIFY_NONE)
    {
        return this->__pop_as<diff_seq_container_t>(/* all = */true, item_count_, flag);
    }

    inline void wait(int timeout_usecs = TIMEOUT_FOREVER)
    {
#if 0 // TODO: Should call the other wait() directly?
        this->wait(timeout_usecs, []{ return false; });
#else
        THREAD_QUEUE_INNER_LOCK();

        if (timeout_usecs < 0)
            notifier_ptr_->wait(lock);
        else
            notifier_ptr_->wait_for(lock, std::chrono::microseconds(timeout_usecs));
#endif
    }

    template<typename bool_function_t/* the so-called "predicate" in somewhere else */>
    inline void wait(int timeout_usecs, bool_function_t should_abort)
    {
        THREAD_QUEUE_INNER_LOCK();

        if (timeout_usecs < 0)
            notifier_ptr_->wait(lock, should_abort);
        else
            notifier_ptr_->wait_for(lock, std::chrono::microseconds(timeout_usecs), should_abort);
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

private: // Inner methods.

    template<typename diff_seq_container_t>
    inline diff_seq_container_t __pop_as(bool all, size_t count_if_not_all, notify_flag_e flag = NOTIFY_NONE)
    {
        diff_seq_container_t items;

        if (0 == item_count_/* Have a fast glimpse of item_count_ before slower locking. */
            || ((!all) && 0 == count_if_not_all))
            return items;

        THREAD_QUEUE_INNER_LOCK();

        if (item_count_ > 0) /* Must confirm again after locking whether the queue is empty. */
        {
            if (all || count_if_not_all > item_count_)
                count_if_not_all = item_count_;

            __general_push_back(count_if_not_all, data_items_, items);

            item_count_ -= count_if_not_all;

            notify(flag);
        }

        return items;
    }

private: // Data for implementation.
    std::shared_ptr<lock_t>         lock_ptr_;
    std::shared_ptr<notifier_t>     notifier_ptr_;
    seq_container_t                 data_items_;
    std::atomic_size_t              item_count_;
};

template<typename T> using threaque_c = thread_queue_c<T>;

#endif /* #ifndef __THREAD_QUEUE_HPP__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2022-03-15, Man Hung-Coeng:
 *  01. Create.
 *
 * >>> 2022-04-04, Man Hung-Coeng:
 *  01. Enhance the queue class template with a second typename parameter
 *      to support customization of underlying container.
 *  02. Add 3 member functions: push_many_with(), pop_some_as() and pop_all_as().
 *
 * >>> 2022-04-05, Man Hung-Coeng:
 *  01. Add an overload of wait() with a predicate parameter.
 *  02. Rename the inner member function __pop_some_as() to __pop_as().
 */

