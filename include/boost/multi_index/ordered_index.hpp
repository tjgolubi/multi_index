/* Copyright 2003-2017 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_ORDERED_INDEX_HPP
#define BOOST_MULTI_INDEX_ORDERED_INDEX_HPP
#pragma once

#include <boost/multi_index/ordered_index_fwd.hpp>
#include <boost/multi_index/detail/ord_index_impl.hpp>

namespace boost::multi_index {

namespace detail {

/* no augment policy for plain ordered indices */

struct null_augment_policy {
  template<typename OrderedIndexImpl>
  struct augmented_interface {
    using type = OrderedIndexImpl;
  };

  template<typename OrderedIndexNodeImpl>
  struct augmented_node {
    using type = OrderedIndexNodeImpl;
  };

  template<typename Pointer> static void add(Pointer, Pointer) { }
  template<typename Pointer> static void remove(Pointer, Pointer) { }
  template<typename Pointer> static void copy(Pointer, Pointer) { }
  template<typename Pointer> static void rotate_left(Pointer, Pointer) { }
  template<typename Pointer> static void rotate_right(Pointer, Pointer) { }

#if defined(BOOST_MULTI_INDEX_ENABLE_INVARIANT_CHECKING)
  /* invariant stuff */

  template<typename Pointer> static bool invariant(Pointer) { return true; }

#endif
}; // null_augment_policy

} // detail

/* ordered_index specifiers */

template<typename Arg1, typename Arg2, typename Arg3>
struct ordered_unique {
  using index_args      = typename detail::ordered_index_args<Arg1, Arg2, Arg3>;
  using tag_list_type       = typename index_args::tag_list_type;
  using key_from_value_type = typename index_args::key_from_value_type;
  using compare_type        = typename index_args::compare_type;

  template<typename Super>
  struct node_class {
    using type = detail::ordered_index_node<detail::null_augment_policy, Super>;
  };

  template<typename SuperMeta>
  struct index_class {
    using type = detail::ordered_index<key_from_value_type, compare_type,
                         SuperMeta, tag_list_type, detail::ordered_unique_tag,
                         detail::null_augment_policy>;
  };
}; // ordered_unique

template<typename Arg1, typename Arg2, typename Arg3>
struct ordered_non_unique {
  using index_args          = detail::ordered_index_args<Arg1, Arg2, Arg3>;
  using tag_list_type       = typename index_args::tag_list_type;
  using key_from_value_type = typename index_args::key_from_value_type;
  using compare_type        = typename index_args::compare_type;

  template<typename Super>
  struct node_class {
    using type = detail::ordered_index_node<detail::null_augment_policy, Super>;
  };

  template<typename SuperMeta>
  struct index_class {
    using type = detail::ordered_index<key_from_value_type, compare_type,
                                       SuperMeta, tag_list_type,
                                       detail::ordered_non_unique_tag,
                                       detail::null_augment_policy>;
  };
}; // ordered_non_unique

} // boost::multi_index

#endif
