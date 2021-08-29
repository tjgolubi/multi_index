/* Copyright 2003-2017 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_DETAIL_BASE_TYPE_HPP
#define BOOST_MULTI_INDEX_DETAIL_BASE_TYPE_HPP
#pragma once

#include <boost/multi_index/detail/index_base.hpp>
#include <boost/multi_index/detail/is_index_list.hpp>
#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/list.hpp>
#include <boost/mp11/utility.hpp>
#include <type_traits>

namespace boost::multi_index::detail {

/* Mp11 machinery to construct a linear hierarchy of indices out of
 * an index list.
 */

namespace {

template<typename IndexSpecifierList, typename N, typename SuperMeta>
using nth_layer_index =
    typename mp11::mp_at<IndexSpecifierList, N>::
    template index_class<SuperMeta>::type;
}

template<int N, typename Value, typename IndexSpecifierList, typename Allocator>
struct nth_layer {
  static const int length = mp11::mp_size<IndexSpecifierList>::value;

  typedef mp11::mp_eval_if_c <
  (N == length),
  index_base<Value, IndexSpecifierList, Allocator>,
  nth_layer_index,
  IndexSpecifierList, mp11::mp_int<N>,
  nth_layer < N + 1, Value, IndexSpecifierList, Allocator >
  > type;
}; // nth_layer

template<typename Value, typename IndexSpecifierList, typename Allocator>
struct multi_index_base_type
  : nth_layer<0, Value, IndexSpecifierList, Allocator> {
  static_assert(detail::is_index_list<IndexSpecifierList>::value);
};

} // boost::multi_index::detail

#endif
