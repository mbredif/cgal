#include <CGAL/DDT/selector/Minimum_selector.h>
#include <CGAL/DDT/selector/Maximum_selector.h>
#include <CGAL/DDT/selector/Median_selector.h>

template<template <class> class Selector, typename T>
void test_selector(T expected1, T expected2) {
    Selector<T> selector;
    selector.insert(1);
    selector.insert(0);
    selector.insert(1);
    selector.insert(2);
    assert(*selector == expected1);

    selector.clear();
    selector.insert(5);
    selector.insert(3);
    selector.insert(3);
    selector.insert(4);
    selector.insert(5);
    assert(*selector == expected2);
}


int main(int, char **)
{
    test_selector<CGAL::DDT::Minimum_selector>(0, 3);
    test_selector<CGAL::DDT::Median_selector>(1, 4);
    test_selector<CGAL::DDT::Maximum_selector>(2, 5);
    return 0;
}
