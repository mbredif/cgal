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

#ifdef CGAL_LINKED_WITH_TBB

#include <CGAL/DDT/scheduler/TBB_scheduler.h>
namespace CGAL {
namespace DDT {
template <typename T> using Scheduler = CGAL::DDT::TBB_scheduler<T>;
}
}
#else

#include <CGAL/DDT/scheduler/Multithread_scheduler.h>
namespace CGAL {
namespace DDT {
template <typename T> using Scheduler = CGAL::DDT::Multithread_scheduler<T>;
}
}
#endif

#else

#include <CGAL/DDT/scheduler/Sequential_scheduler.h>
namespace CGAL {
namespace DDT {
template <typename T> using Scheduler = CGAL::DDT::Sequential_scheduler<T>;
}
}
#endif

#endif // CGAL_DDT_SCHEDULER_H
