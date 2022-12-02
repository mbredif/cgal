#include <CGAL/DDT/selector/Minimum_selector.h>
#include <CGAL/DDT/selector/Maximum_selector.h>
#include <CGAL/DDT/selector/Median_selector.h>
#include <iostream>

template<typename T>
bool test_selection(const char *pretty_function, T selected, T expected) {
    if (selected != expected)
        std::cerr << pretty_function << " : test failed, got " <<  selected << ", expected " << expected << std::endl;
    return selected == expected;
}


template<typename S, typename T>
bool test_selector_1(S& selector, T expected) {
    selector.insert(1);
    selector.insert(0);
    selector.insert(1);
    selector.insert(2);
    return test_selection(__PRETTY_FUNCTION__, selector.select(), expected);
}

template<typename S, typename T>
bool test_selector_2(S& selector, T expected) {
    selector.insert(5);
    selector.insert(3);
    selector.insert(3);
    selector.insert(4);
    selector.insert(5);
    return test_selection(__PRETTY_FUNCTION__, selector.select(), expected);
}

template<template <class> class Selector, typename T>
bool test_selector(T expected1, T expected2) {
    Selector<T> selector;
    bool t1 = test_selector_1(selector, expected1);
    selector.clear();
    bool t2 = test_selector_2(selector, expected2);
    return t1 && t2;
}

int main(int, char **)
{
    bool tmin = test_selector<CGAL::DDT::Minimum_selector>(0, 3);
    bool tmed = test_selector<CGAL::DDT::Median_selector>(1, 4);
    bool tmax = test_selector<CGAL::DDT::Maximum_selector>(2, 5);
    return (tmin && tmed && tmax) ? 0 : 1;
}
