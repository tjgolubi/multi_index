/* Copyright 2003-2017 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_HASHED_INDEX_FWD_HPP
#define BOOST_MULTI_INDEX_HASHED_INDEX_FWD_HPP
#pragma once

#include <boost/multi_index/detail/hash_index_args.hpp>

namespace boost::multi_index{

namespace detail{

template<
  typename KeyFromValue,typename Hash,typename Pred,
  typename SuperMeta,typename TagList,typename Category
>
class hashed_index;

template<
  typename KeyFromValue,typename Hash,typename Pred,
  typename SuperMeta,typename TagList,typename Category
>
bool operator==(
  const hashed_index<KeyFromValue,Hash,Pred,SuperMeta,TagList,Category>& x,
  const hashed_index<KeyFromValue,Hash,Pred,SuperMeta,TagList,Category>& y);

template<
  typename KeyFromValue,typename Hash,typename Pred,
  typename SuperMeta,typename TagList,typename Category
>
bool operator!=(
  const hashed_index<KeyFromValue,Hash,Pred,SuperMeta,TagList,Category>& x,
  const hashed_index<KeyFromValue,Hash,Pred,SuperMeta,TagList,Category>& y);

template<
  typename KeyFromValue,typename Hash,typename Pred,
  typename SuperMeta,typename TagList,typename Category
>
void swap(
  hashed_index<KeyFromValue,Hash,Pred,SuperMeta,TagList,Category>& x,
  hashed_index<KeyFromValue,Hash,Pred,SuperMeta,TagList,Category>& y);

} // detail

/* hashed_index specifiers */

template<
  typename Arg1,typename Arg2=void,
  typename Arg3=void,typename Arg4=void
>
struct hashed_unique;

template<
  typename Arg1,typename Arg2=void,
  typename Arg3=void,typename Arg4=void
>
struct hashed_non_unique;

} // boost::multi_index

#endif
