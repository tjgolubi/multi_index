//  Boost noncopyable.hpp header file  --------------------------------------//

//  (C) Copyright Beman Dawes 1999-2003. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/libs/utility for documentation.

#ifndef BOOST_MULTI_INDEX_DETAIL_NONCOPYABLE_HPP
#define BOOST_MULTI_INDEX_DETAIL_NONCOPYABLE_HPP
#pragma once

namespace boost::multi_index::detail {

class noncopyable
{
protected:
  constexpr noncopyable() = default;
  ~noncopyable() = default;
  noncopyable(const noncopyable&) = delete;
  noncopyable& operator=(const noncopyable&) = delete;
};

} // boost::multi_index::detail

#endif // BOOST_MULTI_INDEX_DETAIL_NONCOPYABLE_HPP
