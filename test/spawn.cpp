//
// spawn.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2019 Casey Bodley (cbodley at redhat dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Disable autolinking for unit tests.
#if !defined(BOOST_ALL_NO_LIB)
#define BOOST_ALL_NO_LIB 1
#endif // !defined(BOOST_ALL_NO_LIB)

// Test that header file is self-contained.
#include <boost/asio/spawn.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/system_timer.hpp>
#include <boost/context/protected_fixedsize_stack.hpp>
#include "unit_test.hpp"


boost::context::protected_fixedsize_stack with_stack_allocator()
{
  return boost::context::protected_fixedsize_stack(65536);
}

struct counting_handler {
  int& count;
  counting_handler(int& count) : count(count) {}
  void operator()() { ++count; }
  template <typename T>
  void operator()(boost::asio::basic_yield_context<T>) { ++count; }
};

void test_spawn_function()
{
  boost::asio::io_context ioc;
  int called = 0;
  boost::asio::spawn(counting_handler(called));
  BOOST_ASIO_CHECK(0 == ioc.run()); // runs in system executor
  BOOST_ASIO_CHECK(ioc.stopped());
  BOOST_ASIO_CHECK(1 == called);
}

void test_spawn_function_stack_allocator()
{
  boost::asio::io_context ioc;
  int called = 0;
  boost::asio::spawn(counting_handler(called),
                     with_stack_allocator());
  BOOST_ASIO_CHECK(0 == ioc.run());
  BOOST_ASIO_CHECK(ioc.stopped());
  BOOST_ASIO_CHECK(1 == called);
}

void test_spawn_handler()
{
  boost::asio::io_context ioc;
  boost::asio::strand<boost::asio::io_context::executor_type> strand(ioc.get_executor());
  int called = 0;
  boost::asio::spawn(bind_executor(strand, counting_handler(called)),
                     counting_handler(called));
  BOOST_ASIO_CHECK(1 == ioc.run());
  BOOST_ASIO_CHECK(ioc.stopped());
  BOOST_ASIO_CHECK(2 == called);
}

void test_spawn_handler_stack_allocator()
{
  boost::asio::io_context ioc;
  typedef boost::asio::io_context::executor_type executor_type;
  boost::asio::strand<executor_type> strand(ioc.get_executor());
  int called = 0;
  boost::asio::spawn(bind_executor(strand, counting_handler(called)),
                     counting_handler(called),
                     with_stack_allocator());
  BOOST_ASIO_CHECK(1 == ioc.run());
  BOOST_ASIO_CHECK(ioc.stopped());
  BOOST_ASIO_CHECK(2 == called);
}

struct spawn_counting_handler {
  int& count;
  spawn_counting_handler(int& count) : count(count) {}
  template <typename T>
  void operator()(boost::asio::basic_yield_context<T> y)
  {
    boost::asio::spawn(y, counting_handler(count));
    ++count;
  }
};

void test_spawn_yield_context()
{
  boost::asio::io_context ioc;
  int called = 0;
  boost::asio::spawn(bind_executor(ioc.get_executor(),
                                   counting_handler(called)),
                     spawn_counting_handler(called));
  BOOST_ASIO_CHECK(1 == ioc.run());
  BOOST_ASIO_CHECK(ioc.stopped());
  BOOST_ASIO_CHECK(3 == called);
}

struct spawn_alloc_counting_handler {
  int& count;
  spawn_alloc_counting_handler(int& count) : count(count) {}
  template <typename T>
  void operator()(boost::asio::basic_yield_context<T> y)
  {
    boost::asio::spawn(y, counting_handler(count),
                       with_stack_allocator());
    ++count;
  }
};

void test_spawn_yield_context_stack_allocator()
{
  boost::asio::io_context ioc;
  int called = 0;
  boost::asio::spawn(bind_executor(ioc.get_executor(),
                                   counting_handler(called)),
                     spawn_alloc_counting_handler(called));
  BOOST_ASIO_CHECK(1 == ioc.run());
  BOOST_ASIO_CHECK(ioc.stopped());
  BOOST_ASIO_CHECK(3 == called);
}

void test_spawn_executor()
{
  boost::asio::io_context ioc;
  int called = 0;
  boost::asio::spawn(ioc.get_executor(), counting_handler(called));
  BOOST_ASIO_CHECK(1 == ioc.run());
  BOOST_ASIO_CHECK(ioc.stopped());
  BOOST_ASIO_CHECK(1 == called);
}

void test_spawn_executor_stack_allocator()
{
  boost::asio::io_context ioc;
  int called = 0;
  boost::asio::spawn(ioc.get_executor(),
                     counting_handler(called),
                     with_stack_allocator());
  BOOST_ASIO_CHECK(1 == ioc.run());
  BOOST_ASIO_CHECK(ioc.stopped());
  BOOST_ASIO_CHECK(1 == called);
}

void test_spawn_strand()
{
  boost::asio::io_context ioc;
  typedef boost::asio::io_context::executor_type executor_type;
  int called = 0;
  boost::asio::spawn(boost::asio::strand<executor_type>(ioc.get_executor()),
                     counting_handler(called));
  BOOST_ASIO_CHECK(1 == ioc.run());
  BOOST_ASIO_CHECK(ioc.stopped());
  BOOST_ASIO_CHECK(1 == called);
}

void test_spawn_strand_stack_allocator()
{
  boost::asio::io_context ioc;
  typedef boost::asio::io_context::executor_type executor_type;
  int called = 0;
  boost::asio::spawn(boost::asio::strand<executor_type>(ioc.get_executor()),
                     counting_handler(called),
                     with_stack_allocator());
  BOOST_ASIO_CHECK(1 == ioc.run());
  BOOST_ASIO_CHECK(ioc.stopped());
  BOOST_ASIO_CHECK(1 == called);
}

void test_spawn_execution_context()
{
  boost::asio::io_context ioc;
  int called = 0;
  boost::asio::spawn(ioc, counting_handler(called));
  BOOST_ASIO_CHECK(1 == ioc.run());
  BOOST_ASIO_CHECK(ioc.stopped());
  BOOST_ASIO_CHECK(1 == called);
}

void test_spawn_execution_context_stack_allocator()
{
  boost::asio::io_context ioc;
  int called = 0;
  boost::asio::spawn(ioc, counting_handler(called),
                     with_stack_allocator());
  BOOST_ASIO_CHECK(1 == ioc.run());
  BOOST_ASIO_CHECK(ioc.stopped());
  BOOST_ASIO_CHECK(1 == called);
}

typedef boost::asio::system_timer timer_type;

struct spawn_wait_handler {
  timer_type& timer;
  spawn_wait_handler(timer_type& timer) : timer(timer) {}
  template <typename T>
  void operator()(boost::asio::basic_yield_context<T> yield)
  {
    timer.async_wait(yield);
  }
};

void test_spawn_timer()
{
  int called = 0;
  {
    boost::asio::io_context ioc;
    timer_type timer(ioc, boost::asio::chrono::hours(0));

    boost::asio::spawn(bind_executor(ioc.get_executor(),
                                     counting_handler(called)),
                       spawn_wait_handler(timer));
    BOOST_ASIO_CHECK(2 == ioc.run());
    BOOST_ASIO_CHECK(ioc.stopped());
  }
  BOOST_ASIO_CHECK(1 == called);
}

void test_spawn_timer_destruct()
{
  int called = 0;
  {
    boost::asio::io_context ioc;
    timer_type timer(ioc, boost::asio::chrono::hours(65536));

    boost::asio::spawn(bind_executor(ioc.get_executor(),
                                     counting_handler(called)),
                       spawn_wait_handler(timer));
    BOOST_ASIO_CHECK(1 == ioc.run_one());
    BOOST_ASIO_CHECK(!ioc.stopped());
  }
  BOOST_ASIO_CHECK(0 == called);
}

BOOST_ASIO_TEST_SUITE
(
  "spawn",
  BOOST_ASIO_TEST_CASE(test_spawn_function)
  BOOST_ASIO_TEST_CASE(test_spawn_function_stack_allocator)
  BOOST_ASIO_TEST_CASE(test_spawn_handler)
  BOOST_ASIO_TEST_CASE(test_spawn_handler_stack_allocator)
  BOOST_ASIO_TEST_CASE(test_spawn_yield_context)
  BOOST_ASIO_TEST_CASE(test_spawn_yield_context_stack_allocator)
  BOOST_ASIO_TEST_CASE(test_spawn_executor)
  BOOST_ASIO_TEST_CASE(test_spawn_executor_stack_allocator)
  BOOST_ASIO_TEST_CASE(test_spawn_strand)
  BOOST_ASIO_TEST_CASE(test_spawn_strand_stack_allocator)
  BOOST_ASIO_TEST_CASE(test_spawn_execution_context)
  BOOST_ASIO_TEST_CASE(test_spawn_execution_context_stack_allocator)
  BOOST_ASIO_TEST_CASE(test_spawn_timer)
  BOOST_ASIO_TEST_CASE(test_spawn_timer_destruct)
)
