/* Copyright 2003-2020 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_DETAIL_COPY_MAP_HPP
#define BOOST_MULTI_INDEX_DETAIL_COPY_MAP_HPP
#pragma once

#include <boost/multi_index/detail/auto_space.hpp>
#include <boost/multi_index/detail/raw_ptr.hpp>
#include <boost/multi_index/detail/noncopyable.hpp>
#include <algorithm>
#include <functional>
#include <memory>

namespace boost::multi_index::detail {

/* copy_map is used as an auxiliary structure during copy_() operations.
 * When a container with n nodes is replicated, node_map holds the pairings
 * between original and copied nodes, and provides a fast way to find a
 * copied node from an original one.
 * The semantics of the class are not simple, and no attempt has been made
 * to enforce it: multi_index_container handles it right. On the other hand,
 * the const interface, which is the one provided to index implementations,
 * only allows for:
 *   - Enumeration of pairs of (original,copied) nodes (excluding the headers),
 *   - fast retrieval of copied nodes (including the headers.)
 */

template <typename Node>
struct copy_map_entry {
  copy_map_entry(Node* f, Node* s): first(f), second(s) {}

  Node* first;
  Node* second;

  bool operator<(const copy_map_entry<Node>& x) const
  {
    return std::less<Node*>()(first, x.first);
  }
};

struct copy_map_value_copier {
  template<typename Value>
  const Value& operator()(Value& x) const
  {
    return x;
  }
};

struct copy_map_value_mover {
  template<typename Value>
  Value&& operator()(Value& x) const
  {
    return std::move(x);
  }
};

template <typename Node, typename Allocator>
class copy_map: private noncopyable {
  using allocator_type = typename std::allocator_traits<Allocator>::
                                                            rebind_alloc<Node>;
  using alloc_traits = std::allocator_traits<allocator_type>;
  using pointer = typename alloc_traits::pointer;

public:
  using const_iterator = const copy_map_entry<Node>*;
  using size_type = typename alloc_traits::size_type;

  copy_map(
      const Allocator& al, size_type size, Node* header_org, Node* header_cpy):
    al_(al), size_(size), spc(al_, size_), n(0),
    header_org_(header_org), header_cpy_(header_cpy), released(false)
  {}

  ~copy_map()
  {
    if (!released) {
      for (size_type i = 0; i < n; ++i) {
        alloc_traits::destroy(
            al_, std::addressof((spc.data() + i)->second->value()));
        deallocate((spc.data() + i)->second);
      }
    }
  }

  const_iterator begin() const
  {
    return raw_ptr<const_iterator>(spc.data());
  }
  const_iterator end() const
  {
    return raw_ptr<const_iterator>(spc.data() + n);
  }

  void copy_clone(Node* node)
  {
    clone(node, copy_map_value_copier());
  }
  void move_clone(Node* node)
  {
    clone(node, copy_map_value_mover());
  }

  Node* find(Node* node) const
  {
    if (node == header_org_)
      return header_cpy_;
    return std::lower_bound(
               begin(), end(), copy_map_entry<Node>(node, 0))->second;
  }

  void release()
  {
    released = true;
  }

private:
  allocator_type                             al_;
  size_type                                  size_;
  auto_space<copy_map_entry<Node>, Allocator> spc;
  size_type                                  n;
  Node*                                      header_org_;
  Node*                                      header_cpy_;
  bool                                       released;

  pointer allocate()
  {
    return alloc_traits::allocate(al_, 1);
  }

  void deallocate(Node* node)
  {
    alloc_traits::deallocate(al_, static_cast<pointer>(node), 1);
  }

  template<typename ValueAccess>
  void clone(Node* node, ValueAccess access)
  {
    (spc.data() + n)->first = node;
    (spc.data() + n)->second = raw_ptr<Node*>(allocate());
    try {
      alloc_traits::construct(
          al_, std::addressof((spc.data() + n)->second->value()),
          access(node->value()));
    }
    catch (...) {
      deallocate((spc.data() + n)->second);
      throw;
    }
    ++n;

    if (n == size_) {
      std::sort(
          raw_ptr<copy_map_entry<Node>*>(spc.data()),
          raw_ptr<copy_map_entry<Node>*>(spc.data()) + size_);
    }
  }
};

} // boost::multi_index::detail

#endif
