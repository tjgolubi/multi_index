/* Copyright 2003-2020 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_DETAIL_ALLOCATOR_TRAITS_HPP
#define BOOST_MULTI_INDEX_DETAIL_ALLOCATOR_TRAITS_HPP
#pragma once

#include <type_traits>
#include <memory>

namespace boost::multi_index::detail{

template<typename T> struct void_helper{typedef void type;};

template<typename Allocator,typename=void>
struct allocator_is_always_equal:std::is_empty<Allocator>{};

template<typename Allocator>
struct allocator_is_always_equal<
  Allocator,
  typename void_helper<
    typename std::allocator_traits<Allocator>::is_always_equal
  >::type
>:std::allocator_traits<Allocator>::is_always_equal{};

template<typename Allocator>
struct allocator_traits:std::allocator_traits<Allocator>
{
  /* wrap std::allocator_traits alias templates for use in C++03 codebase */

  typedef std::allocator_traits<Allocator> super;

  /* pre-C++17 compatibilty */

  typedef allocator_is_always_equal<Allocator> is_always_equal;

  template<typename T>
  struct rebind_alloc
  {
    typedef typename super::template rebind_alloc<T> type;
  };

  template<typename T>
  struct rebind_traits
  {
    typedef typename super::template rebind_traits<T> type;
  };
};

template<typename Allocator,typename T>
struct rebind_alloc_for
{
  typedef typename allocator_traits<Allocator>::
    template rebind_alloc<T>::type               type;
};

} // boost::multi_index::detail

#endif
