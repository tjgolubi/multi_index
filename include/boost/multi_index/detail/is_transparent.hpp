/* Copyright 2003-2017 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_DETAIL_IS_TRANSPARENT_HPP
#define BOOST_MULTI_INDEX_DETAIL_IS_TRANSPARENT_HPP
#pragma once

#include <boost/mp11/function.hpp>
#include <type_traits>

namespace boost::multi_index::detail{

/* Metafunction that checks if f(arg,arg2) executes without argument type
 * conversion. By default (i.e. when it cannot be determined) it evaluates to
 * true.
 */

template<typename F,typename Arg1,typename Arg2,typename=void>
struct is_transparent:std::true_type{};

struct not_is_transparent_result_type{};

template<typename F,typename Arg1,typename Arg2>
struct is_transparent_class_helper:F
{
  using F::operator();
  template<typename T,typename Q>
  not_is_transparent_result_type operator()(const T&,const Q&)const;
};

template<typename F,typename Arg1,typename Arg2,typename=void>
struct is_transparent_class:std::true_type{};

template<typename F,typename Arg1,typename Arg2>
struct is_transparent_class<
  F,Arg1,Arg2,
  typename std::enable_if<
    std::is_same_v<
      decltype(
        std::declval<const is_transparent_class_helper<F,Arg1,Arg2> >()(
          std::declval<const Arg1&>(),std::declval<const Arg2&>())
      ),
      not_is_transparent_result_type
    >
  >::type
>:std::false_type{};

template<typename F,typename Arg1,typename Arg2>
struct is_transparent<
  F,Arg1,Arg2,
  typename std::enable_if<
    mp11::mp_and<
      std::is_class<F>,
      mp11::mp_not<std::is_final<F> > /* is_transparent_class_helper derives from F */
    >::value
  >::type
>:is_transparent_class<F,Arg1,Arg2>{};

template<typename F,typename Arg1,typename Arg2,typename=void>
struct is_transparent_function:std::true_type{};

template<typename F> struct function_traits_helper;

template<typename R, typename T1, typename T2>
struct function_traits_helper<R (*)(T1, T2)> {
  static const unsigned arity = 2;
  typedef R result_type;
  typedef T1 arg1_type;
  typedef T2 arg2_type;
}; // function_traits_helper

template<typename F>
struct function_traits
  : public function_traits_helper<std::add_pointer_t<F>>
{ };

template<typename F,typename Arg1,typename Arg2>
struct is_transparent_function<
  F,Arg1,Arg2,
  typename std::enable_if<
    mp11::mp_or<
      mp11::mp_not<mp11::mp_or<
        std::is_same<typename function_traits<F>::arg1_type, const Arg1&>,
        std::is_same<typename function_traits<F>::arg1_type, Arg1>
      > >,
      mp11::mp_not<mp11::mp_or<
        std::is_same<typename function_traits<F>::arg2_type, const Arg2&>,
        std::is_same<typename function_traits<F>::arg2_type, Arg2>
      > >
    >::value
  >::type
>:std::false_type{};

template<typename F,typename Arg1,typename Arg2>
struct is_transparent<
  F,Arg1,Arg2,
  typename std::enable_if<
    std::is_function_v<typename std::remove_pointer<F>::type>
  >::type
>:is_transparent_function<typename std::remove_pointer<F>::type,Arg1,Arg2>{};

} // boost::multi_index::detail

#endif
