/* Copyright 2003-2020 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_RANKED_INDEX_HPP
#define BOOST_MULTI_INDEX_RANKED_INDEX_HPP
#pragma once

#include <boost/multi_index/ranked_index_fwd.hpp>
#include <boost/multi_index/detail/ord_index_impl.hpp>
#include <boost/multi_index/detail/rnk_index_ops.hpp>

namespace boost::multi_index {

namespace detail {

/* ranked_index augments a given ordered index to provide rank operations */

template<typename OrderedIndexNodeImpl>
struct ranked_node: OrderedIndexNodeImpl {
  using size_type = typename OrderedIndexNodeImpl::size_type;
  size_type size;
};

template<typename OrderedIndexImpl>
class ranked_index : public OrderedIndexImpl {
  using super = OrderedIndexImpl;

protected:
  using index_node_type   = typename super::index_node_type;
  using node_impl_pointer = typename super::node_impl_pointer;

public:
  using ctor_list      = std::tuple<>;
  using ctor_args_list = std::pair<ctor_list, typename super::ctor_args_list>;
  using allocator_type = typename super::allocator_type;
  using iterator       = typename super::iterator;
  using size_type      = typename super::size_type;

  /* rank operations */

  iterator nth(size_type n) const {
    return this->make_iterator(index_node_type::from_impl(
                                  ranked_index_nth(n, this->header()->impl())));
  }

  size_type rank(iterator position) const {
    BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(position);
    BOOST_MULTI_INDEX_CHECK_IS_OWNER(position, *this);

    return ranked_index_rank(
               position.get_node()->impl(), this->header()->impl());
  }

  template<typename CompatibleKey>
  size_type find_rank(const CompatibleKey& x) const {
    return ranked_index_find_rank(
               this->root(), this->header(), this->key, x, this->comp_);
  }

  template<typename CompatibleKey, typename CompatibleCompare>
  size_type find_rank(const CompatibleKey& x, const CompatibleCompare& comp)
      const
  {
    return ranked_index_find_rank(
               this->root(), this->header(), this->key, x, comp);
  }

  template<typename CompatibleKey>
  size_type lower_bound_rank(const CompatibleKey& x) const {
    return ranked_index_lower_bound_rank(
               this->root(), this->header(), this->key, x, this->comp_);
  }

  template<typename CompatibleKey, typename CompatibleCompare>
  size_type lower_bound_rank(const CompatibleKey& x,
                             const CompatibleCompare& comp) const
  {
    return ranked_index_lower_bound_rank(
               this->root(), this->header(), this->key, x, comp);
  }

  template<typename CompatibleKey>
  size_type upper_bound_rank(const CompatibleKey& x) const {
    return ranked_index_upper_bound_rank(
               this->root(), this->header(), this->key, x, this->comp_);
  }

  template<typename CompatibleKey, typename CompatibleCompare>
  size_type upper_bound_rank(const CompatibleKey& x,
                             const CompatibleCompare& comp) const
  {
    return ranked_index_upper_bound_rank(
               this->root(), this->header(), this->key, x, comp);
  }

  template<typename CompatibleKey>
  std::pair<size_type, size_type>
  equal_range_rank(const CompatibleKey& x) const {
    return ranked_index_equal_range_rank(
               this->root(), this->header(), this->key, x, this->comp_);
  }

  template<typename CompatibleKey, typename CompatibleCompare>
  std::pair<size_type, size_type>
  equal_range_rank(const CompatibleKey& x, const CompatibleCompare& comp) const
  {
    return ranked_index_equal_range_rank(
               this->root(), this->header(), this->key, x, comp);
  }

  template<typename LowerBounder, typename UpperBounder>
  std::pair<size_type, size_type>
  range_rank(LowerBounder lower, UpperBounder upper) const {
    using dispatch =  std::conditional_t<
                        std::is_same_v<LowerBounder, unbounded_type>,
                        std::conditional_t<
                          std::is_same_v<UpperBounder, unbounded_type>,
                          both_unbounded_tag,
                          lower_unbounded_tag
                        >,
                        std::conditional_t<
                          std::is_same_v<UpperBounder, unbounded_type>,
                          upper_unbounded_tag,
                          none_unbounded_tag
                        >
                      >;

    return range_rank(lower, upper, dispatch());
  }

protected:
  ranked_index(const ranked_index& x) : super(x) { }

  ranked_index(const ranked_index& x, do_not_copy_elements_tag)
    : super(x, do_not_copy_elements_tag()) { }

  ranked_index(const ctor_args_list& args_list, const allocator_type& al)
    : super(args_list.second, al) { }

private:
  template<typename LowerBounder, typename UpperBounder>
  std::pair<size_type, size_type>
  range_rank(LowerBounder lower, UpperBounder upper, none_unbounded_tag) const {
    index_node_type* y = this->header();
    index_node_type* z = this->root();

    if (!z)
      return std::pair<size_type, size_type>(0, 0);

    size_type s = z->impl()->size;

    do {
      if (!lower(this->key(z->value())))
        z = index_node_type::from_impl(z->right());
      else if (!upper(this->key(z->value()))) {
        y = z;
        s -= ranked_node_size(y->right()) + 1;
        z = index_node_type::from_impl(z->left());
      }
      else {
        return std::pair<size_type, size_type>(
                   s - z->impl()->size +
            lower_range_rank(index_node_type::from_impl(z->left()),  z, lower),
                   s - ranked_node_size(z->right()) +
            upper_range_rank(index_node_type::from_impl(z->right()), y, upper));
      }
    } while (z);

    return std::pair<size_type, size_type>(s, s);
  } // range_rank

  template<typename LowerBounder, typename UpperBounder>
  std::pair<size_type, size_type>
  range_rank(LowerBounder, UpperBounder upper, lower_unbounded_tag) const {
    return std::pair<size_type, size_type>(
               0,
               upper_range_rank(this->root(), this->header(), upper));
  }

  template<typename LowerBounder, typename UpperBounder>
  std::pair<size_type, size_type>
  range_rank(LowerBounder lower, UpperBounder, upper_unbounded_tag) const {
    return std::pair<size_type, size_type>(
               lower_range_rank(this->root(), this->header(), lower),
               this->size());
  }

  template<typename LowerBounder, typename UpperBounder>
  std::pair<size_type, size_type>
  range_rank(LowerBounder, UpperBounder, both_unbounded_tag) const
  { return std::pair<size_type, size_type>(0, this->size()); }

  template<typename LowerBounder>
  size_type lower_range_rank(index_node_type* top, index_node_type* y,
                             LowerBounder lower) const
  {
    if (!top)
      return 0;

    size_type s = top->impl()->size;

    do {
      if (lower(this->key(top->value()))) {
        y = top;
        s -= ranked_node_size(y->right()) + 1;
        top = index_node_type::from_impl(top->left());
      }
      else
        top = index_node_type::from_impl(top->right());
    } while (top);

    return s;
  } // lower_range_rank

  template<typename UpperBounder>
  size_type upper_range_rank(index_node_type* top, index_node_type* y,
                             UpperBounder upper) const
  {
    if (!top)
      return 0;

    size_type s = top->impl()->size;

    do {
      if (!upper(this->key(top->value()))) {
        y = top;
        s -= ranked_node_size(y->right()) + 1;
        top = index_node_type::from_impl(top->left());
      }
      else
        top = index_node_type::from_impl(top->right());
    } while (top);

    return s;
  }
}; // ranked_index

/* augmenting policy for ordered_index */

struct rank_policy {
  template<typename OrderedIndexNodeImpl>
  struct augmented_node {
    using type = ranked_node<OrderedIndexNodeImpl>;
  };

  template<typename OrderedIndexImpl>
  struct augmented_interface {
    using type = ranked_index<OrderedIndexImpl>;
  };

  /* algorithmic stuff */

  template<typename Pointer>
  static void add(Pointer x, Pointer root) {
    x->size = 1;
    while (x != root) {
      x = x->parent();
      ++(x->size);
    }
  }

  template<typename Pointer>
  static void remove(Pointer x, Pointer root) {
    while (x != root) {
      x = x->parent();
      --(x->size);
    }
  }

  template<typename Pointer>
  static void copy(Pointer x, Pointer y)
  { y->size = x->size; }

  template<typename Pointer>
  static void rotate_left(Pointer x, Pointer y) { /* in: x==y->left() */
    y->size = x->size;
    x->size = ranked_node_size(x->left()) + ranked_node_size(x->right()) + 1;
  }

  template<typename Pointer>
  static void rotate_right(Pointer x, Pointer y) /* in: x==y->right() */
  { rotate_left(x, y); }

#if defined(BOOST_MULTI_INDEX_ENABLE_INVARIANT_CHECKING)
  /* invariant stuff */

  template<typename Pointer>
  static bool invariant(Pointer x) {
    return x->size == ranked_node_size(x->left()) + ranked_node_size(
               x->right()) + 1;
  }
#endif
}; // rank_policy

} // detail

/* ranked_index specifiers */

template<typename Arg1, typename Arg2, typename Arg3>
struct ranked_unique {
  using index_args          = detail::ordered_index_args<Arg1, Arg2, Arg3>;
  using tag_list_type       = typename index_args::tag_list_type;
  using key_from_value_type = typename index_args::key_from_value_type;
  using compare_type        = typename index_args::compare_type;

  template<typename Super>
  struct node_class {
    using type = detail::ordered_index_node<detail::rank_policy, Super>;
  };

  template<typename SuperMeta>
  struct index_class {
    using type = detail::ordered_index<key_from_value_type, compare_type,
                         SuperMeta, tag_list_type, detail::ordered_unique_tag,
                         detail::rank_policy>;
  };
}; // ranked_unique

template<typename Arg1, typename Arg2, typename Arg3>
struct ranked_non_unique {
  using index_args          = detail::ordered_index_args<Arg1, Arg2, Arg3>;
  using tag_list_type       = typename index_args::tag_list_type;
  using key_from_value_type = typename index_args::key_from_value_type;
  using compare_type        = typename index_args::compare_type;

  template<typename Super>
  struct node_class {
    using type = detail::ordered_index_node<detail::rank_policy, Super>;
  };

  template<typename SuperMeta>
  struct index_class {
    using type = detail::ordered_index<key_from_value_type, compare_type,
                                       SuperMeta, tag_list_type,
                                       detail::ordered_non_unique_tag,
                                       detail::rank_policy>;
  };
}; // ranked_non_unique

} // boost::multi_index

#endif
