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

#ifdef CGAL_DDT_TRACE
#include <tbb/tick_count.h>

#define CGAL_DDT_TRACE_(cat, name, cname, ph, args) \
    do { \
        using std::to_string; \
        tbb::tick_count t1 = tbb::tick_count::now(); \
        std::size_t ts =  1e6*(t1 - trace.t0).seconds(); \
        int tid = tbb::this_task_arena::current_thread_index(); \
        trace.out \
            << "{\"name\": \"" << name << "\"" \
            << ", \"cat\": \"" << cat << "\"" \
            << ", \"ph\": \"" << ph << "\"" \
            << ", \"pid\": 0" \
            << ", \"tid\": " << tid \
            << ", \"args\": {" << args << "}" \
            << ", \"ts\": " << ts; \
        if (cname) trace.out << ", \"cname\": \"" << cname << "\""; \
        trace.out << "},\n"; \
    } while(0)

#define CGAL_DDT_TRACE_ARG(name, value) "\"" #name "\": " << value
#define CGAL_DDT_TRACE_LOCK std::unique_lock lock(mutex);


namespace CGAL {
namespace DDT {

struct trace_logger {
    tbb::tick_count t0;
    std::ofstream out;

    trace_logger(const std::string& filename = "perf.json") : t0(tbb::tick_count::now()), out(filename) {
        out << "[";
    }

    ~trace_logger() {
        out.seekp(out.tellp()-2);
        out << "]";
    }
};

}
}
#else // CGAL_DDT_TRACE
#define CGAL_DDT_TRACE_(cat, name, cname, ph, args)
#define CGAL_DDT_TRACE_ARG(name, value) ""
#define CGAL_DDT_TRACE_LOCK

#endif // CGAL_DDT_TRACE

#define CGAL_DDT_TRACE0(cat, name, cname, ph) \
        CGAL_DDT_TRACE_(cat, name, cname, ph, "")
#define CGAL_DDT_TRACE1(cat, name, cname, ph, k1, v1) \
        CGAL_DDT_TRACE_(cat, name, cname, ph, CGAL_DDT_TRACE_ARG(k1, v1))
#define CGAL_DDT_TRACE2(cat, name, cname, ph, k1, v1, k2, v2) \
        CGAL_DDT_TRACE_(cat, name, cname, ph, CGAL_DDT_TRACE_ARG(k1, v1) \
                                   << ", " << CGAL_DDT_TRACE_ARG(k2, v2))
#define CGAL_DDT_TRACE0_LOCK(cat, name, cname, ph) do { CGAL_DDT_TRACE_LOCK  CGAL_DDT_TRACE0(cat, name, cname, ph); } while(0)
#define CGAL_DDT_TRACE1_LOCK(cat, name, cname, ph, k1, v1) do { CGAL_DDT_TRACE_LOCK  CGAL_DDT_TRACE1(cat, name, cname, ph, k1, v1); } while(0)
#define CGAL_DDT_TRACE2_LOCK(cat, name, cname, ph, k1, v1, k2, v2) do { CGAL_DDT_TRACE_LOCK  CGAL_DDT_TRACE1(cat, name, cname, ph, k1, v1, k2, v2); } while(0)


#endif // CGAL_DDT_IO_TRACE_LOGGER_H
