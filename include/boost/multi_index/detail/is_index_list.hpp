/* Copyright 2003-2017 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_DETAIL_IS_INDEX_LIST_HPP
#define BOOST_MULTI_INDEX_DETAIL_IS_INDEX_LIST_HPP
#pragma once

#include <type_traits>

namespace boost::multi_index::detail {

template<typename T>
struct is_index_list: std::false_type {};

template<template<class...> class L, typename T0, typename... T>
struct is_index_list<L<T0, T...>>: std::true_type {};

} // boost::multi_index::detail

#endif
