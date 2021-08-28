/* Copyright 2003-2020 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_SEQUENCED_INDEX_HPP
#define BOOST_MULTI_INDEX_SEQUENCED_INDEX_HPP
#pragma once

#include <boost/multi_index/sequenced_index_fwd.hpp>

#include <boost/multi_index/detail/allocator_traits.hpp>
#include <boost/multi_index/detail/bidir_node_iterator.hpp>
#include <boost/multi_index/detail/do_not_copy_elements_tag.hpp>
#include <boost/multi_index/detail/index_node_base.hpp>
#include <boost/multi_index/detail/node_handle.hpp>
#include <boost/multi_index/detail/safe_mode.hpp>
#include <boost/multi_index/detail/seq_index_node.hpp>
#include <boost/multi_index/detail/seq_index_ops.hpp>
#include <boost/multi_index/detail/scope_guard.hpp>
#include <boost/multi_index/detail/call_traits.hpp>

#include <boost/foreach_fwd.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mp11/list.hpp>
#include <boost/mp11/function.hpp>

#include <tuple>
#include <type_traits>
#include <utility>
#include <initializer_list>
#include <iterator>

#if defined(BOOST_MULTI_INDEX_ENABLE_INVARIANT_CHECKING)

#define BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT_OF(x)                    \
  detail::scope_guard BOOST_JOIN(check_invariant_, __LINE__)=                \
    detail::make_obj_guard(x, &sequenced_index::check_invariant_);           \
  BOOST_JOIN(check_invariant_, __LINE__).touch();

#define BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT                          \
  BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT_OF(*this)

#else

#define BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT_OF(x)
#define BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT

#endif

namespace boost::multi_index {

namespace detail {

/* sequenced_index adds a layer of sequenced indexing to a given Super */

template<typename SuperMeta, typename TagList>
class sequenced_index
  : protected SuperMeta::type

#if defined(BOOST_MULTI_INDEX_ENABLE_SAFE_MODE)
  , public safe_mode::safe_container<sequenced_index<SuperMeta, TagList>>
#endif

{
  using super = typename SuperMeta::type;

protected:
  using index_node_type = sequenced_index_node<typename super::index_node_type>;

private:
  using node_impl_type = typename index_node_type::impl_type;

public:
  /* types */

  using value_type      = typename index_node_type::value_type;
  using ctor_args       = std::tuple<>;
  using allocator_type  = typename super::final_allocator_type;
  using reference       = value_type&;
  using const_reference = const value_type&;

#if defined(BOOST_MULTI_INDEX_ENABLE_SAFE_MODE)
  using iterator = safe_mode::safe_iterator<
                        bidir_node_iterator<index_node_type>, sequenced_index>;
#else
  using iterator = bidir_node_iterator<index_node_type>;
#endif

  using const_iterator = iterator;

private:
  using alloc_traits = allocator_traits<allocator_type>;

public:
  using pointer            = typename alloc_traits::pointer;
  using const_pointer      = typename alloc_traits::const_pointer;
  using size_type          =  typename alloc_traits::size_type;
  using difference_type    = typename alloc_traits::difference_type;
  using reverse_iterator   = typename std::reverse_iterator<iterator>;
  using const_reverse_iterator = typename std::reverse_iterator<const_iterator>;
  using node_type          = typename super::final_node_handle_type;
  using insert_return_type = detail::insert_return_type<iterator, node_type>;
  using tag_list           = TagList;

protected:
  using final_node_type = typename super::final_node_type;
  using ctor_args_list = std::pair<ctor_args, typename super::ctor_args_list>;
  using index_type_list =
          mp11::mp_push_front<typename super::index_type_list, sequenced_index>;
  using iterator_type_list =
          mp11::mp_push_front<typename super::iterator_type_list, iterator>;
  using const_iterator_type_list = mp11::mp_push_front<
                      typename super::const_iterator_type_list, const_iterator>;
  using copy_map_type = typename super::copy_map_type;

#if !defined(BOOST_MULTI_INDEX_DISABLE_SERIALIZATION)
  using index_saver_type  = typename super::index_saver_type;
  using index_loader_type = typename super::index_loader_type;
#endif

private:
#if defined(BOOST_MULTI_INDEX_ENABLE_SAFE_MODE)
  using safe_super = safe_mode::safe_container<sequenced_index>;
#endif

  using value_param_type = typename detail::call_traits<value_type>::param_type;

  using emplace_return_type = std::pair<iterator, bool>;

public:

  /* construct/copy/destroy
   * Default and copy ctors are in the protected section as indices are
   * not supposed to be created on their own. No range ctor either.
   */

  sequenced_index& operator=(const sequenced_index& x) {
    this->final() = x.final();
    return *this;
  }

  sequenced_index& operator=(std::initializer_list<value_type> list) {
    this->final() = list;
    return *this;
  }

  template<class InputIterator>
  void assign(InputIterator first, InputIterator last)
  { assign_iter(first, last, mp11::mp_not<std::is_integral<InputIterator>>()); }

  void assign(std::initializer_list<value_type> list)
  { assign(list.begin(), list.end()); }

  void assign(size_type n, value_param_type value) {
    BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT;
    clear();
    for (size_type i = 0; i < n; ++i)
      push_back(value);
  }

  allocator_type get_allocator() const noexcept
  { return this->final().get_allocator(); }

  /* iterators */

  iterator       begin() noexcept
  { return make_iterator(index_node_type::from_impl(header()->next())); }

  const_iterator begin() const noexcept
  { return make_iterator(index_node_type::from_impl(header()->next())); }

  iterator       end()       noexcept { return make_iterator(header()); }

  const_iterator end() const noexcept { return make_iterator(header()); }

  reverse_iterator       rbegin() noexcept
  { return std::make_reverse_iterator(end()); }

  const_reverse_iterator rbegin() const noexcept
  { return std::make_reverse_iterator(end()); }

  reverse_iterator       rend() noexcept
  { return std::make_reverse_iterator(begin()); }

  const_reverse_iterator rend() const noexcept
  { return std::make_reverse_iterator(begin()); }

  const_iterator cbegin() const noexcept { return begin(); }

  const_iterator         cend()    const noexcept { return end(); }
  const_reverse_iterator crbegin() const noexcept { return rbegin(); }
  const_reverse_iterator crend()   const noexcept { return rend(); }

  iterator iterator_to(const value_type& x)
  { return make_iterator(node_from_value<index_node_type>(std::addressof(x))); }

  const_iterator iterator_to(const value_type& x) const
  { return make_iterator(node_from_value<index_node_type>(std::addressof(x))); }

  /* capacity */

  bool      empty()    const noexcept { return this->final_empty_(); }
  size_type size()     const noexcept { return this->final_size_(); }
  size_type max_size() const noexcept { return this->final_max_size_(); }

  void resize(size_type n) {
    BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT;
    if (n > size()) {
      for (size_type m = n - size(); m--;)
        this->final_emplace_();
    }
    else if (n < size()) {
      for (size_type m = size() - n; m--;)
        pop_back();
    }
  }

  void resize(size_type n, value_param_type x) {
    BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT;
    if (n > size())
      insert(end(), static_cast<size_type>(n - size()), x);
    else if (n < size())
      for (size_type m = size() - n; m--;)
        pop_back();
  }

  /* access: no non-const versions provided as sequenced_index
   * handles const elements.
   */

  const_reference front() const { return *begin(); }
  const_reference back()  const { return *--end(); }

  /* modifiers */

  template<typename... Args>
  emplace_return_type emplace_front(Args&& ... args)
  { return emplace_front_impl(std::forward<Args>(args)...); }

  std::pair<iterator, bool> push_front(const value_type& x)
  { return insert(begin(), x); }

  std::pair<iterator, bool> push_front(value_type&& x)
  { return insert(begin(), std::move(x)); }

  void pop_front() { erase(begin()); }

  template<typename... Args>
  emplace_return_type emplace_back(Args&& ... args)
  { return emplace_back_impl(std::forward<Args>(args)...); }

  std::pair<iterator, bool> push_back(const value_type& x)
  { return insert(end(), x); }
  std::pair<iterator, bool> push_back(value_type&& x)
  { return insert(end(), std::move(x)); }
  void pop_back() { erase(--end()); }

  template<typename... Args>
  emplace_return_type emplace(iterator position, Args&& ... args)
  { return emplace_impl(position, std::forward<Args>(args)...); }

  std::pair<iterator, bool> insert(iterator position, const value_type& x) {
    BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(position);
    BOOST_MULTI_INDEX_CHECK_IS_OWNER(position, *this);
    BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT;
    std::pair<final_node_type*, bool> p = this->final_insert_(x);
    if (p.second && position.get_node() != header())
      relink(position.get_node(), p.first);
    return std::pair<iterator, bool>(make_iterator(p.first), p.second);
  }

  std::pair<iterator, bool> insert(iterator position, value_type&& x) {
    BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(position);
    BOOST_MULTI_INDEX_CHECK_IS_OWNER(position, *this);
    BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT;
    std::pair<final_node_type*, bool> p = this->final_insert_rv_(x);
    if (p.second && position.get_node() != header())
      relink(position.get_node(), p.first);
    return std::pair<iterator, bool>(make_iterator(p.first), p.second);
  }

  void insert(iterator position, size_type n, value_param_type x) {
    BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(position);
    BOOST_MULTI_INDEX_CHECK_IS_OWNER(position, *this);
    BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT;
    for (size_type i = 0; i < n; ++i)
      insert(position, x);
  }

  template<typename InputIterator>
  void insert(iterator position, InputIterator first, InputIterator last) {
    insert_iter(position, first, last,
                mp11::mp_not<std::is_integral<InputIterator>>());
  }

  void insert(iterator position, std::initializer_list<value_type> list)
  { insert(position, list.begin(), list.end()); }

  insert_return_type insert(const_iterator position, node_type&& nh) {
    BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(position);
    BOOST_MULTI_INDEX_CHECK_IS_OWNER(position, *this);
    if (nh)
      BOOST_MULTI_INDEX_CHECK_EQUAL_ALLOCATORS(*this, nh);
    BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT;
    std::pair<final_node_type*, bool> p = this->final_insert_nh_(nh);
    if (p.second && position.get_node() != header())
      relink(position.get_node(), p.first);
    return insert_return_type(make_iterator(p.first), p.second, std::move(nh));
  }

  node_type extract(const_iterator position) {
    BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(position);
    BOOST_MULTI_INDEX_CHECK_DEREFERENCEABLE_ITERATOR(position);
    BOOST_MULTI_INDEX_CHECK_IS_OWNER(position, *this);
    BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT;
    return this->final_extract_(
               static_cast<final_node_type*>(position.get_node()));
  }

  iterator erase(iterator position) {
    BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(position);
    BOOST_MULTI_INDEX_CHECK_DEREFERENCEABLE_ITERATOR(position);
    BOOST_MULTI_INDEX_CHECK_IS_OWNER(position, *this);
    BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT;
    this->final_erase_(static_cast<final_node_type*>(position++.get_node()));
    return position;
  }

  iterator erase(iterator first, iterator last) {
    BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(first);
    BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(last);
    BOOST_MULTI_INDEX_CHECK_IS_OWNER(first, *this);
    BOOST_MULTI_INDEX_CHECK_IS_OWNER(last, *this);
    BOOST_MULTI_INDEX_CHECK_VALID_RANGE(first, last);
    BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT;
    while (first != last)
      first = erase(first);
    return first;
  }

  bool replace(iterator position, const value_type& x) {
    BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(position);
    BOOST_MULTI_INDEX_CHECK_DEREFERENCEABLE_ITERATOR(position);
    BOOST_MULTI_INDEX_CHECK_IS_OWNER(position, *this);
    BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT;
    return this->final_replace_(
               x, static_cast<final_node_type*>(position.get_node()));
  }

  bool replace(iterator position, value_type&& x) {
    BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(position);
    BOOST_MULTI_INDEX_CHECK_DEREFERENCEABLE_ITERATOR(position);
    BOOST_MULTI_INDEX_CHECK_IS_OWNER(position, *this);
    BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT;
    return this->final_replace_rv_(
               x, static_cast<final_node_type*>(position.get_node()));
  }

  template<typename Modifier>
  bool modify(iterator position, Modifier mod) {
    BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(position);
    BOOST_MULTI_INDEX_CHECK_DEREFERENCEABLE_ITERATOR(position);
    BOOST_MULTI_INDEX_CHECK_IS_OWNER(position, *this);
    BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT;

#if defined(BOOST_MULTI_INDEX_ENABLE_SAFE_MODE)
    /* MSVC++ 6.0 optimizer on safe mode code chokes if this
     * this is not added. Left it for all compilers as it does no
     * harm.
     */

    position.detach();
#endif

    return this->final_modify_(
               mod, static_cast<final_node_type*>(position.get_node()));
  } // modify

  template<typename Modifier, typename Rollback>
  bool modify(iterator position, Modifier mod, Rollback back_) {
    BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(position);
    BOOST_MULTI_INDEX_CHECK_DEREFERENCEABLE_ITERATOR(position);
    BOOST_MULTI_INDEX_CHECK_IS_OWNER(position, *this);
    BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT;

#if defined(BOOST_MULTI_INDEX_ENABLE_SAFE_MODE)
    /* MSVC++ 6.0 optimizer on safe mode code chokes if this
     * this is not added. Left it for all compilers as it does no
     * harm.
     */

    position.detach();
#endif

    return this->final_modify_(
               mod, back_, static_cast<final_node_type*>(position.get_node()));
  } // modify

  void swap(sequenced_index& x) {
    BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT;
    BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT_OF(x);
    this->final_swap_(x.final());
  }

  void clear() noexcept {
    BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT;
    this->final_clear_();
  }

  /* list operations */

  void splice(iterator position, sequenced_index& x) {
    BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(position);
    BOOST_MULTI_INDEX_CHECK_IS_OWNER(position, *this);
    BOOST_MULTI_INDEX_CHECK_DIFFERENT_CONTAINER(*this, x);
    BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT;
    iterator first = x.begin(), last = x.end();
    while (first != last) {
      if (insert(position, *first).second)
        first = x.erase(first);
      else ++first;
    }
  }

  void splice(iterator position, sequenced_index& x, iterator i) {
    BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(position);
    BOOST_MULTI_INDEX_CHECK_IS_OWNER(position, *this);
    BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(i);
    BOOST_MULTI_INDEX_CHECK_DEREFERENCEABLE_ITERATOR(i);
    BOOST_MULTI_INDEX_CHECK_IS_OWNER(i, x);
    BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT;
    if (&x == this) {
      if (position != i)
        relink(position.get_node(), i.get_node());
    }
    else {
      if (insert(position, *i).second) {

#if defined(BOOST_MULTI_INDEX_ENABLE_SAFE_MODE)
        /* MSVC++ 6.0 optimizer has a hard time with safe mode, and the following
         * workaround is needed. Left it for all compilers as it does no
         * harm.
         */
        i.detach();
        x.erase(x.make_iterator(i.get_node()));
#else
        x.erase(i);
#endif

      }
    }
  } // splice

  void splice(iterator position, sequenced_index& x,
              iterator first, iterator last)
  {
    BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(position);
    BOOST_MULTI_INDEX_CHECK_IS_OWNER(position, *this);
    BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(first);
    BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(last);
    BOOST_MULTI_INDEX_CHECK_IS_OWNER(first, x);
    BOOST_MULTI_INDEX_CHECK_IS_OWNER(last, x);
    BOOST_MULTI_INDEX_CHECK_VALID_RANGE(first, last);
    BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT;
    if (&x == this) {
      BOOST_MULTI_INDEX_CHECK_OUTSIDE_RANGE(position, first, last);
      if (position != last)
        relink(
            position.get_node(), first.get_node(), last.get_node());
    }
    else {
      while (first != last) {
        if (insert(position, *first).second)
          first = x.erase(first);
        else ++first;
      }
    }
  } // splice

  void remove(value_param_type value) {
    auto pred = [value](value_param_type v)
                  { return std::equal_to<value_type>()(v, value); };
    sequenced_index_remove(*this, pred);
  }

  template<typename Predicate>
  void remove_if(Predicate pred)
  { sequenced_index_remove(*this, pred); }

  void unique()
  { sequenced_index_unique(*this, std::equal_to<value_type>()); }

  template<class BinaryPredicate>
  void unique(BinaryPredicate binary_pred)
  { sequenced_index_unique(*this, binary_pred); }

  void merge(sequenced_index& x)
  { sequenced_index_merge(*this, x, std::less<value_type>()); }

  template<typename Compare>
  void merge(sequenced_index& x, Compare comp)
  { sequenced_index_merge(*this, x, comp); }

  void sort() {
    BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT;
    sequenced_index_sort(header(), std::less<value_type>());
  }

  template<typename Compare>
  void sort(Compare comp) {
    BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT;
    sequenced_index_sort(header(), comp);
  }

  void reverse() noexcept {
    BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT;
    node_impl_type::reverse(header()->impl());
  }

  /* rearrange operations */

  void relocate(iterator position, iterator i) {
    BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(position);
    BOOST_MULTI_INDEX_CHECK_IS_OWNER(position, *this);
    BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(i);
    BOOST_MULTI_INDEX_CHECK_DEREFERENCEABLE_ITERATOR(i);
    BOOST_MULTI_INDEX_CHECK_IS_OWNER(i, *this);
    BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT;
    if (position != i)
      relink(position.get_node(), i.get_node());
  }

  void relocate(iterator position, iterator first, iterator last) {
    BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(position);
    BOOST_MULTI_INDEX_CHECK_IS_OWNER(position, *this);
    BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(first);
    BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(last);
    BOOST_MULTI_INDEX_CHECK_IS_OWNER(first, *this);
    BOOST_MULTI_INDEX_CHECK_IS_OWNER(last, *this);
    BOOST_MULTI_INDEX_CHECK_VALID_RANGE(first, last);
    BOOST_MULTI_INDEX_CHECK_OUTSIDE_RANGE(position, first, last);
    BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT;
    if (position != last)
      relink(
          position.get_node(), first.get_node(), last.get_node());
  } // relocate

  template<typename InputIterator>
  void rearrange(InputIterator first) {
    BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT;
    index_node_type* pos = header();
    for (size_type s = size(); s--;) {
      const value_type& v = *first++;
      relink(pos, node_from_value<index_node_type>(&v));
    }
  }

protected:
  sequenced_index(const ctor_args_list& args_list, const allocator_type& al)
    : super(args_list.second, al)
  { empty_initialize(); }

  sequenced_index(const sequenced_index& x)
    : super(x)

#if defined(BOOST_MULTI_INDEX_ENABLE_SAFE_MODE)
    , safe_super()
#endif

  { } // The actual copying takes place in subsequent call to copy_().

  sequenced_index(const sequenced_index& x, do_not_copy_elements_tag)
    : super(x, do_not_copy_elements_tag())

#if defined(BOOST_MULTI_INDEX_ENABLE_SAFE_MODE)
    , safe_super()
#endif

  { empty_initialize(); }

  ~sequenced_index() { } // The container is guaranteed to be empty by now.

#if defined(BOOST_MULTI_INDEX_ENABLE_SAFE_MODE)
  iterator       make_iterator(index_node_type* node)
  { return iterator(node, this); }

  const_iterator make_iterator(index_node_type* node) const
  { return const_iterator(node, const_cast<sequenced_index*>(this)); }
#else
  iterator       make_iterator(index_node_type* node)
  { return iterator(node); }

  const_iterator make_iterator(index_node_type* node) const
  { return const_iterator(node); }
#endif

  void copy_(const sequenced_index& x, const copy_map_type& map) {
    index_node_type* org = x.header();
    index_node_type* cpy = header();
    do {
      index_node_type* next_org = index_node_type::from_impl(org->next());
      index_node_type* next_cpy = map.find(
                                      static_cast<final_node_type*>(next_org));
      cpy->next() = next_cpy->impl();
      next_cpy->prior() = cpy->impl();
      org = next_org;
      cpy = next_cpy;
    } while (org != x.header());

    super::copy_(x, map);
  } // copy_

  template<typename Variant>
  final_node_type* insert_(value_param_type v, final_node_type*& x,
                           Variant variant)
  {
    final_node_type* res = super::insert_(v, x, variant);
    if (res == x)
      link(static_cast<index_node_type*>(x));
    return res;
  }

  template<typename Variant>
  final_node_type* insert_(value_param_type v, index_node_type* position,
                           final_node_type*& x, Variant variant)
  {
    final_node_type* res = super::insert_(v, position, x, variant);
    if (res == x)
      link(static_cast<index_node_type*>(x));
    return res;
  }

  void extract_(index_node_type* x) {
    unlink(x);
    super::extract_(x);

#if defined(BOOST_MULTI_INDEX_ENABLE_SAFE_MODE)
    detach_iterators(x);
#endif
  }

  void delete_all_nodes_() {
    for (index_node_type* x = index_node_type::from_impl(header()->next());
         x != header();) {
      index_node_type* y = index_node_type::from_impl(x->next());
      this->final_delete_node_(static_cast<final_node_type*>(x));
      x = y;
    }
  }

  void clear_() {
    super::clear_();
    empty_initialize();

#if defined(BOOST_MULTI_INDEX_ENABLE_SAFE_MODE)
    safe_super::detach_dereferenceable_iterators();
#endif
  }

  template<typename BoolConstant>
  void swap_(sequenced_index& x, BoolConstant swap_allocators) {
#if defined(BOOST_MULTI_INDEX_ENABLE_SAFE_MODE)
    safe_super::swap(x);
#endif

    super::swap_(x, swap_allocators);
  }

  void swap_elements_(sequenced_index& x) {
#if defined(BOOST_MULTI_INDEX_ENABLE_SAFE_MODE)
    safe_super::swap(x);
#endif

    super::swap_elements_(x);
  }

  template<typename Variant>
  bool replace_(value_param_type v, index_node_type* x, Variant variant)
  { return super::replace_(v, x, variant); }

  bool modify_(index_node_type* x) {
    try {
      if (!super::modify_(x)) {
        unlink(x);

#if defined(BOOST_MULTI_INDEX_ENABLE_SAFE_MODE)
        detach_iterators(x);
#endif

        return false;
      }
      return true;
    }
    catch (...) {
      unlink(x);

#if defined(BOOST_MULTI_INDEX_ENABLE_SAFE_MODE)
      detach_iterators(x);
#endif

      throw;
    }
  }

  bool modify_rollback_(index_node_type* x)
  { return super::modify_rollback_(x); }

  bool check_rollback_(index_node_type* x) const
  { return super::check_rollback_(x); }

#if !defined(BOOST_MULTI_INDEX_DISABLE_SERIALIZATION)
  /* serialization */

  template<typename Archive>
  void save_(Archive& ar, const unsigned int version,
             const index_saver_type& sm) const
  {
    sm.save(begin(), end(), ar, version);
    super::save_(ar, version, sm);
  }

  template<typename Archive>
  void load_(Archive& ar, const unsigned int version,
             const index_loader_type& lm)
  {
    auto fn = [this](index_node_type* n1, index_node_type* n2)
                { this->rearranger(n1, n2); };
    lm.load(fn, ar, version);
    super::load_(ar, version, lm);
  }
#endif

#if defined(BOOST_MULTI_INDEX_ENABLE_INVARIANT_CHECKING)
  /* invariant stuff */

  bool invariant_() const {
    if (size() == 0 || begin() == end()) {
      if (size() != 0 || begin() != end() ||
          header()->next() != header()->impl() ||
          header()->prior() != header()->impl())
        return false;
    }
    else {
      size_type s = 0;
      for (const_iterator it = begin(), it_end = end(); it != it_end;
          ++it, ++s)
      {
        if (it.get_node()->next()->prior() != it.get_node()->impl())
          return false;
        if (it.get_node()->prior()->next() != it.get_node()->impl())
          return false;
      }
      if (s != size())
        return false;
    }

    return super::invariant_();
  } // invariant_

  /* This forwarding function eases things for the boost::mem_fn construct
   * in BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT. Actually,
   * final_check_invariant is already an inherited member function of index.
   */
  void check_invariant_() const { this->final_check_invariant_(); }
#endif

private:
  index_node_type* header() const { return this->final_header(); }

  void empty_initialize()
  { header()->prior() = header()->next() = header()->impl(); }

  void link(index_node_type* x)
  { node_impl_type::link(x->impl(), header()->impl()); }

  static void unlink(index_node_type* x)
  { node_impl_type::unlink(x->impl()); }

  static void relink(index_node_type* position, index_node_type* x)
  { node_impl_type::relink(position->impl(), x->impl()); }

  static void relink(index_node_type* position,
                     index_node_type* first, index_node_type* last)
  { node_impl_type::relink(position->impl(), first->impl(), last->impl()); }

#if !defined(BOOST_MULTI_INDEX_DISABLE_SERIALIZATION)
  void rearranger(index_node_type* position, index_node_type* x) {
    if (!position)
      position = header();
    index_node_type::increment(position);
    if (position != x)
      relink(position, x);
  }
#endif

#if defined(BOOST_MULTI_INDEX_ENABLE_SAFE_MODE)
  void detach_iterators(index_node_type* x) {
    iterator it = make_iterator(x);
    safe_mode::detach_equivalent_iterators(it);
  }
#endif

  template<class InputIterator>
  void assign_iter(InputIterator first, InputIterator last, std::true_type) {
    BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT;
    clear();
    for (; first != last; ++first)
      this->final_insert_ref_(*first);
  }

  void assign_iter(size_type n, value_param_type value, std::false_type) {
    BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT;
    clear();
    for (size_type i = 0; i < n; ++i)
      push_back(value);
  }

  template<typename InputIterator>
  void insert_iter(iterator position, InputIterator first, InputIterator last,
                   std::true_type)
  {
    BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT;
    for (; first != last; ++first) {
      std::pair<final_node_type*, bool> p =
          this->final_insert_ref_(*first);
      if (p.second && position.get_node() != header())
        relink(position.get_node(), p.first);
    }
  }

  void insert_iter(iterator position, size_type n, value_param_type x,
                   std::false_type)
  {
    BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(position);
    BOOST_MULTI_INDEX_CHECK_IS_OWNER(position, *this);
    BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT;
    for (size_type i = 0; i < n; ++i)
      insert(position, x);
  }

  template<typename... Args>
  std::pair<iterator, bool> emplace_front_impl(Args&& ... args)
  { return emplace_impl(begin(), std::forward<Args>(args)...); }

  template<typename... Args>
  std::pair<iterator, bool> emplace_back_impl(Args&& ... args)
  { return emplace_impl(end(), std::forward<Args>(args)...); }

  template<typename... Args>
  std::pair<iterator, bool> emplace_impl(iterator position, Args&& ... args) {
    BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(position);
    BOOST_MULTI_INDEX_CHECK_IS_OWNER(position, *this);
    BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT;
    std::pair<final_node_type*, bool> p =
        this->final_emplace_(std::forward<Args>(args)...);
    if (p.second && position.get_node() != header())
      relink(position.get_node(), p.first);
    return std::pair<iterator, bool>(make_iterator(p.first), p.second);
  }
}; // sequenced_index

/* comparison */

template<typename SuperMeta1, typename TagList1,
         typename SuperMeta2, typename TagList2>
bool operator==(const sequenced_index<SuperMeta1, TagList1>& x,
                const sequenced_index<SuperMeta2, TagList2>& y)
{ return x.size() == y.size() && std::equal(x.begin(), x.end(), y.begin()); }

template<typename SuperMeta1, typename TagList1,
         typename SuperMeta2, typename TagList2>
bool operator<(const sequenced_index<SuperMeta1, TagList1>& x,
               const sequenced_index<SuperMeta2, TagList2>& y)
{ return std::lexicographical_compare(x.begin(), x.end(), y.begin(), y.end()); }

template<typename SuperMeta1, typename TagList1,
         typename SuperMeta2, typename TagList2>
bool operator!=(const sequenced_index<SuperMeta1, TagList1>& x,
                const sequenced_index<SuperMeta2, TagList2>& y)
{ return !(x == y); }

template<typename SuperMeta1, typename TagList1,
         typename SuperMeta2, typename TagList2>
bool operator>(const sequenced_index<SuperMeta1, TagList1>& x,
               const sequenced_index<SuperMeta2, TagList2>& y)
{ return y < x; }

template<typename SuperMeta1, typename TagList1,
         typename SuperMeta2, typename TagList2>
bool operator>=(const sequenced_index<SuperMeta1, TagList1>& x,
                const sequenced_index<SuperMeta2, TagList2>& y)
{ return !(x < y); }

template<typename SuperMeta1, typename TagList1,
         typename SuperMeta2, typename TagList2>
bool operator<=(const sequenced_index<SuperMeta1, TagList1>& x,
                const sequenced_index<SuperMeta2, TagList2>& y)
{ return !(x > y); }

/*  specialized algorithms */

template<typename SuperMeta, typename TagList>
void swap(sequenced_index<SuperMeta, TagList>& x,
          sequenced_index<SuperMeta, TagList>& y)
{ x.swap(y); }

} // detail

/* sequenced index specifier */

template<typename TagList>
struct sequenced {
  static_assert(detail::is_tag<TagList>::value);

  template<typename Super>
  struct node_class {
    using type = detail::sequenced_index_node<Super>;
  };

  template<typename SuperMeta>
  struct index_class {
    using type = detail::sequenced_index<SuperMeta, TagList>;
  };
}; // sequenced

} // boost::multi_index

/* Boost.Foreach compatibility */

template<typename SuperMeta, typename TagList>
inline boost::mpl::true_* boost_foreach_is_noncopyable(
    boost::multi_index::detail::sequenced_index<SuperMeta, TagList>*&,
    boost_foreach_argument_dependent_lookup_hack)
{ return nullptr; }

#undef BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT
#undef BOOST_MULTI_INDEX_SEQ_INDEX_CHECK_INVARIANT_OF

#endif
