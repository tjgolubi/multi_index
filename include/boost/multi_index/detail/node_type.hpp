/* Copyright 2003-2017 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_DETAIL_NODE_TYPE_HPP
#define BOOST_MULTI_INDEX_DETAIL_NODE_TYPE_HPP
#pragma once

#include <boost/multi_index_container_fwd.hpp>
#include <boost/multi_index/detail/header_holder.hpp>
#include <boost/multi_index/detail/index_node_base.hpp>
#include <boost/multi_index/detail/is_index_list.hpp>
#include <boost/mp11/algorithm.hpp>

namespace boost::multi_index::detail {

/* MPL machinery to construct the internal node type associated to an
 * index list.
 */

template<typename IndexSpecifier, typename Super>
using node_type = typename IndexSpecifier::template node_class<Super>::type;

template<typename Value, typename IndexSpecifierList, typename Allocator>
struct multi_index_node_type {
  static_assert(detail::is_index_list<IndexSpecifierList>::value);

  typedef mp11::mp_reverse_fold <
  IndexSpecifierList,
  index_node_base<Value, Allocator>,
  node_type
  > type;
};

} // boost::multi_index::detail

#endif
