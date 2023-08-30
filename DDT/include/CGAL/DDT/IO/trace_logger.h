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
#ifndef CGAL_DDT_IO_TRACE_LOGGER_H
#define CGAL_DDT_IO_TRACE_LOGGER_H

#ifdef CGAL_DDT_TRACING
#include <tbb/tick_count.h>

#define CGAL_DDT_TRACE_(sch, cat, name, cname, ph, args) \
    do { \
        using std::to_string; \
        std::size_t ts = (sch).clock_microsec(); \
        int tid = (sch).thread_index(); \
        (sch).trace.out \
            << "{\"name\": \"" << name << "\"" \
            << ", \"cat\": \"" << cat << "\"" \
            << ", \"ph\": \"" << ph << "\"" \
            << ", \"pid\": 0" \
            << ", \"tid\": " << tid \
            << ", \"args\": {" << args << "}" \
            << ", \"ts\": " << ts; \
        if (cname) (sch).trace.out << ", \"cname\": \"" << cname << "\""; \
        (sch).trace.out << "},\n"; \
    } while(0)

#define CGAL_DDT_TRACE_ARG(name, value) "\"" #name "\": " << value
#define CGAL_DDT_TRACE_LOCK(sch) std::unique_lock lock((sch).mutex);


namespace CGAL {
namespace DDT {

template<typename clock_type> struct trace_logger {
    clock_type t0;
    std::ofstream out;

    trace_logger(const std::string& filename = "perf.json") : out(filename), t0() {
        out << "[";
    }

    ~trace_logger() {
        out.seekp(out.tellp()-2);
        out << "]";
    }
};

}
}
#else // CGAL_DDT_TRACING
#define CGAL_DDT_TRACE_(sch, cat, name, cname, ph, args)
#define CGAL_DDT_TRACE_ARG(name, value) ""
#define CGAL_DDT_TRACE_LOCK(sch)

#endif // CGAL_DDT_TRACING

#define CGAL_DDT_TRACE0(sch, cat, name, cname, ph) \
        CGAL_DDT_TRACE_(sch, cat, name, cname, ph, "")
#define CGAL_DDT_TRACE1(sch, cat, name, cname, ph, k1, v1) \
        CGAL_DDT_TRACE_(sch, cat, name, cname, ph, CGAL_DDT_TRACE_ARG(k1, v1))
#define CGAL_DDT_TRACE2(sch, cat, name, cname, ph, k1, v1, k2, v2) \
        CGAL_DDT_TRACE_(sch, cat, name, cname, ph, CGAL_DDT_TRACE_ARG(k1, v1) \
                                   << ", " << CGAL_DDT_TRACE_ARG(k2, v2))
#define CGAL_DDT_TRACE0_LOCK(sch, cat, name, cname, ph) do { CGAL_DDT_TRACE_LOCK(sch)  CGAL_DDT_TRACE0(sch, cat, name, cname, ph); } while(0)
#define CGAL_DDT_TRACE1_LOCK(sch, cat, name, cname, ph, k1, v1) do { CGAL_DDT_TRACE_LOCK(sch)  CGAL_DDT_TRACE1(sch, cat, name, cname, ph, k1, v1); } while(0)
#define CGAL_DDT_TRACE2_LOCK(sch, cat, name, cname, ph, k1, v1, k2, v2) do { CGAL_DDT_TRACE_LOCK(sch)  CGAL_DDT_TRACE2(sch, cat, name, cname, ph, k1, v1, k2, v2); } while(0)

namespace CGAL {
namespace DDT {

template<typename T>
std::ostream& write_summary(std::ostream& out, const T& t)
{
    return out << std::to_string(t);
}

template<typename T, typename Alloc>
std::ostream& write_summary(std::ostream& out, const std::vector<T, Alloc>& v)
{
    return out << v.size();
}

template<class T, class U>
std::ostream& write_summary(std::ostream& out, const std::pair<T, U>& p)
{
    return write_summary(write_summary(out << "[", p.first) << ",", p.second) << "]";
}

template<typename Iterator>
std::ostream& write_summary(std::ostream& out, Iterator begin, Iterator end)
{
    out << "[";
    for(Iterator it = begin; it != end; ++it)
        write_summary(out << (it==begin?" ":", "), *it);
    return out << " ]";
}

}
}

#endif // CGAL_DDT_IO_TRACE_LOGGER_H
