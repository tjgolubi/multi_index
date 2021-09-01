/* Copyright 2003-2018 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_DETAIL_DUPLICATES_ITERATOR_HPP
#define BOOST_MULTI_INDEX_DETAIL_DUPLICATES_ITERATOR_HPP
#pragma once

#include <iterator>

namespace boost::multi_index::detail {

/* duplicates_operator is given a range of ordered elements and
 * passes only over those which are duplicated.
 */

template<typename Node, typename Predicate>
class duplicates_iterator {
public:
  using value_type        = typename Node::value_type;
  using difference_type   = typename Node::difference_type;
  using pointer           = const typename Node::value_type*;
  using reference         = const typename Node::value_type&;
  using iterator_category = std::forward_iterator_tag;

  duplicates_iterator(Node* node_, Node* end_, Predicate pred_)
    : node(node_), begin_chunk(0), end(end_), pred(pred_)
  { advance(); }

  duplicates_iterator(Node* end_, Predicate pred_)
    : node(end_), begin_chunk(end_), end(end_), pred(pred_)
  { }

  reference operator*() const { return node->value(); }

  pointer operator->() const { return &node->value(); }

  duplicates_iterator& operator++() {
    Node::increment(node);
    sync();
    return *this;
  }

  duplicates_iterator operator++(int) {
    duplicates_iterator tmp(*this);
    ++(*this);
    return tmp;
  }

  Node* get_node() const { return node; }

private:
  void sync() {
    if (node != end && pred(begin_chunk->value(), node->value()))
      advance();
  }

  void advance() {
    for (Node* node2 = node; node != end; node = node2) {
      Node::increment(node2);
      if (node2 != end && !pred(node->value(), node2->value()))
        break;
    }
    begin_chunk = node;
  }

  Node*     node;
  Node*     begin_chunk;
  Node*     end;
  Predicate pred;
}; // duplicates_iterator

template<typename Node, typename Predicate>
bool operator==(const duplicates_iterator<Node, Predicate>& x,
                const duplicates_iterator<Node, Predicate>& y)
{ return (x.get_node() == y.get_node()); }

template<typename Node, typename Predicate>
bool operator!=(const duplicates_iterator<Node, Predicate>& x,
                const duplicates_iterator<Node, Predicate>& y)
{ return !(x == y); }

} // boost::multi_index::detail

#endif
