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


#define CGAL_DDT_TRACE_(sch, cat, name, cname, ph, args) \
    do { \
        using std::to_string; \
        (sch).trace.out \
            << "{\"name\": \"" << name << "\"" \
            << ", \"cat\": \"" << cat << "\"" \
            << ", \"ph\": \"" << ph << "\"" \
            << ", \"pid\": " << (sch).process_index() \
            << ", \"tid\": " << (sch).thread_index() \
            << ", \"args\": {" << args << "}" \
            << ", \"ts\": " << (sch).clock_microsec(); \
        if (cname) (sch).trace.out << ", \"cname\": \"" << cname << "\""; \
        (sch).trace.out << "},\n"; \
    } while(0)

#define CGAL_DDT_TRACE_ARG(name, value) "\"" #name "\": " << value
#define CGAL_DDT_TRACE_LOCK(sch) std::unique_lock lock((sch).mutex);

#include <boost/core/demangle.hpp>

namespace CGAL {
namespace DDT {

template<typename T> struct Type {
    static std::string name;
    static std::string init() {
        // demangle
        std::string demangled = boost::core::demangle(typeid(T).name());
        // remove template parameter lists
        std::string basename;
        int open = 0;
        for(auto c: demangled) switch (c) {
            case '<': ++open; break;
            case '>': --open; break;
            default: if(!open) basename += c;
        }
        // remove everything after the start of the first parameter list, if any, including the parenthesis
        basename = basename.substr(0, basename.find('('));
        // remove the leading namespaces
        basename = basename.substr(basename.rfind(':')+1);
        return basename;
    }
};
template<typename T> std::string Type<T>::name = Type<T>::init();

template<typename clock_type> struct trace_logger {
    clock_type t0;
    std::ofstream out;

    trace_logger(const std::string& filename = "perf.json", clock_type t = {}) : t0(t) {
        if (!filename.empty()) open(filename);
    }

    void open(const std::string& filename) {
        out.open(filename);
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
#define CGAL_DDT_TRACE3(sch, cat, name, cname, ph, k1, v1, k2, v2, k3, v3) \
        CGAL_DDT_TRACE_(sch, cat, name, cname, ph, CGAL_DDT_TRACE_ARG(k1, v1) \
                                        << ", " << CGAL_DDT_TRACE_ARG(k2, v2) \
                                        << ", " << CGAL_DDT_TRACE_ARG(k3, v3))
#define CGAL_DDT_TRACE0_LOCK(sch, cat, name, cname, ph) do { CGAL_DDT_TRACE_LOCK(sch)  CGAL_DDT_TRACE0(sch, cat, name, cname, ph); } while(0)
#define CGAL_DDT_TRACE1_LOCK(sch, cat, name, cname, ph, k1, v1) do { CGAL_DDT_TRACE_LOCK(sch)  CGAL_DDT_TRACE1(sch, cat, name, cname, ph, k1, v1); } while(0)
#define CGAL_DDT_TRACE2_LOCK(sch, cat, name, cname, ph, k1, v1, k2, v2) do { CGAL_DDT_TRACE_LOCK(sch)  CGAL_DDT_TRACE2(sch, cat, name, cname, ph, k1, v1, k2, v2); } while(0)

namespace CGAL {
namespace DDT {

std::ostream& write_summary(std::ostream& out, const std::string& s)
{
    return out << s;
}

std::ostream& write_summary(std::ostream& out, char * const s)
{
    return out << "\"" << s << "\"";
}

std::ostream& write_summary(std::ostream& out, const char * s)
{
    return out << "\"" << s << "\"";
}

template<typename T, typename Alloc>
std::ostream& write_summary(std::ostream& out, const std::vector<T, Alloc>& v)
{
    return out << v.size();
}

template<typename T>
std::ostream& write_summary(std::ostream& out, const T& t)
{
    using std::to_string;
    return out << to_string(t);
}

template<typename T, typename Alloc>
std::ostream& write_summary(std::ostream& out, const std::vector<T, Alloc>& v);
template<class T, class U>
std::ostream& write_summary(std::ostream& out, const std::pair<T, U>& p);


template<class T, class U>
std::ostream& write_summary(std::ostream& out, const std::pair<T, U>& p)
{
    return write_summary(write_summary(out << "\"", p.first) << "\":", p.second);
}

template <typename T> struct to_summary_helper { static constexpr const char *s = "[]"; };
template <typename T, typename U> struct to_summary_helper<std::pair<T,U>> { static constexpr const char *s = "{}"; };

template<typename Iterator>
std::ostream& write_summary(std::ostream& out, Iterator begin, Iterator end)
{
    std::string s = to_summary_helper<typename std::iterator_traits<Iterator>::value_type>::s;
    if (begin == end) return out << s;
    for(Iterator it = begin; it != end; ++it)
        write_summary(out << (it==begin ? s[0] : ','), *it);
    return out << s[1];
}

template<typename Iterator>
std::string to_summary(Iterator begin, Iterator end) {
    std::ostringstream oss;
    write_summary(oss, begin, end);
    return oss.str();
}

template<typename T>
std::string to_summary(const T& t) {
    std::ostringstream oss;
    write_summary(oss, t);
    return oss.str();
}

}
}

#endif // CGAL_DDT_IO_TRACE_LOGGER_H
