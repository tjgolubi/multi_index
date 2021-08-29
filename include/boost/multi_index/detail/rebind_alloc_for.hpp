/* Copyright 2003-2020 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_DETAIL_REBIND_ALLOC_FOR_HPP
#define BOOST_MULTI_INDEX_DETAIL_REBIND_ALLOC_FOR_HPP
#pragma once

#include <type_traits>
#include <memory>

namespace boost::multi_index::detail {

template<typename Allocator, typename T>
struct rebind_alloc_for {
  using type =
      typename std::allocator_traits<Allocator>::template rebind_alloc<T>;
};

} // boost::multi_index::detail

#endif
