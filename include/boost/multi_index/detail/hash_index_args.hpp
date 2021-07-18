/* Copyright 2003-2017 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_DETAIL_HASH_INDEX_ARGS_HPP
#define BOOST_MULTI_INDEX_DETAIL_HASH_INDEX_ARGS_HPP
#pragma once

#include <boost/multi_index/tag.hpp>
#include <boost/mp11/utility.hpp>
#include <functional>
#include <type_traits>

namespace boost::multi_index::detail{

/* Hashed index specifiers can be instantiated in two forms:
 *
 *   (hashed_unique|hashed_non_unique)<
 *     KeyFromValue,
 *     Hash=std::hash<KeyFromValue::result_type>,
 *     Pred=std::equal_to<KeyFromValue::result_type> >
 *   (hashed_unique|hashed_non_unique)<
 *     TagList,
 *     KeyFromValue,
 *     Hash=std::hash<KeyFromValue::result_type>,
 *     Pred=std::equal_to<KeyFromValue::result_type> >
 *
 * hashed_index_args implements the machinery to accept this
 * argument-dependent polymorphism.
 */

template<typename Arg1,typename Arg2,typename Arg3,typename Arg4>
struct hashed_index_args
{
  typedef is_tag<Arg1> full_form;

  typedef mp11::mp_if<
    full_form,
    Arg1,
    tag< > >                                  tag_list_type;
  typedef mp11::mp_if<
    full_form,
    Arg2,
    Arg1>                                     key_from_value_type;
  typedef mp11::mp_if<
    full_form,
    Arg3,
    Arg2>                                     supplied_hash_type;
  typedef mp11::mp_eval_if_c<
    !std::is_void_v<supplied_hash_type>,
    supplied_hash_type,
    std::hash,
    typename key_from_value_type::result_type
  >                                           hash_type;
  typedef mp11::mp_if<
    full_form,
    Arg4,
    Arg3>                                     supplied_pred_type;
  typedef mp11::mp_eval_if_c<
    !std::is_void_v<supplied_pred_type>,
    supplied_pred_type,
    std::equal_to,
    typename key_from_value_type::result_type
  >                                           pred_type;

  static_assert(is_tag<tag_list_type>::value);
  static_assert(!std::is_void_v<key_from_value_type>);
  static_assert(!std::is_void_v<hash_type>);
  static_assert(!std::is_void_v<pred_type>);
};

} // boost::multi_index::detail

#endif
