#ifndef DDT_SCHEDULER_HPP
#define DDT_SCHEDULER_HPP

#if DDT_USE_THREADS

#include <CGAL/DDT/scheduler/multithread_scheduler.h>
namespace ddt
{
template <typename T> using Scheduler = ddt::multithread_scheduler<T>;
}

#else

#include <CGAL/DDT/scheduler/sequential_scheduler.h>
namespace ddt
{
template <typename T> using Scheduler = ddt::sequential_scheduler<T>;
}

#endif

#endif // DDT_SCHEDULER_HPP
