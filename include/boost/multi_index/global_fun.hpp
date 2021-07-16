/* Copyright 2003-2017 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_GLOBAL_FUN_HPP
#define BOOST_MULTI_INDEX_GLOBAL_FUN_HPP
#pragma once

#include <boost/mp11/utility.hpp>
#include <type_traits>

namespace boost{

namespace multi_index{

namespace detail{

/* global_fun is a read-only key extractor from Value based on a given global
 * (or static member) function with signature:
 *
 *   Type f([const] Value [&]);
 *
 * Additionally, global_fun  and const_global_fun are overloaded to support
 * referece_wrappers of Value and "chained pointers" to Value's. By chained
 * pointer to T we  mean a type P such that, given a p of Type P
 *   *...n...*x is convertible to T&, for some n>=1.
 * Examples of chained pointers are raw and smart pointers, iterators and
 * arbitrary combinations of these (vg. T** or unique_ptr<T*>.)
 */

template<class Value,typename Type,Type (*PtrToFunction)(Value)>
struct const_ref_global_fun_base
{
  typedef typename std::remove_reference_t<Type> result_type;

  template<typename ChainedPtr>
  typename std::enable_if_t<
    !std::is_convertible_v<const ChainedPtr&,Value>,Type>
  operator()(const ChainedPtr& x)const
  {
    return operator()(*x);
  }

  Type operator()(Value x)const
  {
    return PtrToFunction(x);
  }

  Type operator()(
    const std::reference_wrapper<
      typename std::remove_reference_t<Value>>& x)const
  { 
    return operator()(x.get());
  }

  Type operator()(
    const std::reference_wrapper<
      typename std::remove_const_t<
        typename std::remove_reference_t<Value>>>& x
  )const
  { 
    return operator()(x.get());
  }
};

template<class Value,typename Type,Type (*PtrToFunction)(Value)>
struct non_const_ref_global_fun_base
{
  typedef typename std::remove_reference_t<Type> result_type;

  template<typename ChainedPtr>
  typename std::enable_if_t<
    !std::is_convertible_v<ChainedPtr&,Value>,Type>
  operator()(const ChainedPtr& x)const
  {
    return operator()(*x);
  }

  Type operator()(Value x)const
  {
    return PtrToFunction(x);
  }

  Type operator()(
    const std::reference_wrapper<
      typename std::remove_reference_t<Value>>& x)const
  { 
    return operator()(x.get());
  }
};

template<class Value,typename Type,Type (*PtrToFunction)(Value)>
struct non_ref_global_fun_base
{
  typedef typename std::remove_reference_t<Type> result_type;

  template<typename ChainedPtr>
  typename std::enable_if_t<
    !std::is_convertible_v<const ChainedPtr&,const Value&>,Type>
  operator()(const ChainedPtr& x)const
  {
    return operator()(*x);
  }

  Type operator()(const Value& x)const
  {
    return PtrToFunction(x);
  }

  Type operator()(const std::reference_wrapper<const Value>& x)const
  { 
    return operator()(x.get());
  }

  Type operator()(
    const std::reference_wrapper<typename std::remove_const_t<Value>>& x)const
  { 
    return operator()(x.get());
  }
};

} /* namespace multi_index::detail */

template<class Value,typename Type,Type (*PtrToFunction)(Value)>
struct global_fun:
  mp11::mp_if_c<
    std::is_reference_v<Value>,
    mp11::mp_if_c<
      std::is_const_v<typename std::remove_reference_t<Value>>,
      detail::const_ref_global_fun_base<Value,Type,PtrToFunction>,
      detail::non_const_ref_global_fun_base<Value,Type,PtrToFunction>
    >,
    detail::non_ref_global_fun_base<Value,Type,PtrToFunction>
  >
{
};

} /* namespace multi_index */

} /* namespace boost */

#endif
