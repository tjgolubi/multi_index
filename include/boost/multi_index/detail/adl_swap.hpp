/* Copyright 2003-2013 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_DETAIL_ADL_SWAP_HPP
#define BOOST_MULTI_INDEX_DETAIL_ADL_SWAP_HPP
#pragma once

#include <algorithm>

namespace boost::multi_index::detail{

template<typename T>
void adl_swap(T& x,T& y)
{
  using std::swap;
  swap(x,y);
}

} // boost::multi_index::detail

#endif
