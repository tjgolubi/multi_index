/* Copyright 2003-2020 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_DETAIL_ARCHIVE_CONSTRUCTED_HPP
#define BOOST_MULTI_INDEX_DETAIL_ARCHIVE_CONSTRUCTED_HPP
#pragma once

#include <boost/serialization/serialization.hpp>
#include <type_traits>

namespace boost::multi_index::detail {

/* constructs a stack-based object from a serialization archive */

template<typename T>
struct archive_constructed {
  archive_constructed(const archive_constructed&) = delete;
  archive_constructed& operator=(const archive_constructed&) = delete;

  template<class Archive> archive_constructed(Archive& ar,
      const unsigned int version)
  {
    serialization::load_construct_data_adl(ar, &get(), version);
    try {
      ar >> get();
    }
    catch (...) {
      (&get())->~T();
      throw;
    }
  }

  template<class Archive> archive_constructed(const char* name, Archive& ar,
      const unsigned int version)
  {
    serialization::load_construct_data_adl(ar, &get(), version);
    try {
      ar >> serialization::make_nvp(name, get());
    }
    catch (...) {
      (&get())->~T();
      throw;
    }
  }

  ~archive_constructed()
  {
    (&get())->~T();
  }

  T& get()
  {
    return *reinterpret_cast<T*>(&space);
  }

private:
  typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type
  space;
};

} // boost::multi_index::detail

#endif
