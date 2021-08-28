/* Copyright 2003-2017 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_IDENTITY_HPP
#define BOOST_MULTI_INDEX_IDENTITY_HPP
#pragma once

#include <boost/multi_index/identity_fwd.hpp>
#include <type_traits>

namespace boost::multi_index {

namespace detail {

/* identity is a do-nothing key extractor that returns the [const] Type&
 * object passed.
 * Additionally, identity is overloaded to support referece_wrappers
 * of Type and "chained pointers" to Type's. By chained pointer to Type we
 * mean a  type  P such that, given a p of type P
 *   *...n...*x is convertible to Type&, for some n>=1.
 * Examples of chained pointers are raw and smart pointers, iterators and
 * arbitrary combinations of these (vg. Type** or unique_ptr<Type*>.)
 */

template<typename Type>
struct const_identity_base {
  using result_type = Type;

  template<typename ChainedPtr>
  typename std::enable_if_t<
    !std::is_convertible_v<const ChainedPtr&, Type&>,
  Type&> operator()(const ChainedPtr& x) const
  { return operator()(*x); }

  Type& operator()(Type& x) const
  { return x; }

  Type& operator()(const std::reference_wrapper<Type>& x) const
  { return x.get(); }

  Type& operator()(const std::reference_wrapper<
                      typename std::remove_const_t<Type>>& x
                  ) const
  { return x.get(); }
}; // const_identity_base

template<typename Type>
struct non_const_identity_base {
  using result_type = Type;

  /* templatized for pointer-like types */

  template<typename ChainedPtr>
  typename std::enable_if_t<
    !std::is_convertible_v<const ChainedPtr&, const Type&>,
    Type&>
  operator()(const ChainedPtr& x) const
  { return operator()(*x); }

  const Type& operator()(const Type& x) const { return x; }

  Type& operator()(Type& x) const { return x; }

  const Type& operator()(const std::reference_wrapper<const Type>& x) const
  { return x.get(); }

  Type& operator()(const std::reference_wrapper<Type>& x) const
  { return x.get(); }
}; // non_const_identity_base

} // detail

template<class Type>
struct identity:
  std::conditional_t<
    std::is_const_v<Type>,
    detail::const_identity_base<Type>, detail::non_const_identity_base<Type>
  >
{ };

} // boost::multi_index

#endif
