/* Multiply indexed container.
 *
 * Copyright 2003-2020 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_HPP
#define BOOST_MULTI_INDEX_HPP
#pragma once

#include <boost/multi_index_container_fwd.hpp>
#include <boost/multi_index/detail/access_specifier.hpp>
#include <boost/multi_index/detail/do_not_copy_elements_tag.hpp>
#include <boost/multi_index/detail/converter.hpp>
#include <boost/multi_index/detail/has_tag.hpp>
#include <boost/multi_index/detail/safe_mode.hpp>
#include <boost/multi_index/detail/no_duplicate_tags.hpp>
#include <boost/multi_index/detail/header_holder.hpp>
#include <boost/multi_index/detail/allocator_traits.hpp>
#include <boost/multi_index/detail/base_type.hpp>
#include <boost/multi_index/detail/scope_guard.hpp>
#include <boost/multi_index/detail/adl_swap.hpp>

#if !defined(BOOST_MULTI_INDEX_DISABLE_SERIALIZATION)
#include <boost/multi_index/detail/archive_constructed.hpp>
#include <boost/multi_index/detail/serialization_version.hpp>
#include <boost/serialization/collection_size_type.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/version.hpp>
#include <boost/throw_exception.hpp> 
#endif

#include <boost/utility/base_from_member.hpp>
#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/list.hpp>
#include <boost/mp11/utility.hpp>
#include <initializer_list>
#include <algorithm>

#if defined(BOOST_MULTI_INDEX_ENABLE_INVARIANT_CHECKING)
#include <boost/multi_index/detail/invariant_assert.hpp>
#define BOOST_MULTI_INDEX_CHECK_INVARIANT_OF(x)                              \
  detail::scope_guard BOOST_JOIN(check_invariant_,__LINE__)=                 \
    detail::make_obj_guard(x,&multi_index_container::check_invariant_);      \
  BOOST_JOIN(check_invariant_,__LINE__).touch();
#define BOOST_MULTI_INDEX_CHECK_INVARIANT                                    \
  BOOST_MULTI_INDEX_CHECK_INVARIANT_OF(*this)
#else
#define BOOST_MULTI_INDEX_CHECK_INVARIANT_OF(x)
#define BOOST_MULTI_INDEX_CHECK_INVARIANT
#endif

namespace boost{

namespace multi_index{

namespace detail{

struct unequal_alloc_move_ctor_tag{};

} // detail

template<typename Value,typename IndexSpecifierList,typename Allocator>
class multi_index_container:
  private ::boost::base_from_member<
    typename detail::rebind_alloc_for<
      Allocator,
      typename detail::multi_index_node_type<
        Value,IndexSpecifierList,Allocator>::type
    >::type
  >,
  BOOST_MULTI_INDEX_PRIVATE_IF_MEMBER_TEMPLATE_FRIENDS detail::header_holder<
    typename detail::allocator_traits<
      typename detail::rebind_alloc_for<
        Allocator,
        typename detail::multi_index_node_type<
          Value,IndexSpecifierList,Allocator>::type
      >::type
    >::pointer,
    multi_index_container<Value,IndexSpecifierList,Allocator> >,
  public detail::multi_index_base_type<
    Value,IndexSpecifierList,Allocator>::type
{
private:
  template <typename,typename,typename> friend class  detail::index_base;
  template <typename,typename>          friend struct detail::header_holder;
  template <typename,typename>          friend struct detail::converter;

  typedef typename detail::multi_index_base_type<
      Value,IndexSpecifierList,Allocator>::type    super;
  typedef typename detail::rebind_alloc_for<
    Allocator,
    typename super::index_node_type
  >::type                                          node_allocator;
  typedef detail::allocator_traits<node_allocator> node_alloc_traits;
  typedef typename node_alloc_traits::pointer      node_pointer;
  typedef ::boost::base_from_member<
    node_allocator>                                bfm_allocator;
  typedef detail::header_holder<
    node_pointer,
    multi_index_container>                         bfm_header;

public:
  /* All types are inherited from super, a few are explicitly
   * brought forward here to save us some typename's.
   */

  typedef typename super::ctor_args_list           ctor_args_list;
  typedef IndexSpecifierList                       index_specifier_type_list;
 
  typedef typename super::index_type_list          index_type_list;

  typedef typename super::iterator_type_list       iterator_type_list;
  typedef typename super::const_iterator_type_list const_iterator_type_list;
  typedef typename super::value_type               value_type;
  typedef typename super::final_allocator_type     allocator_type;
  typedef typename super::size_type                size_type;
  typedef typename super::iterator                 iterator;
  typedef typename super::const_iterator           const_iterator;

  static_assert(
    detail::no_duplicate_tags_in_index_list<index_type_list>::value);

  /* global project() needs to see this publicly */

  typedef typename super::final_node_type         final_node_type;

  /* construct/copy/destroy */

  multi_index_container():
    bfm_allocator(allocator_type()),
    super(ctor_args_list(),bfm_allocator::member),
    node_count(0)
  {
    BOOST_MULTI_INDEX_CHECK_INVARIANT;
  }

  explicit multi_index_container(
    const ctor_args_list& args_list,

    const allocator_type& al=allocator_type()):

    bfm_allocator(al),
    super(args_list,bfm_allocator::member),
    node_count(0)
  {
    BOOST_MULTI_INDEX_CHECK_INVARIANT;
  }

  explicit multi_index_container(const allocator_type& al):
    bfm_allocator(al),
    super(ctor_args_list(),bfm_allocator::member),
    node_count(0)
  {
    BOOST_MULTI_INDEX_CHECK_INVARIANT;
  }
  
  template<typename InputIterator>
  multi_index_container(
    InputIterator first,InputIterator last,

    const ctor_args_list& args_list=ctor_args_list(),
    const allocator_type& al=allocator_type()):

    bfm_allocator(al),
    super(args_list,bfm_allocator::member),
    node_count(0)
  {
    BOOST_MULTI_INDEX_CHECK_INVARIANT;
    try{
      iterator hint=super::end();
      for(;first!=last;++first){
        hint=super::make_iterator(
          insert_ref_(*first,hint.get_node()).first);
        ++hint;
      }
    }
    catch(...){
      clear_();
      throw;
    }
  }

  multi_index_container(
    std::initializer_list<Value> list,
    const ctor_args_list& args_list=ctor_args_list(),
    const allocator_type& al=allocator_type()):
    bfm_allocator(al),
    super(args_list,bfm_allocator::member),
    node_count(0)
  {
    BOOST_MULTI_INDEX_CHECK_INVARIANT;
    try{
      typedef const Value* init_iterator;

      iterator hint=super::end();
      for(init_iterator first=list.begin(),last=list.end();
          first!=last;++first){
        hint=super::make_iterator(insert_(*first,hint.get_node()).first);
        ++hint;
      }
    }
    catch(...){
      clear_();
      throw;
    }
  }

  multi_index_container(
    const multi_index_container<Value,IndexSpecifierList,Allocator>& x):
    bfm_allocator(
      node_alloc_traits::select_on_container_copy_construction(
        x.bfm_allocator::member)),
    bfm_header(),
    super(x),
    node_count(0)
  {
    copy_construct_from(x);
  }

  multi_index_container(multi_index_container&& x):
    bfm_allocator(std::move(x.bfm_allocator::member)),
    bfm_header(),
    super(x,detail::do_not_copy_elements_tag()),
    node_count(0)
  {
    BOOST_MULTI_INDEX_CHECK_INVARIANT;
    BOOST_MULTI_INDEX_CHECK_INVARIANT_OF(x);
    swap_elements_(x);
  }

  multi_index_container(
    const multi_index_container<Value,IndexSpecifierList,Allocator>& x,
    const allocator_type& al):
    bfm_allocator(al),
    bfm_header(),
    super(x),
    node_count(0)
  {
    copy_construct_from(x);
  }

  multi_index_container(
    multi_index_container&& x,const allocator_type& al):
    bfm_allocator(al),
    bfm_header(),
    super(x,detail::do_not_copy_elements_tag()),
    node_count(0)
  {
    BOOST_MULTI_INDEX_CHECK_INVARIANT;
    BOOST_MULTI_INDEX_CHECK_INVARIANT_OF(x);

    if(al==x.get_allocator()){
      swap_elements_(x);
    }
    else{
      multi_index_container y(x,al,detail::unequal_alloc_move_ctor_tag());
      swap_elements_(y);
    }
  }

  ~multi_index_container()
  {
    delete_all_nodes_();
  }

  multi_index_container<Value,IndexSpecifierList,Allocator>& operator=(
    const multi_index_container& x)
  {
    multi_index_container y(
      x,
      node_alloc_traits::propagate_on_container_copy_assignment::value?
        x.get_allocator():this->get_allocator());
    swap_(y,std::true_type() /* swap_allocators */);
    return *this;
  }

  multi_index_container<Value,IndexSpecifierList,Allocator>& operator=(
    multi_index_container&& x)
  {
    if constexpr(
      node_alloc_traits::propagate_on_container_move_assignment::value){
      swap_(x,std::true_type() /* swap_allocators */);
    }
    else if(this->get_allocator()==x.get_allocator()){
      swap_(x,std::false_type() /* swap_allocators */);
    }
    else{
      multi_index_container y(std::move(x),this->get_allocator());
      swap_(y,std::false_type() /* swap_allocators */);
    }
    return *this;
  }

  multi_index_container<Value,IndexSpecifierList,Allocator>& operator=(
    std::initializer_list<Value> list)
  {
    BOOST_MULTI_INDEX_CHECK_INVARIANT;
    typedef const Value* init_iterator;

    multi_index_container x(*this,detail::do_not_copy_elements_tag());    
    iterator hint=x.end();
    for(init_iterator first=list.begin(),last=list.end();
        first!=last;++first){
      hint=x.make_iterator(x.insert_(*first,hint.get_node()).first);
      ++hint;
    }
    x.swap_elements_(*this);
    return*this;
  }

  allocator_type get_allocator()const noexcept
  {
    return allocator_type(bfm_allocator::member);
  }

  /* retrieval of indices by number */

  template<int N>
  struct nth_index
  {
    static_assert(N>=0&&N<mp11::mp_size<index_type_list>::value);
    typedef mp11::mp_at_c<index_type_list,N> type;
  };

  template<int N>
  typename nth_index<N>::type& get()noexcept
  {
    static_assert(N>=0&&N<mp11::mp_size<index_type_list>::value);
    return *this;
  }

  template<int N>
  const typename nth_index<N>::type& get()const noexcept
  {
    static_assert(N>=0&&N<mp11::mp_size<index_type_list>::value);
    return *this;
  }

  /* retrieval of indices by tag */

  template<typename Tag>
  struct index
  {
    typedef mp11::mp_find_if_q<
      index_type_list,
      typename detail::has_tag<Tag>
    >                                        pos;

    static const bool index_found=
        (pos::value<mp11::mp_size<index_type_list>::value);
    static_assert(index_found);

    typedef mp11::mp_at<index_type_list,pos> type;
  };

  template<typename Tag>
  typename index<Tag>::type& get()noexcept
  {
    return *this;
  }

  template<typename Tag>
  const typename index<Tag>::type& get()const noexcept
  {
    return *this;
  }

  /* projection of iterators by number */

  template<int N>
  struct nth_index_iterator
  {
    typedef typename nth_index<N>::type::iterator type;
  };

  template<int N>
  struct nth_index_const_iterator
  {
    typedef typename nth_index<N>::type::const_iterator type;
  };

  template<int N,typename IteratorType>
  typename nth_index_iterator<N>::type project(IteratorType it)
  {
    typedef typename nth_index<N>::type index_type;

    static_assert(
      (mp11::mp_contains<iterator_type_list,IteratorType>::value));

    BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(it);
    BOOST_MULTI_INDEX_CHECK_IS_OWNER(
      it,static_cast<typename IteratorType::container_type&>(*this));

    return index_type::make_iterator(
      static_cast<final_node_type*>(it.get_node()));
  }

  template<int N,typename IteratorType>
  typename nth_index_const_iterator<N>::type project(IteratorType it)const
  {
    typedef typename nth_index<N>::type index_type;

    static_assert((
      mp11::mp_contains<iterator_type_list,IteratorType>::value||
      mp11::mp_contains<const_iterator_type_list,IteratorType>::value));

    BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(it);
    BOOST_MULTI_INDEX_CHECK_IS_OWNER(
      it,static_cast<const typename IteratorType::container_type&>(*this));
    return index_type::make_iterator(
      static_cast<final_node_type*>(it.get_node()));
  }

  /* projection of iterators by tag */

  template<typename Tag>
  struct index_iterator
  {
    typedef typename index<Tag>::type::iterator type;
  };

  template<typename Tag>
  struct index_const_iterator
  {
    typedef typename index<Tag>::type::const_iterator type;
  };

  template<typename Tag,typename IteratorType>
  typename index_iterator<Tag>::type project(IteratorType it)
  {
    typedef typename index<Tag>::type index_type;

    static_assert(
      (mp11::mp_contains<iterator_type_list,IteratorType>::value));

    BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(it);
    BOOST_MULTI_INDEX_CHECK_IS_OWNER(
      it,static_cast<typename IteratorType::container_type&>(*this));
    return index_type::make_iterator(
      static_cast<final_node_type*>(it.get_node()));
  }

  template<typename Tag,typename IteratorType>
  typename index_const_iterator<Tag>::type project(IteratorType it)const
  {
    typedef typename index<Tag>::type index_type;

    static_assert((
      mp11::mp_contains<iterator_type_list,IteratorType>::value||
      mp11::mp_contains<const_iterator_type_list,IteratorType>::value));

    BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(it);
    BOOST_MULTI_INDEX_CHECK_IS_OWNER(
      it,static_cast<const typename IteratorType::container_type&>(*this));
    return index_type::make_iterator(
      static_cast<final_node_type*>(it.get_node()));
  }

BOOST_MULTI_INDEX_PROTECTED_IF_MEMBER_TEMPLATE_FRIENDS:
  typedef typename super::final_node_handle_type final_node_handle_type;
  typedef typename super::copy_map_type          copy_map_type;

  multi_index_container(
    multi_index_container<Value,IndexSpecifierList,Allocator>& x,
    const allocator_type& al,
    detail::unequal_alloc_move_ctor_tag):
    bfm_allocator(al),
    bfm_header(),
    super(x),
    node_count(0)
  {
    BOOST_MULTI_INDEX_CHECK_INVARIANT_OF(x);
    try{
      copy_map_type map(bfm_allocator::member,x.size(),x.header(),header());
      for(const_iterator it=x.begin(),it_end=x.end();it!=it_end;++it){
        map.move_clone(it.get_node());
      }
      super::copy_(x,map);
      map.release();
      node_count=x.size();
      x.clear();
    }
    catch(...){
      x.clear();
      throw;
    }

    /* Not until this point are the indices required to be consistent,
     * hence the position of the invariant checker.
     */

    BOOST_MULTI_INDEX_CHECK_INVARIANT;
  }

  multi_index_container(
    const multi_index_container<Value,IndexSpecifierList,Allocator>& x,
    detail::do_not_copy_elements_tag):
    bfm_allocator(x.bfm_allocator::member),
    bfm_header(),
    super(x,detail::do_not_copy_elements_tag()),
    node_count(0)
  {
    BOOST_MULTI_INDEX_CHECK_INVARIANT;
  }

  void copy_construct_from(
    const multi_index_container<Value,IndexSpecifierList,Allocator>& x)
  {
    copy_map_type map(bfm_allocator::member,x.size(),x.header(),header());
    for(const_iterator it=x.begin(),it_end=x.end();it!=it_end;++it){
      map.copy_clone(it.get_node());
    }
    super::copy_(x,map);
    map.release();
    node_count=x.size();

    /* Not until this point are the indices required to be consistent,
     * hence the position of the invariant checker.
     */

    BOOST_MULTI_INDEX_CHECK_INVARIANT;
  }

  final_node_type* header()const
  {
    return &*bfm_header::member;
  }

  final_node_type* allocate_node()
  {
    return &*node_alloc_traits::allocate(bfm_allocator::member,1);
  }

  void deallocate_node(final_node_type* x)
  {
    node_alloc_traits::deallocate(
      bfm_allocator::member,static_cast<node_pointer>(x),1);
  }

  void construct_value(final_node_type* x,const Value& v)
  {
    node_alloc_traits::construct(
      bfm_allocator::member,std::addressof(x->value()),v);
  }

  void construct_value(final_node_type* x,Value&& v)
  {
    node_alloc_traits::construct(
      bfm_allocator::member,std::addressof(x->value()),std::move(v));
  }

  template<typename... Args>
  void construct_value(final_node_type* x, Args&&... args) {
    return construct_value_impl(x, std::forward<Args>(args)...);
  }

  void destroy_value(final_node_type* x)
  {
    node_alloc_traits::destroy(
      bfm_allocator::member,std::addressof(x->value()));
  }

  bool empty_()const
  {
    return node_count==0;
  }

  size_type size_()const
  {
    return node_count;
  }

  size_type max_size_()const
  {
    return static_cast<size_type>(-1);
  }

  template<typename Variant>
  std::pair<final_node_type*,bool> insert_(const Value& v,Variant variant)
  {
    final_node_type* x=0;
    final_node_type* res=super::insert_(v,x,variant);
    if(res==x){
      ++node_count;
      return std::pair<final_node_type*,bool>(res,true);
    }
    else{
      return std::pair<final_node_type*,bool>(res,false);
    }
  }

  std::pair<final_node_type*,bool> insert_(const Value& v)
  {
    return insert_(v,detail::lvalue_tag());
  }

  std::pair<final_node_type*,bool> insert_rv_(const Value& v)
  {
    return insert_(v,detail::rvalue_tag());
  }

  template<typename T>
  std::pair<final_node_type*,bool> insert_ref_(T& t)
  {
    final_node_type* x=allocate_node();
    try{
      construct_value(x,t);
      try{
        final_node_type* res=super::insert_(
          x->value(),x,detail::emplaced_tag());
        if(res==x){
          ++node_count;
          return std::pair<final_node_type*,bool>(res,true);
        }
        else{
          delete_node_(x);
          return std::pair<final_node_type*,bool>(res,false);
        }
      }
      catch(...){
        destroy_value(x);
        throw;
      }
    }
    catch(...){
      deallocate_node(x);
      throw;
    }
  }

  std::pair<final_node_type*,bool> insert_ref_(const value_type& x)
  {
    return insert_(x);
  }

  std::pair<final_node_type*,bool> insert_ref_(value_type& x)
  {
    return insert_(x);
  }

  std::pair<final_node_type*,bool> insert_nh_(final_node_handle_type& nh)
  {
    if(!nh)return std::pair<final_node_type*,bool>(header(),false);
    else{
      final_node_type* x=nh.node;
      final_node_type* res=super::insert_(
        x->value(),x,detail::emplaced_tag());
      if(res==x){
        nh.release_node();
        ++node_count;
        return std::pair<final_node_type*,bool>(res,true);
      }
      else return std::pair<final_node_type*,bool>(res,false);
    }
  }

  template<typename... Args>
  std::pair<final_node_type*,bool> emplace_(Args&&... args)
  {
    final_node_type* x=allocate_node();
    try{
      construct_value(x,std::forward<Args>(args)...);
      try{
        final_node_type* res=super::insert_(
          x->value(),x,detail::emplaced_tag());
        if(res==x){
          ++node_count;
          return std::pair<final_node_type*,bool>(res,true);
        }
        else{
          delete_node_(x);
          return std::pair<final_node_type*,bool>(res,false);
        }
      }
      catch(...){
        destroy_value(x);
        throw;
      }
    }
    catch(...){
      deallocate_node(x);
      throw;
    }
  }

  template<typename Variant>
  std::pair<final_node_type*,bool> insert_(
    const Value& v,final_node_type* position,Variant variant)
  {
    final_node_type* x=0;
    final_node_type* res=super::insert_(v,position,x,variant);
    if(res==x){
      ++node_count;
      return std::pair<final_node_type*,bool>(res,true);
    }
    else{
      return std::pair<final_node_type*,bool>(res,false);
    }
  }

  std::pair<final_node_type*,bool> insert_(
    const Value& v,final_node_type* position)
  {
    return insert_(v,position,detail::lvalue_tag());
  }

  std::pair<final_node_type*,bool> insert_rv_(
    const Value& v,final_node_type* position)
  {
    return insert_(v,position,detail::rvalue_tag());
  }

  template<typename T>
  std::pair<final_node_type*,bool> insert_ref_(
    T& t,final_node_type* position)
  {
    final_node_type* x=allocate_node();
    try{
      construct_value(x,t);
      try{
        final_node_type* res=super::insert_(
          x->value(),position,x,detail::emplaced_tag());
        if(res==x){
          ++node_count;
          return std::pair<final_node_type*,bool>(res,true);
        }
        else{
          delete_node_(x);
          return std::pair<final_node_type*,bool>(res,false);
        }
      }
      catch(...){
        destroy_value(x);
        throw;
      }
    }
    catch(...){
      deallocate_node(x);
      throw;
    }
  }

  std::pair<final_node_type*,bool> insert_ref_(
    const value_type& x,final_node_type* position)
  {
    return insert_(x,position);
  }

  std::pair<final_node_type*,bool> insert_ref_(
    value_type& x,final_node_type* position)
  {
    return insert_(x,position);
  }

  std::pair<final_node_type*,bool> insert_nh_(
    final_node_handle_type& nh,final_node_type* position)
  {
    if(!nh)return std::pair<final_node_type*,bool>(header(),false);
    else{
      final_node_type* x=nh.node;
      final_node_type* res=super::insert_(
        x->value(),position,x,detail::emplaced_tag());
      if(res==x){
        nh.release_node();
        ++node_count;
        return std::pair<final_node_type*,bool>(res,true);
      }
      else return std::pair<final_node_type*,bool>(res,false);
    }
  }

  template<typename... Args>
  std::pair<final_node_type*,bool> emplace_hint_(
    final_node_type* position,Args&&... args)
  {
    final_node_type* x=allocate_node();
    try{
      construct_value(x,std::forward<Args>(args)...);
      try{
        final_node_type* res=super::insert_(
          x->value(),position,x,detail::emplaced_tag());
        if(res==x){
          ++node_count;
          return std::pair<final_node_type*,bool>(res,true);
        }
        else{
          delete_node_(x);
          return std::pair<final_node_type*,bool>(res,false);
        }
      }
      catch(...){
        destroy_value(x);
        throw;
      }
    }
    catch(...){
      deallocate_node(x);
      throw;
    }
  }

  final_node_handle_type extract_(final_node_type* x)
  {
    --node_count;
    super::extract_(x);
    return final_node_handle_type(x,get_allocator());
  }

  void erase_(final_node_type* x)
  {
    --node_count;
    super::extract_(x);
    delete_node_(x);
  }

  void delete_node_(final_node_type* x)
  {
    destroy_value(x);
    deallocate_node(x);
  }

  void delete_all_nodes_()
  {
    super::delete_all_nodes_();
  }

  void clear_()
  {
    delete_all_nodes_();
    super::clear_();
    node_count=0;
  }

  void swap_(multi_index_container<Value,IndexSpecifierList,Allocator>& x)
  {
    swap_(
      x,
      std::bool_constant<
        node_alloc_traits::propagate_on_container_swap::value>());
  }

  void swap_(
    multi_index_container<Value,IndexSpecifierList,Allocator>& x,
    std::true_type swap_allocators)
  {
    detail::adl_swap(bfm_allocator::member,x.bfm_allocator::member);
    std::swap(bfm_header::member,x.bfm_header::member);
    super::swap_(x,swap_allocators);
    std::swap(node_count,x.node_count);
  }

  void swap_(
    multi_index_container<Value,IndexSpecifierList,Allocator>& x,
    std::false_type swap_allocators)
  {
    std::swap(bfm_header::member,x.bfm_header::member);
    super::swap_(x,swap_allocators);
    std::swap(node_count,x.node_count);
  }

  void swap_elements_(
    multi_index_container<Value,IndexSpecifierList,Allocator>& x)
  {
    std::swap(bfm_header::member,x.bfm_header::member);
    super::swap_elements_(x);
    std::swap(node_count,x.node_count);
  }

  bool replace_(const Value& k,final_node_type* x)
  {
    return super::replace_(k,x,detail::lvalue_tag());
  }

  bool replace_rv_(const Value& k,final_node_type* x)
  {
    return super::replace_(k,x,detail::rvalue_tag());
  }

  template<typename Modifier>
  bool modify_(Modifier& mod,final_node_type* x)
  {
    try{
      mod(const_cast<value_type&>(x->value()));
    }
    catch(...){
      this->erase_(x);
      throw;
    }

    try{
      if(!super::modify_(x)){
        delete_node_(x);
        --node_count;
        return false;
      }
      else return true;
    }
    catch(...){
      delete_node_(x);
      --node_count;
      throw;
    }
  }

  template<typename Modifier,typename Rollback>
  bool modify_(Modifier& mod,Rollback& back_,final_node_type* x)
  {
    try{
      mod(const_cast<value_type&>(x->value()));
    }
    catch(...){
      this->erase_(x);
      throw;
    }

    bool b;
    try{
      b=super::modify_rollback_(x);
    }
    catch(...){
      try{
        back_(const_cast<value_type&>(x->value()));
        if(!super::check_rollback_(x))this->erase_(x);
        throw;
      }
      catch(...){
        this->erase_(x);
        throw;
      }
    }

    try{
      if(!b){
        back_(const_cast<value_type&>(x->value()));
        if(!super::check_rollback_(x))this->erase_(x);
        return false;
      }
      else return true;
    }
    catch(...){
      this->erase_(x);
      throw;
    }
  }

#if !defined(BOOST_MULTI_INDEX_DISABLE_SERIALIZATION)
  /* serialization */

  friend class boost::serialization::access;

  BOOST_SERIALIZATION_SPLIT_MEMBER()

  typedef typename super::index_saver_type        index_saver_type;
  typedef typename super::index_loader_type       index_loader_type;

  template<class Archive>
  void save(Archive& ar,const unsigned int version)const
  {
    const serialization::collection_size_type       s(size_());
    const detail::serialization_version<value_type> value_version;
    ar<<serialization::make_nvp("count",s);
    ar<<serialization::make_nvp("value_version",value_version);

    index_saver_type sm(bfm_allocator::member,s);

    for(iterator it=super::begin(),it_end=super::end();it!=it_end;++it){
      serialization::save_construct_data_adl(
        ar,std::addressof(*it),value_version);
      ar<<serialization::make_nvp("item",*it);
      sm.add(it.get_node(),ar,version);
    }
    sm.add_track(header(),ar,version);

    super::save_(ar,version,sm);
  }

  template<class Archive>
  void load(Archive& ar,const unsigned int version)
  {
    BOOST_MULTI_INDEX_CHECK_INVARIANT;

    clear_(); 
    serialization::collection_size_type       s;
    detail::serialization_version<value_type> value_version;
    if(version<1){
      std::size_t sz;
      ar>>serialization::make_nvp("count",sz);
      s=static_cast<serialization::collection_size_type>(sz);
    }
    else{
      ar>>serialization::make_nvp("count",s);
    }
    if(version<2){
      value_version=0;
    }
    else{
      ar>>serialization::make_nvp("value_version",value_version);
    }

    index_loader_type lm(bfm_allocator::member,s);

    for(std::size_t n=0;n<s;++n){
      detail::archive_constructed<Value> value("item",ar,value_version);
      std::pair<final_node_type*,bool> p=insert_rv_(
        value.get(),super::end().get_node());
      if(!p.second)throw_exception(
        archive::archive_exception(
          archive::archive_exception::other_exception));
      ar.reset_object_address(
        std::addressof(p.first->value()),std::addressof(value.get()));
      lm.add(p.first,ar,version);
    }
    lm.add_track(header(),ar,version);

    super::load_(ar,version,lm);
  }
#endif

#if defined(BOOST_MULTI_INDEX_ENABLE_INVARIANT_CHECKING)
  /* invariant stuff */

  bool invariant_()const
  {
    return super::invariant_();
  }

  void check_invariant_()const
  {
    BOOST_MULTI_INDEX_INVARIANT_ASSERT(invariant_());
  }
#endif

private:
  template<typename... Args>
  void construct_value_impl(final_node_type* x,Args&&... args)
  {
    node_alloc_traits::construct(
      bfm_allocator::member,std::addressof(x->value()),
      std::forward<Args>(args)...);
  }

  size_type node_count;
};

/* retrieval of indices by number */

template<typename MultiIndexContainer,int N>
struct nth_index
{
  static const int M=
      mp11::mp_size<typename MultiIndexContainer::index_type_list>::value;
  static_assert(N>=0&&N<M);
  typedef mp11::mp_at_c<
    typename MultiIndexContainer::index_type_list,N> type;
};

template<int N,typename Value,typename IndexSpecifierList,typename Allocator>
typename nth_index<
  multi_index_container<Value,IndexSpecifierList,Allocator>,N>::type&
get(
  multi_index_container<Value,IndexSpecifierList,Allocator>& m)noexcept
{
  typedef multi_index_container<
    Value,IndexSpecifierList,Allocator>    multi_index_type;
  typedef typename nth_index<
    multi_index_container<
      Value,IndexSpecifierList,Allocator>,
    N
  >::type                                  index_type;

  static_assert(N>=0&&
    N<
    mp11::mp_size<
      typename multi_index_type::index_type_list
    >::value);

  return detail::converter<multi_index_type,index_type>::index(m);
}

template<int N,typename Value,typename IndexSpecifierList,typename Allocator>
const typename nth_index<
  multi_index_container<Value,IndexSpecifierList,Allocator>,N>::type&
get(
  const multi_index_container<Value,IndexSpecifierList,Allocator>& m
)noexcept
{
  typedef multi_index_container<
    Value,IndexSpecifierList,Allocator>    multi_index_type;
  typedef typename nth_index<
    multi_index_container<
      Value,IndexSpecifierList,Allocator>,
    N
  >::type                                  index_type;

  static_assert(N>=0&&
    N<
    mp11::mp_size<
      typename multi_index_type::index_type_list
    >::value);

  return detail::converter<multi_index_type,index_type>::index(m);
}

/* retrieval of indices by tag */

template<typename MultiIndexContainer,typename Tag>
struct index
{
  typedef typename MultiIndexContainer::index_type_list index_type_list;

  typedef mp11::mp_find_if_q<
    index_type_list,
    typename detail::has_tag<Tag>
  >                                        pos;

  static const bool index_found=
      (pos::value<mp11::mp_size<index_type_list>::value);
  static_assert(index_found);

  typedef mp11::mp_at<index_type_list,pos> type;
};

template<
  typename Tag,typename Value,typename IndexSpecifierList,typename Allocator
>
typename ::boost::multi_index::index<
  multi_index_container<Value,IndexSpecifierList,Allocator>,Tag>::type&
get(
  multi_index_container<Value,IndexSpecifierList,Allocator>& m)noexcept
{
  typedef multi_index_container<
    Value,IndexSpecifierList,Allocator>         multi_index_type;
  typedef typename ::boost::multi_index::index<
    multi_index_container<
      Value,IndexSpecifierList,Allocator>,
    Tag
  >::type                                       index_type;

  return detail::converter<multi_index_type,index_type>::index(m);
}

template<
  typename Tag,typename Value,typename IndexSpecifierList,typename Allocator
>
const typename ::boost::multi_index::index<
  multi_index_container<Value,IndexSpecifierList,Allocator>,Tag>::type&
get(
  const multi_index_container<Value,IndexSpecifierList,Allocator>& m
)noexcept
{
  typedef multi_index_container<
    Value,IndexSpecifierList,Allocator>         multi_index_type;
  typedef typename ::boost::multi_index::index<
    multi_index_container<
      Value,IndexSpecifierList,Allocator>,
    Tag
  >::type                                       index_type;

  return detail::converter<multi_index_type,index_type>::index(m);
}

/* projection of iterators by number */

template<typename MultiIndexContainer,int N>
struct nth_index_iterator
{
  typedef typename nth_index<MultiIndexContainer,N>::type::iterator type;
};

template<typename MultiIndexContainer,int N>
struct nth_index_const_iterator
{
  typedef typename nth_index<MultiIndexContainer,N>::type::const_iterator type;
};

template<
  int N,typename IteratorType,
  typename Value,typename IndexSpecifierList,typename Allocator>
typename nth_index_iterator<
  multi_index_container<Value,IndexSpecifierList,Allocator>,N>::type
project(
  multi_index_container<Value,IndexSpecifierList,Allocator>& m,
  IteratorType it)
{
  typedef multi_index_container<
    Value,IndexSpecifierList,Allocator>                multi_index_type;
  typedef typename nth_index<multi_index_type,N>::type index_type;

#if !defined(__SUNPRO_CC)||!(__SUNPRO_CC<0x580) /* Sun C++ 5.7 fails */
  static_assert((
    mp11::mp_contains<
      typename multi_index_type::iterator_type_list,
      IteratorType>::value));
#endif

  BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(it);

#if defined(BOOST_MULTI_INDEX_ENABLE_SAFE_MODE)
  typedef detail::converter<
    multi_index_type,
    typename IteratorType::container_type> converter;
  BOOST_MULTI_INDEX_CHECK_IS_OWNER(it,converter::index(m));
#endif

  return detail::converter<multi_index_type,index_type>::iterator(
    m,static_cast<typename multi_index_type::final_node_type*>(it.get_node()));
}

template<
  int N,typename IteratorType,
  typename Value,typename IndexSpecifierList,typename Allocator>
typename nth_index_const_iterator<
  multi_index_container<Value,IndexSpecifierList,Allocator>,N>::type
project(
  const multi_index_container<Value,IndexSpecifierList,Allocator>& m,
  IteratorType it)
{
  typedef multi_index_container<
    Value,IndexSpecifierList,Allocator>                multi_index_type;
  typedef typename nth_index<multi_index_type,N>::type index_type;

#if !defined(__SUNPRO_CC)||!(__SUNPRO_CC<0x580) /* Sun C++ 5.7 fails */
  static_assert((
    mp11::mp_contains<
      typename multi_index_type::iterator_type_list,
      IteratorType>::value||
    mp11::mp_contains<
      typename multi_index_type::const_iterator_type_list,
      IteratorType>::value));
#endif

  BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(it);

#if defined(BOOST_MULTI_INDEX_ENABLE_SAFE_MODE)
  typedef detail::converter<
    multi_index_type,
    typename IteratorType::container_type> converter;
  BOOST_MULTI_INDEX_CHECK_IS_OWNER(it,converter::index(m));
#endif

  return detail::converter<multi_index_type,index_type>::const_iterator(
    m,static_cast<typename multi_index_type::final_node_type*>(it.get_node()));
}

/* projection of iterators by tag */

template<typename MultiIndexContainer,typename Tag>
struct index_iterator
{
  typedef typename ::boost::multi_index::index<
    MultiIndexContainer,Tag>::type::iterator    type;
};

template<typename MultiIndexContainer,typename Tag>
struct index_const_iterator
{
  typedef typename ::boost::multi_index::index<
    MultiIndexContainer,Tag>::type::const_iterator type;
};

template<
  typename Tag,typename IteratorType,
  typename Value,typename IndexSpecifierList,typename Allocator>
typename index_iterator<
  multi_index_container<Value,IndexSpecifierList,Allocator>,Tag>::type
project(
  multi_index_container<Value,IndexSpecifierList,Allocator>& m,
  IteratorType it)
{
  typedef multi_index_container<
    Value,IndexSpecifierList,Allocator>         multi_index_type;
  typedef typename ::boost::multi_index::index<
    multi_index_type,Tag>::type                 index_type;

#if !defined(__SUNPRO_CC)||!(__SUNPRO_CC<0x580) /* Sun C++ 5.7 fails */
  static_assert((
    mp11::mp_contains<
      typename multi_index_type::iterator_type_list,
      IteratorType>::value));
#endif

  BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(it);

#if defined(BOOST_MULTI_INDEX_ENABLE_SAFE_MODE)
  typedef detail::converter<
    multi_index_type,
    typename IteratorType::container_type> converter;
  BOOST_MULTI_INDEX_CHECK_IS_OWNER(it,converter::index(m));
#endif

  return detail::converter<multi_index_type,index_type>::iterator(
    m,static_cast<typename multi_index_type::final_node_type*>(it.get_node()));
}

template<
  typename Tag,typename IteratorType,
  typename Value,typename IndexSpecifierList,typename Allocator>
typename index_const_iterator<
  multi_index_container<Value,IndexSpecifierList,Allocator>,Tag>::type
project(
  const multi_index_container<Value,IndexSpecifierList,Allocator>& m,
  IteratorType it)
{
  typedef multi_index_container<
    Value,IndexSpecifierList,Allocator>         multi_index_type;
  typedef typename ::boost::multi_index::index<
    multi_index_type,Tag>::type                 index_type;

#if !defined(__SUNPRO_CC)||!(__SUNPRO_CC<0x580) /* Sun C++ 5.7 fails */
  static_assert((
    mp11::mp_contains<
      typename multi_index_type::iterator_type_list,
      IteratorType>::value||
    mp11::mp_contains<
      typename multi_index_type::const_iterator_type_list,
      IteratorType>::value));
#endif

  BOOST_MULTI_INDEX_CHECK_VALID_ITERATOR(it);

#if defined(BOOST_MULTI_INDEX_ENABLE_SAFE_MODE)
  typedef detail::converter<
    multi_index_type,
    typename IteratorType::container_type> converter;
  BOOST_MULTI_INDEX_CHECK_IS_OWNER(it,converter::index(m));
#endif

  return detail::converter<multi_index_type,index_type>::const_iterator(
    m,static_cast<typename multi_index_type::final_node_type*>(it.get_node()));
}

/* Comparison. Simple forward to first index. */

template<
  typename Value1,typename IndexSpecifierList1,typename Allocator1,
  typename Value2,typename IndexSpecifierList2,typename Allocator2
>
bool operator==(
  const multi_index_container<Value1,IndexSpecifierList1,Allocator1>& x,
  const multi_index_container<Value2,IndexSpecifierList2,Allocator2>& y)
{
  return get<0>(x)==get<0>(y);
}

template<
  typename Value1,typename IndexSpecifierList1,typename Allocator1,
  typename Value2,typename IndexSpecifierList2,typename Allocator2
>
bool operator<(
  const multi_index_container<Value1,IndexSpecifierList1,Allocator1>& x,
  const multi_index_container<Value2,IndexSpecifierList2,Allocator2>& y)
{
  return get<0>(x)<get<0>(y);
}

template<
  typename Value1,typename IndexSpecifierList1,typename Allocator1,
  typename Value2,typename IndexSpecifierList2,typename Allocator2
>
bool operator!=(
  const multi_index_container<Value1,IndexSpecifierList1,Allocator1>& x,
  const multi_index_container<Value2,IndexSpecifierList2,Allocator2>& y)
{
  return get<0>(x)!=get<0>(y);
}

template<
  typename Value1,typename IndexSpecifierList1,typename Allocator1,
  typename Value2,typename IndexSpecifierList2,typename Allocator2
>
bool operator>(
  const multi_index_container<Value1,IndexSpecifierList1,Allocator1>& x,
  const multi_index_container<Value2,IndexSpecifierList2,Allocator2>& y)
{
  return get<0>(x)>get<0>(y);
}

template<
  typename Value1,typename IndexSpecifierList1,typename Allocator1,
  typename Value2,typename IndexSpecifierList2,typename Allocator2
>
bool operator>=(
  const multi_index_container<Value1,IndexSpecifierList1,Allocator1>& x,
  const multi_index_container<Value2,IndexSpecifierList2,Allocator2>& y)
{
  return get<0>(x)>=get<0>(y);
}

template<
  typename Value1,typename IndexSpecifierList1,typename Allocator1,
  typename Value2,typename IndexSpecifierList2,typename Allocator2
>
bool operator<=(
  const multi_index_container<Value1,IndexSpecifierList1,Allocator1>& x,
  const multi_index_container<Value2,IndexSpecifierList2,Allocator2>& y)
{
  return get<0>(x)<=get<0>(y);
}

/* specialized algorithms */

template<typename Value,typename IndexSpecifierList,typename Allocator>
void swap(
  multi_index_container<Value,IndexSpecifierList,Allocator>& x,
  multi_index_container<Value,IndexSpecifierList,Allocator>& y)
{
  x.swap(y);
}

} // multi_index

#if !defined(BOOST_MULTI_INDEX_DISABLE_SERIALIZATION)
/* class version = 1 : we now serialize the size through
 * boost::serialization::collection_size_type.
 * class version = 2 : proper use of {save|load}_construct_data.
 */

namespace serialization {
template<typename Value,typename IndexSpecifierList,typename Allocator>
struct version<
  boost::multi_index_container<Value,IndexSpecifierList,Allocator>
>
{
  static const int value=2;
};
} // serialization
#endif

/* Associated global functions are promoted to namespace boost, except
 * comparison operators and swap, which are meant to be Koenig looked-up.
 */

using multi_index::get;
using multi_index::project;

} // boost

#undef BOOST_MULTI_INDEX_CHECK_INVARIANT
#undef BOOST_MULTI_INDEX_CHECK_INVARIANT_OF

#endif
