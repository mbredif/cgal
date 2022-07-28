#ifndef DDT_DATA_HPP
#define DDT_DATA_HPP

#include <iostream>
#include <CGAL/IO/io.h>

namespace ddt
{

template<typename I, typename F>
struct Data
{
    typedef I Id;
    typedef F Flag;
    Data(Id i=0, Flag f=0) : id(i), flag(f) {}

    void write(std::ostream& os) const
    {
        if(CGAL::is_ascii(os))
        {
            os << " " << id << " "  << flag;
        }
        else
        {
            os.write((char*)(&(id)), sizeof(id));
            os.write((char*)(&(flag)), sizeof(flag));
        }
    }
    void read(std::istream& is)
    {
        if(CGAL::is_ascii(is))
        {
            is >> id >> flag;
        }
        else
        {
            is.read((char*)(&(id)), sizeof(id));
            is.read((char*)(&(flag)), sizeof(flag));
        }
    }
    Id id;
    mutable Flag flag;
};


template< typename I, typename F>
std::ostream &
operator<<(std::ostream & os, const Data<I,F> & dd)
{
    dd.write(os);
    return os;
}

template< typename I, typename F>
std::istream &
operator>>(std::istream & is,  Data<I,F> & dd)
{
    dd.read(is);
    return is;
}


}

#endif // DDT_DATA_HPP
