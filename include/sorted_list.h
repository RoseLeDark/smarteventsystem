#pragma once
#include <list>
#include <algorithm>
#include <functional>
#include "sorted.h"

namespace ses {
    template <class T, class TAlloc = std::allocator<T>>
    class sorted_list : public sorted<T> {
    public:
        using base_type = sorted<T>;
        using compare_func_t = typename base_type::compare_func_t;

        using container_type = std::list<T, TAlloc>;
        using iterator = typename container_type::iterator;
        using const_iterator = typename container_type::const_iterator;

        sorted_list(bool auto_sort = true)
            : sorted(auto_sort), m_funcCompare([](const T& a, const T& b) { return a < b; }), m_bIsSorted(auto_sort) {
        }

        void set_handle(compare_func_t comp) {
            m_funcCompare = std::move(comp);
            if (autosort) {
                sort();
            }
        }

        void insert(const T& value) {
            push_back(value);
        }

        void push_back(const T& value) {
            if (autosort) {
                auto it = std::find_if(m_listData.begin(), m_listData.end(),
                    [&](const T& elem) { return m_funcCompare(value, elem); });
                m_listData.insert(it, value);
                m_bIsSorted = true;
            }
            else {
                m_listData.push_back(value);
                m_bIsSorted = false;
            }
        }

        void push_back(T&& value) {
            if (autosort) {
                auto it = std::find_if(m_listData.begin(), m_listData.end(),
                    [&](const T& elem) { return m_funcCompare(value, elem); });
                m_listData.insert(it, std::move(value));
                m_bIsSorted = true;
            }
            else {
                m_listData.push_back(std::move(value));
                m_bIsSorted = false;
            }
        }

        void sort() {
            m_listData.sort(m_funcCompare);
            m_bIsSorted = true;
        }

        void clear() {
            m_listData.clear();
        }

        size_t size() const {
            return m_listData.size();
        }

        bool empty() const {
            return m_listData.empty();
        }

        iterator begin() { return m_listData.begin(); }
        iterator end() { return m_listData.end(); }
        const_iterator begin() const { return m_listData.begin(); }
        const_iterator end() const { return m_listData.end(); }

    private:
        container_type m_listData;
        compare_func_t m_funcCompare;
    };
}
