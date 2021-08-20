/* Copyright 2003-2017 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_TAG_HPP
#define BOOST_MULTI_INDEX_TAG_HPP
#pragma once

#include <boost/multi_index/detail/no_duplicate_tags.hpp>
#include <type_traits>

/* A type list containing types used as tag names for indices in get()
 * functions.
 */

namespace boost::multi_index {

namespace detail {

struct tag_marker {};

template<typename T>
using is_tag = std::is_base_of<tag_marker, T>;

} // detail

template<typename... T>
struct tag : private detail::tag_marker {
  static_assert(detail::no_duplicate_tags<tag>::value);
};

} // boost::multi_index

#endif
