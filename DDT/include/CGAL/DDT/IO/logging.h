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

#ifndef CGAL_DDT_LOGGING_H
#define CGAL_DDT_LOGGING_H

#include <chrono>
#include <iostream>

namespace ddt
{

template <int ID=0>
class logging
{
public:
    logging(const std::string& s, int l) : level(l), time(), overall(s)
    {
        time = last = start = std::chrono::system_clock::now();
    }
    ~logging()
    {
      step(overall);
      time = std::chrono::system_clock::now();
      operator()(0, std::chrono::duration<float>(time-start).count(), "\n");
      last = time;
    }
    template<typename...Args>
    void operator()(int l, Args&&... args) const
    {
        if(level>=l) do_log(args...);
    }
    void step(const std::string& s) const
    {
      time = std::chrono::system_clock::now();
      if(last!=start)
      {
          operator()(2, "\n");
          operator()(0, std::chrono::duration<float>(time-last).count(), "\n");
      }
      last = time;
      operator()(0, s, "\t");
    }
    int level;
private:

    void do_log() const
    {
      std::cout << std::flush;
    }
    template<typename Arg, typename...Args>
    void do_log(Arg&& arg, Args&&... args) const
    {
        std::cout << arg;
        do_log(args...);
    }

    mutable std::chrono::time_point<std::chrono::system_clock> time, last, start;
    std::string overall;
};

}

#endif // CGAL_DDT_LOGGING_H
