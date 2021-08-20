/* Copyright 2003-2019 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_MEM_FUN_HPP
#define BOOST_MULTI_INDEX_MEM_FUN_HPP
#pragma once

#include <type_traits>

namespace boost::multi_index {

/* mem_fun implements a read-only key extractor based on a given non-const
 * member function of a class.
 * Also, the following variations are provided:
 *   const_mem_fun:    const member functions
 *   volatile_mem_fun: volatile member functions
 *   cv_mem_fun:       const volatile member functions
 *   ref_mem_fun:      ref-qualifed member functions (C++11)
 *   cref_mem_fun:     const ref-qualifed member functions (C++11)
 *   vref_mem_fun:     volatile ref-qualifed member functions (C++11)
 *   cvref_mem_fun:    const volatile ref-qualifed member functions (C++11)
 *
 * All of these classes are overloaded to support std::referece_wrappers
 * of T and "chained pointers" to T's. By chained pointer to T we mean a type
 * P such that, given a p of Type P
 *   *...n...*x is convertible to T&, for some n>=1.
 * Examples of chained pointers are raw and smart pointers, iterators and
 * arbitrary combinations of these (vg. T** or unique_ptr<T*>.)
 */

namespace detail {

template<
    class Class,
    typename Type,
    typename PtrToMemberFunctionType,
    PtrToMemberFunctionType PtrToMemberFunction
>
struct const_mem_fun_impl {
  using result_type = typename std::remove_reference_t<Type>;

  template<typename ChainedPtr>
  typename std::enable_if_t<
    !std::is_convertible_v<const ChainedPtr&, const Class&>,
  Type> operator()(const ChainedPtr& x) const
  { return operator()(*x); }

  Type operator()(const Class& x) const
  { return (x.*PtrToMemberFunction)(); }

  Type operator()(const std::reference_wrapper<const Class>& x) const
  { return operator()(x.get()); }

  Type operator()(const std::reference_wrapper<Class>& x) const
  { return operator()(x.get()); }
}; // const_mem_fun_impl

template<class Class, typename Type,
         typename PtrToMemberFunctionType,
         PtrToMemberFunctionType PtrToMemberFunction>
struct mem_fun_impl {
  using result_type = std::remove_reference_t<Type>;

  template<typename ChainedPtr>
  typename std::enable_if_t<
    !std::is_convertible_v<ChainedPtr&, Class&>,
  Type> operator()(const ChainedPtr& x) const
  { return operator()(*x); }

  Type operator()(Class& x) const
  { return (x.*PtrToMemberFunction)(); }

  Type operator()(const std::reference_wrapper<Class>& x) const
  { return operator()(x.get()); }
}; // mem_fun_impl

} // detail

template<class Class, typename Type,
         Type(Class::*PtrToMemberFunction)() const>
using const_mem_fun = detail::const_mem_fun_impl<Class, Type,
                                                 Type(Class::*)() const,
                                                 PtrToMemberFunction>;
template<class Class, typename Type,
         Type(Class::*PtrToMemberFunction)() const volatile>
using cv_mem_fun = detail::const_mem_fun_impl<Class, Type,
                                              Type(Class::*)() const volatile,
                                              PtrToMemberFunction>;

template<class Class, typename Type, Type(Class::*PtrToMemberFunction)()>
using mem_fun = detail::mem_fun_impl<Class, Type,
                                     Type(Class::*)(),
                                     PtrToMemberFunction>;

template<class Class, typename Type,
         Type(Class::*PtrToMemberFunction)() volatile>
using volatile_mem_fun = detail::mem_fun_impl<Class, Type,
                                              Type(Class::*)() volatile,
                                              PtrToMemberFunction>;

template<class Class, typename Type,
         Type(Class::*PtrToMemberFunction)() const & >
using cref_mem_fun = detail::const_mem_fun_impl<Class, Type,
                                                Type(Class::*)() const &,
                                                PtrToMemberFunction>;

template<class Class, typename Type,
         Type(Class::*PtrToMemberFunction)() const volatile & >
using cvref_mem_fun = detail::const_mem_fun_impl<Class, Type,
                                              Type(Class::*)() const volatile &,
                                              PtrToMemberFunction>;

template<class Class, typename Type, Type(Class::*PtrToMemberFunction)() & >
using ref_mem_fun = detail::mem_fun_impl<Class, Type,
                                         Type(Class::*)() &,
                                         PtrToMemberFunction>;

template<class Class, typename Type,
         Type(Class::*PtrToMemberFunction)() volatile& >
using vref_mem_fun = detail::mem_fun_impl<Class, Type,
                                          Type(Class::*)() volatile &,
                                          PtrToMemberFunction>;

#define BOOST_MULTI_INDEX_CONST_MEM_FUN(Class, Type, MemberFunName) \
::boost::multi_index::const_mem_fun< Class, Type, &Class::MemberFunName>

#define BOOST_MULTI_INDEX_MEM_FUN(Class, Type, MemberFunName) \
::boost::multi_index::mem_fun< Class, Type, &Class::MemberFunName>

} // boost::multi_index

#endif
