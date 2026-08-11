#pragma once
#include <functional>
#include <sstream>
#include <string_view>
#include <utility>
#include <tuple>
#include "helpers.hpp"


namespace cpasm {
    void _format_inner(
        std::string_view text, 
        std::function<void(char c)> on_char,
        std::function<void(size_t idx)> on_fmt
    );

    template<class Tuple, size_t ...I>
    void _fmt_idx(std::stringstream& out, size_t index, Tuple args, std::index_sequence<I...>) {
        ((
            index == I ? (void)(out << std::get<I>(args)) : void()
        ), ...);
    }

    /**
     * Similar to std::snprintf, except the format string syntax is simpler but less powerful.
     */
    template<class ...T>
    std::string format(std::string_view fmt, T... args) {
        std::stringstream res;

        _format_inner(
            fmt,
            [&](char c) {
                res << c;
            },
            [&](size_t idx) {
                _fmt_idx<std::tuple<T...>>(res, idx, std::make_tuple(args...), std::index_sequence_for<T...>{});
            }
        );

        return res.str();
    }
}


