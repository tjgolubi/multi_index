/* Copyright 2003-2017 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_MEMBER_HPP
#define BOOST_MULTI_INDEX_MEMBER_HPP
#pragma once

#include <type_traits>
#include <cstddef>

namespace boost::multi_index {

namespace detail {

/* member is a read/write key extractor for accessing a given
 * member of a class.
 * Additionally, member is overloaded to support referece_wrappers
 * of T and "chained pointers" to T's. By chained pointer to T we mean
 * a type P  such that, given a p of Type P
 *   *...n...*x is convertible to T&, for some n>=1.
 * Examples of chained pointers are raw and smart pointers, iterators and
 * arbitrary combinations of these (vg. T** or unique_ptr<T*>.)
 */

template<class Class, typename Type, Type Class::*PtrToMember>
struct const_member_base {
  using result_type = Type;

  template<typename ChainedPtr>
  typename std::enable_if_t<
    !std::is_convertible_v<const ChainedPtr&, const Class&>,
    Type&>
  operator()(const ChainedPtr& x) const  { return operator()(*x); }

  Type& operator()(const Class& x) const { return x.*PtrToMember; }

  Type& operator()(const std::reference_wrapper<const Class>& x) const
  { return operator()(x.get()); }

  Type& operator()(const std::reference_wrapper<Class>& x) const
  { return operator()(x.get()); }
}; // const_member_base

template<class Class, typename Type, Type Class::*PtrToMember>
struct non_const_member_base {
  typedef Type result_type;

  template<typename ChainedPtr>
  typename std::enable_if_t<
    !std::is_convertible_v<const ChainedPtr&, const Class&>,
    Type&>
  operator()(const ChainedPtr& x) const { return operator()(*x); }

  const Type& operator()(const Class& x) const { return x.*PtrToMember; }

  Type& operator()(Class& x) const { return x.*PtrToMember; }

  const Type& operator()(const std::reference_wrapper<const Class>& x) const
  { return operator()(x.get()); }

  Type& operator()(const std::reference_wrapper<Class>& x) const
  { return operator()(x.get()); }
}; // non_const_member_base

} // detail

template<class Class, typename Type, Type Class::*PtrToMember>
struct member:
  std::conditional_t<
    std::is_const_v<Type>,
    detail::const_member_base<Class, Type, PtrToMember>,
    detail::non_const_member_base<Class, Type, PtrToMember>
  >
{ };

#define BOOST_MULTI_INDEX_MEMBER(Class, Type, MemberName) \
::boost::multi_index::member< Class, Type, &Class::MemberName>

} // boost::multi_index

#endif
