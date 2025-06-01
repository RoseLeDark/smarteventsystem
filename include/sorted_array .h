/**
* SES Smart Event System
* (c) 2025 Amber-Sophia Schröck
* This file is copyright under the latest version of the EUPL.
* Please see LICENSE file for your rights under this license.
**/
#pragma once
#include <array>
#include <algorithm>
#include <functional>
#include "sorted.h"

namespace ses {

    template <typename T, std::size_t N>
    class sorted_array : public sorted<T> {
    public:
        using base_type = sorted<T>;
        using compare_func_t = typename base_type::compare_func_t;

        using value_type = T;
        using size_type = std::size_t;
        using iterator = T*;
        using const_iterator = const T*;

        sorted_array(std::array<T, N> data, bool auto_sort = true)
            : sorted(auto_sort), m_data(init), m_funcCompare([](const T& a, const T& b) { return a < b; }) {
            if (auto_sort) {
                sort();
            }
        }

        sorted_array(std::array<T, N> data, compare_func_t func, bool auto_sort = true)
            : sorted(auto_sort), m_data(init), m_funcCompare(func) {
            if (auto_sort) {
                sort();
            }
        }

        void sort() {
            std::sort(m_data.begin(), m_data.end(), m_funcCompare);
        }

        void set_handle(compare_func_t comp) {
            m_funcCompare = std::move(comp);
            if (autosort) {
                sort();
            }
        }

        const T& operator[](size_type i) const { return m_data[i]; }
        T& operator[](size_type i) { return m_data[i]; }

        constexpr size_type size() const { return N; }
        constexpr bool empty() const { return N == 0; }

        iterator begin() { return m_data.begin(); }
        iterator end() { return m_data.end(); }
        const_iterator begin() const { return m_data.begin(); }
        const_iterator end() const { return m_data.end(); }

    private:
        std::array<T, N> m_data;
        compare_func_t m_funcCompare;
    };

}