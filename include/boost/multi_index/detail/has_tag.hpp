/* Copyright 2003-2017 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_DETAIL_HAS_TAG_HPP
#define BOOST_MULTI_INDEX_DETAIL_HAS_TAG_HPP
#pragma once

#include <boost/mp11/algorithm.hpp>

namespace boost::multi_index::detail {

/* determines whether an index type has a given tag in its tag list */

template<typename Tag>
struct has_tag {
  template<typename Index>
  using fn = mp11::mp_contains<typename Index::tag_list, Tag>;
};

} // boost::multi_index::detail

#endif
