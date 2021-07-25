/* Copyright 2003-2019 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_KEY_HPP
#define BOOST_MULTI_INDEX_KEY_HPP
#pragma once

#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/global_fun.hpp>
#include <boost/multi_index/mem_fun.hpp>

#define BOOST_MULTI_INDEX_KEY_SUPPORTED

#include <boost/preprocessor/facilities/empty.hpp>
#include <type_traits>

namespace boost::multi_index {

/* C++17 terse key specification syntax */

namespace detail {

template<typename T, T, typename = void>
struct typed_key_impl;

template<typename Class, typename Type, Type Class::*PtrToMember>
struct typed_key_impl <
    Type Class::*,
    PtrToMember,
    typename std::enable_if_t< !std::is_function_v<Type> >
  >
{
  using value_type = Class;
  using type = member<Class, Type, PtrToMember>;
}; // typed_key_impl

#define BOOST_MULTI_INDEX_KEY_TYPED_KEY_IMPL(qualifier, extractor)             \
template<                                                                      \
  typename Class, typename Type, Type (Class::*PtrToMemberFunction)()qualifier \
>                                                                              \
struct typed_key_impl<Type (Class::*)()qualifier, PtrToMemberFunction>         \
{                                                                              \
  using value_type = Class;                                                    \
  using type = extractor<Class, Type, PtrToMemberFunction>;                    \
}; // typed_key_impl

BOOST_MULTI_INDEX_KEY_TYPED_KEY_IMPL(, mem_fun)
BOOST_MULTI_INDEX_KEY_TYPED_KEY_IMPL(const, const_mem_fun)
BOOST_MULTI_INDEX_KEY_TYPED_KEY_IMPL(volatile, volatile_mem_fun)
BOOST_MULTI_INDEX_KEY_TYPED_KEY_IMPL(const volatile, cv_mem_fun)
BOOST_MULTI_INDEX_KEY_TYPED_KEY_IMPL(&, ref_mem_fun)
BOOST_MULTI_INDEX_KEY_TYPED_KEY_IMPL(const&, cref_mem_fun)
BOOST_MULTI_INDEX_KEY_TYPED_KEY_IMPL(volatile&, vref_mem_fun)
BOOST_MULTI_INDEX_KEY_TYPED_KEY_IMPL(const volatile&, cvref_mem_fun)

#undef BOOST_MULTI_INDEX_KEY_TYPED_KEY_IMPL

template<class Value, typename Type, Type(*PtrToFunction)(Value)>
struct typed_key_impl<Type(*)(Value), PtrToFunction> {
  using value_type = Value;
  using type = global_fun<Value, Type, PtrToFunction>;
}; // typed_key_impl

template<typename T>
struct remove_noexcept {
  using type = T;
};

#define BOOST_MULTI_INDEX_KEY_REMOVE_MEMFUN_NOEXCEPT(qualifier)  \
template<typename R, typename C, typename... Args>               \
struct remove_noexcept<R(C::*)(Args...) qualifier noexcept>      \
{ using type = R(C::*)(Args...) qualifier; };                    \
                                                                 \
template<typename R, typename C, typename... Args>               \
struct remove_noexcept<R(C::*)(Args..., ...) qualifier noexcept> \
{ using type = R(C::*)(Args..., ...) qualifier; };

BOOST_MULTI_INDEX_KEY_REMOVE_MEMFUN_NOEXCEPT(BOOST_PP_EMPTY())
/* VS warns without dummy arg */
BOOST_MULTI_INDEX_KEY_REMOVE_MEMFUN_NOEXCEPT(const)
BOOST_MULTI_INDEX_KEY_REMOVE_MEMFUN_NOEXCEPT(volatile)
BOOST_MULTI_INDEX_KEY_REMOVE_MEMFUN_NOEXCEPT(const volatile)
BOOST_MULTI_INDEX_KEY_REMOVE_MEMFUN_NOEXCEPT(&)
BOOST_MULTI_INDEX_KEY_REMOVE_MEMFUN_NOEXCEPT(const&)
BOOST_MULTI_INDEX_KEY_REMOVE_MEMFUN_NOEXCEPT(volatile&)
BOOST_MULTI_INDEX_KEY_REMOVE_MEMFUN_NOEXCEPT(const volatile&)
BOOST_MULTI_INDEX_KEY_REMOVE_MEMFUN_NOEXCEPT(&&)
BOOST_MULTI_INDEX_KEY_REMOVE_MEMFUN_NOEXCEPT(const&&)
BOOST_MULTI_INDEX_KEY_REMOVE_MEMFUN_NOEXCEPT(volatile&&)
BOOST_MULTI_INDEX_KEY_REMOVE_MEMFUN_NOEXCEPT(const volatile&&)

#undef BOOST_MULTI_INDEX_KEY_REMOVE_MEMFUN_NOEXCEPT

template<typename R, typename... Args>
struct remove_noexcept<R(*)(Args...) noexcept> {
  using type = R(*)(Args...);
};

template<typename R, typename... Args>
struct remove_noexcept<R(*)(Args..., ...) noexcept> {
  using type = R(*)(Args..., ...);
};

template<typename T>
using remove_noexcept_t = typename remove_noexcept<T>::type;

template<auto... Keys> struct key_impl;

template<auto Key>
struct key_impl<Key>
  : typed_key_impl<remove_noexcept_t<decltype(Key)>, Key> {};

template<typename... Ts> struct least_generic;

template<typename T0, typename... Ts>
struct least_generic<T0, Ts...> {
  using type = T0;
};

template<typename T0, typename T1, typename... Ts>
struct least_generic<T0, T1, Ts...> {
  static_assert(   std::is_convertible_v<const T0&, const T1&>
                || std::is_convertible_v<const T1&, const T0&>,
                "one type should be convertible to the other");

  using type = typename least_generic<
                  typename std::conditional_t<
                    std::is_convertible_v<const T0&, const T1&>, T0, T1
                  >,
                  Ts...
               >::type;
}; // least_generic

template<auto Key0, auto... Keys>
struct key_impl<Key0, Keys...> {
  using value_type =
      typename least_generic<
        typename std::decay_t<typename key_impl<Key0>::value_type>,
        typename std::decay_t<typename key_impl<Keys>::value_type>...
      >::type;

  using type = composite_key<value_type,
                             typename key_impl<Key0>::type,
                             typename key_impl<Keys>::type...>;
}; // key_impl

template<auto... Keys>
struct limited_size_key_impl {
  static_assert((sizeof...(Keys) <= cons_tuple_size), /// @todo
                "specified number of keys must meet the limits of "
                "boost::multi_index::composite_key");
  using type = typename key_impl<Keys...>::type;
}; // limited_size_key_impl

} // detail

template<auto... Keys>
using key = typename detail::limited_size_key_impl<Keys...>::type;

} // boost::multi_index

#endif
