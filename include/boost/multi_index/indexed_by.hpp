/* Copyright 2003-2017 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_INDEXED_BY_HPP
#define BOOST_MULTI_INDEX_INDEXED_BY_HPP
#pragma once

#include <boost/mp11/list.hpp>

/* indexed_by contains the index specifiers for instantiation
 * of a multi_index_container.
 */

namespace boost::multi_index {

template<typename... T> struct indexed_by;

} // boost::multi_index

#endif
