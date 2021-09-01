//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/utility for most recent version including documentation.

// call_traits: defines typedefs for function usage
// (see libs/utility/call_traits.htm)

/* Release notes:
   23rd July 2000:
      Fixed array specialization. (JM)
      Added Borland specific fixes for reference types
      (issue raised by Steve Cleary).
*/

#ifndef BOOST_MULTI_INDEX_DETAIL_CALL_TRAITS_HPP
#define BOOST_MULTI_INDEX_DETAIL_CALL_TRAITS_HPP
#pragma once

#include <type_traits>
#include <cstddef>

namespace boost::multi_index::detail {

template<typename T, bool small_>
struct ct_imp2 {
  using param_type = const T&;
};

template<typename T>
struct ct_imp2<T, true> {
  using param_type = const T;
};

template<typename T, bool isp, bool b1, bool b2>
struct ct_imp {
  using param_type = const T&;
};

template<typename T, bool isp, bool b2>
struct ct_imp<T, isp, true, b2> {
  using param_type =
                  typename ct_imp2<T, (sizeof(T) <= sizeof(void*))>::param_type;
};

template<typename T, bool isp, bool b1>
struct ct_imp<T, isp, b1, true> {
  using param_type =
                typename ct_imp2<T, (sizeof(T) <= sizeof(void*))>::param_type;
};

template<typename T, bool b1, bool b2>
struct ct_imp<T, true, b1, b2> {
  using param_type = const T;
};

template<typename T>
struct call_traits {
public:
  using value_type      = T;
  using reference       = T&;
  using const_reference = const T&;

  // C++ Builder workaround: we should be able to define a compile time
  // constant and pass that as a single template parameter to ct_imp<T, bool>,
  // however compiler bugs prevent this - instead pass three bool's to
  // ct_imp<T, bool, bool, bool> and add an extra partial specialisation
  // of ct_imp to handle the logic. (JM)
  using param_type =
      typename detail::ct_imp<T, std::is_pointer<T>::value,
                                 std::is_arithmetic<T>::value,
                                 std::is_enum<T>::value>::param_type;
}; // call_traits

template<typename T>
struct call_traits<T&> {
  using value_type      = T&;
  using reference       = T&;
  using const_reference = const T&;
  using param_type      = T&;  // hh removed const
};

template<typename T, std::size_t N>
struct call_traits<T[N]> {
private:
  using array_type      = T[N];
public:
  // degrades array to pointer:
  using value_type      = const T*;
  using reference       = array_type&;
  using const_reference = const array_type&;
  using param_type      = const T* const;
};

template<typename T, std::size_t N>
struct call_traits<const T[N]> {
private:
  using array_type      = const T[N];
public:
  // degrades array to pointer:
  using value_type      = const T*;
  using reference       = array_type&;
  using const_reference = const array_type&;
  using param_type      = const T* const;
};

} // boost::multi_index::detail

#endif
