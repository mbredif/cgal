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

#ifndef CGAL_DDT_SCHEDULER_H
#define CGAL_DDT_SCHEDULER_H

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

#endif // CGAL_DDT_SCHEDULER_H
