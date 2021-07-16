/* Copyright 2003-2017 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_DETAIL_UINTPTR_TYPE_HPP
#define BOOST_MULTI_INDEX_DETAIL_UINTPTR_TYPE_HPP
#pragma once

#include <type_traits>

namespace boost::multi_index::detail{

/* has_uintptr_type is an MPL integral constant determining whether
 * there exists an unsigned integral type with the same size as
 * void *.
 * uintptr_type is such a type if has_uintptr is true, or unsigned int
 * otherwise.
 * Note that uintptr_type is more restrictive than C99 uintptr_t,
 * where an integral type with size greater than that of void *
 * would be conformant.
 */

template<int N>struct uintptr_candidates;
template<>struct uintptr_candidates<-1>{typedef unsigned int           type;};
template<>struct uintptr_candidates<0> {typedef unsigned int           type;};
template<>struct uintptr_candidates<1> {typedef unsigned short         type;};
template<>struct uintptr_candidates<2> {typedef unsigned long          type;};
template<>struct uintptr_candidates<3> {typedef unsigned long long     type;};

struct uintptr_aux
{
  static const int index=
    sizeof(void*)==sizeof(uintptr_candidates<0>::type)?0:
    sizeof(void*)==sizeof(uintptr_candidates<1>::type)?1:
    sizeof(void*)==sizeof(uintptr_candidates<2>::type)?2:
    sizeof(void*)==sizeof(uintptr_candidates<3>::type)?3:-1;

  static const bool has_uintptr_type=(index>=0);

  typedef uintptr_candidates<index>::type type;
};

typedef std::bool_constant<uintptr_aux::has_uintptr_type> has_uintptr_type;
typedef uintptr_aux::type                            uintptr_type;

} // boost::multi_index::detail

#endif
