/*
 * Tests to show usage of the class template thread_queue_c.
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

#include <typeinfo>
#include <deque>
#include <ctime>
#include <thread>
#include <iostream>

#include "thread_queue.hpp"

template<typename C>
static void print_container(const C &c)
{
    for (auto iter = c.begin(); c.end() != iter; ++iter)
    {
        std::cout << " " << *iter;
    }
    std::cout << std::endl;
}

template<typename T, typename C1, typename C2>
static void single_threading_test(void)
{
    thread_queue_c<T, C1> q;

    std::cout << ">>> [" << typeid(q).name() << "] test:" << std::endl;
    std::cout << "Init: empty(): " << q.empty() << ", size(): " << q.size() << std::endl;
    q.push_one(1);
    std::cout << "After push_one(1): empty(): " << q.empty() << ", size(): " << q.size() << std::endl;
    q.push_many({ 2, 3, 4 });
    std::cout << "After push_many({ 2, 3, 4 }): empty(): " << q.empty() << ", size(): " << q.size() << std::endl;

    C1 c11 = q.pop_some(1);

    std::cout << "After pop_some(1): empty(): " << q.empty() << ", size(): " << q.size() << ", pop:";
    print_container(c11);

    C1 c12 = q.pop_some(3);

    std::cout << "After pop_some(3): empty(): " << q.empty() << ", size(): " << q.size() << ", pop:";
    print_container(c12);

    C1 c13 = q.pop_all();

    std::cout << "After pop_all(): empty(): " << q.empty() << ", size(): " << q.size() << ", pop:";
    print_container(c13);

    q.push_many({ 5, 6, 7 });
    std::cout << "After push_many({ 5, 6, 7 }): empty(): " << q.empty() << ", size(): " << q.size() << std::endl;

    C1 c14 = q.pop_all();

    std::cout << "After pop_all(): empty(): " << q.empty() << ", size(): " << q.size() << ", pop:";
    print_container(c14);

    q.push_many_with(C2({ 1, 2, 3, 4, 5, 6 }));
    std::cout << "After push_many_with({ 1, 2, 3, 4, 5, 6 }): empty(): " << q.empty() << ", size(): " << q.size() << std::endl;

    C2 c21 = q.template pop_some_as<C2>((size_t)2);

    std::cout << "After pop_some_as(2): empty(): " << q.empty() << ", size(): " << q.size() << ", pop:";
    print_container(c21);

    C2 c22 = q.template pop_all_as<C2>();

    std::cout << "After pop_all_as(): empty(): " << q.empty() << ", size(): " << q.size() << ", pop:";
    print_container(c22);

    std::cout << std::endl << std::endl;
}

typedef struct test_struct_t
{
    int num;
    char str[8];
} test_struct_t;

typedef threaque_c<test_struct_t, std::list<test_struct_t>> test_queue_t;

#define LOCK_WITH(mutex_ptr)                                    std::lock_guard<std::mutex> lock(*(mutex_ptr))

#define LOCKED_PRINT(mutex_ptr, streamed_statements)            do { \
    std::lock_guard<std::mutex> lock(*(mutex_ptr)); \
    std::cout << __func__ << "(): " << streamed_statements; \
} while (0)

#define SLEEP_FOR(msecs)                                        std::this_thread::sleep_for(std::chrono::milliseconds(msecs))

#define STARTUP_DELAY()                                         SLEEP_FOR(200 + rand() % 400)

static void wait_until_fetching_something(test_queue_t *queue, std::mutex *mutex, int *counter)
{
    STARTUP_DELAY();

    while (true)
    {
        queue->wait(/* timeout_usecs = */200000);

        auto items = queue->pop_some(1);

        LOCK_WITH(mutex);

        if (items.empty())
        {
            std::cout << __func__ << "(): Got nothing..." << std::endl;

            continue;
        }

        for (auto &item : items)
        {
            std::cout << __func__ << "(): num = " << item.num << ", str = " << item.str << std::endl;
        }

        ++*counter;

        break;
    }

    LOCKED_PRINT(mutex, "Finished." << std::endl);
}

static void wait_until_counter_greater_than(test_queue_t *queue, std::mutex *mutex, int *counter, int value)
{
    //STARTUP_DELAY();

    queue->wait_until_required_for_stop(test_queue_t::TIMEOUT_FOREVER, [counter, value]{ return *counter > value; });

    auto items = queue->pop_all();

    LOCK_WITH(mutex);

    if (items.empty())
        std::cout << __func__ << "(): Got nothing..." << std::endl;
    else
    {
        for (auto &item : items)
        {
            std::cout << __func__ << "(): num = " << item.num << ", str = " << item.str << std::endl;
        }
    }

    std::cout << __func__ << "(): Finished." << std::endl;
}

static void push_and_notify(test_queue_t *queue, std::mutex *mutex, int *counter)
{
    int push_count = 0;
    test_struct_t structs[] = {
        { 1, "abc" }
        , { 2, "def" }
        , { 3, "ghi" }
    };

    //SLEEP_FOR(1000);

    queue->push_one(std::move(structs[push_count++]), test_queue_t::NOTIFY_NONE);
    LOCKED_PRINT(mutex, "Pushed: " << push_count << std::endl);

    queue->push_one(std::move(structs[push_count++]), test_queue_t::NOTIFY_ALL);
    LOCKED_PRINT(mutex, "Pushed: " << push_count << std::endl);

    SLEEP_FOR(1000);

    queue->push_one(std::move(structs[push_count++]), test_queue_t::NOTIFY_ALL);
    LOCKED_PRINT(mutex, "Pushed: " << push_count << std::endl << __func__ << "(): " << "Finished." << std::endl);
}

static void multi_threading_test(void)
{
    srand(time(nullptr)); 

    test_queue_t queue;
    std::mutex mutex;
    int counter = 0;
    std::thread t1(wait_until_counter_greater_than, &queue, &mutex, &counter, 0);
    std::thread t2(wait_until_fetching_something, &queue, &mutex, &counter);
    std::thread t3(push_and_notify, &queue, &mutex, &counter);

    t1.join();
    t2.join();
    t3.join();
}

int main(int argc, char **argv)
{
    single_threading_test<int, std::list<int>, std::deque<int>>();
    single_threading_test<float, std::deque<float>, std::vector<float>>();
    single_threading_test<double, std::vector<double>, std::list<double>>();

    multi_threading_test();

    return 0;
}

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2022-04-05, Man Hung-Coeng:
 *  01. Create.
 */

