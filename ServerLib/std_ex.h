#pragma once

#include <algorithm>
#include <vector>
#include <initializer_list>

namespace heap {
    template <std::random_access_iterator it>
    using value_t = typename std::iterator_traits<it>::value_type;

    template <typename T>
    std::vector<T> make_max_heap(std::initializer_list<T> init) {
        std::vector<T> heap{ init.begin(), init.end() };
        make_max_heap(heap.begin(), heap.end());
        return heap;
    }

    template <class Cont>
    void make_max_heap(Cont& cont) {
        using iter = typename Cont::iterator;
        make_max_heap(cont.begin(), cont.end());
    }

    template <class Cont>
    void make_min_heap(Cont& cont) {
        using iter = typename Cont::iterator;
        make_min_heap(cont.begin(), cont.end());
    }

    template <std::random_access_iterator it>
    void make_max_heap(it first, it last) {
        std::make_heap(first, last);
    }

    template <std::random_access_iterator it>
    void make_min_heap(it first, it last) {
        std::make_heap(first, last, std::greater<value_t<it>>{ });
    }

    template <std::random_access_iterator it>
    void push_max_heap(it first, it last) {
        std::push_heap(first, last);
    }

    template <std::random_access_iterator it>
    void push_min_heap(it first, it last) {
        std::push_heap(first, last, std::greater<value_t<it>>{ });
    }

    template <std::random_access_iterator it>
    value_t<it> pop_max_heap(it first, it last) {
        std::pop_heap(first, last); 
        return *std::prev(last);
    }

    template <std::random_access_iterator it>
    value_t<it> pop_min_heap(it first, it last) {
        std::pop_heap(first, last, std::greater<value_t<it>>{ });
        return *std::prev(last);
    }
}