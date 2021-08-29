/* Copyright 2003-2017 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_DETAIL_ORD_INDEX_ARGS_HPP
#define BOOST_MULTI_INDEX_DETAIL_ORD_INDEX_ARGS_HPP
#pragma once

#include <boost/multi_index/tag.hpp>
#include <boost/mp11/utility.hpp>
#include <functional>
#include <type_traits>

namespace boost::multi_index::detail {

/* Oredered index specifiers can be instantiated in two forms:
 *
 *   (ordered_unique|ordered_non_unique)<
 *     KeyFromValue,Compare=std::less<KeyFromValue::result_type> >
 *   (ordered_unique|ordered_non_unique)<
 *     TagList,KeyFromValue,Compare=std::less<KeyFromValue::result_type> >
 *
 * index_args implements the machinery to accept this argument-dependent
 * polymorphism.
 */

template<typename Arg1, typename Arg2, typename Arg3>
struct ordered_index_args {
  typedef is_tag<Arg1> full_form;

  typedef mp11::mp_if <
  full_form,
  Arg1,
  tag< > >                                     tag_list_type;
  typedef mp11::mp_if <
  full_form,
  Arg2,
  Arg1 >                                        key_from_value_type;
  typedef mp11::mp_if <
  full_form,
  Arg3,
  Arg2 >                                        supplied_compare_type;
  typedef mp11::mp_eval_if_c <
  !std::is_void_v<supplied_compare_type>,
  supplied_compare_type,
  std::less,
  typename key_from_value_type::result_type
  >                                              compare_type;

  static_assert(is_tag<tag_list_type>::value);
  static_assert(!std::is_void_v<key_from_value_type>);
  static_assert(!std::is_void_v<compare_type>);
};

} // boost::multi_index::detail

#endif
