/* Copyright 2003-2015 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_DETAIL_VALUE_COMPARE_HPP
#define BOOST_MULTI_INDEX_DETAIL_VALUE_COMPARE_HPP
#pragma once

#include <boost/multi_index/detail/call_traits.hpp>

namespace boost::multi_index::detail {

template<typename Value, typename KeyFromValue, typename Compare>
struct value_comparison {
  typedef Value first_argument_type;
  typedef Value second_argument_type;
  typedef bool  result_type;

  value_comparison(
      const KeyFromValue& key_ = KeyFromValue(), const Compare& comp_ = Compare()):
    key(key_), comp(comp_)
  {
  }

  bool operator()(
      typename detail::call_traits<Value>::param_type x,
      typename detail::call_traits<Value>::param_type y) const
  {
    return comp(key(x), key(y));
  }

private:
  KeyFromValue key;
  Compare      comp;
};

} // boost::multi_index::detail

#endif
