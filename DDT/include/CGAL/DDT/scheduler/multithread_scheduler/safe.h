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

#ifndef CGAL_DDT_SCHEDULER_SAFE_H
#define CGAL_DDT_SCHEDULER_SAFE_H

#include <mutex>

namespace ddt {

// Thread safe implementation of a Queue using a std::queue
template <typename Container>
class safe
{
private:
    Container m_container;
    mutable std::mutex m_mutex;
public:
    typedef typename Container::value_type value_type;
    typedef typename Container::size_type size_type;

    safe() {}

    safe(const safe& other) = delete;

    ~safe() {}

    bool empty() const
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_container.empty();
    }

    size_type size() const
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_container.size();
    }

    void enqueue(value_type& t)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_container.push(t);
    }

    bool dequeue(value_type& t)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_container.empty()) return false;
        t = std::move(m_container.front());
        m_container.pop();
        return true;
    }

    template< class... Args >
    void emplace_back( Args&&... args )
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_container.emplace_back(std::forward<Args>(args)...);
    }

    void swap( Container& container )
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_container.swap(container);
    }

    void append( const Container& container )
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_container.insert(m_container.end(), container.begin(), container.end());
    }
};

}

#endif // CGAL_DDT_SCHEDULER_SAFE_H
