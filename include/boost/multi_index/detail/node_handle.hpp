/* Copyright 2003-2020 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef BOOST_MULTI_INDEX_DETAIL_NODE_HANDLE_HPP
#define BOOST_MULTI_INDEX_DETAIL_NODE_HANDLE_HPP
#pragma once

#include <boost/multi_index_container_fwd.hpp>
#include <boost/multi_index/detail/allocator_traits.hpp>
#include <algorithm>
#include <type_traits>
#include <new>
#include <memory>

namespace boost{

namespace multi_index{

namespace detail{

/* Node handle template class following [container.node] specs.
 */

template<typename Node,typename Allocator>
class node_handle
{
public:
  typedef typename Node::value_type        value_type;
  typedef Allocator                        allocator_type;

private:
  typedef allocator_traits<allocator_type> alloc_traits;

public:
  node_handle()noexcept:node(0){}

  node_handle(node_handle&& x)noexcept:node(x.node)
  {
    if(!x.empty()){
      move_construct_allocator(std::move(x));
      x.destroy_allocator();
      x.node=0;
    }
  }

  ~node_handle()
  {
    if(!empty()){
      delete_node();
      destroy_allocator();
    }
  }

  node_handle& operator=(node_handle&& x)
  {
    if(this!=&x){
      if(!empty()){
        delete_node();
        if(!x.empty()){
          if constexpr(
            alloc_traits::propagate_on_container_move_assignment::value){
            move_assign_allocator(std::move(x));
          }
          x.destroy_allocator();
        }
        else{
          destroy_allocator();
        }
      }
      else if(!x.empty()){
        move_construct_allocator(std::move(x));
        x.destroy_allocator();
      }
      node=x.node;
      x.node=0;
    }
    return *this;
  }

  value_type& value()const{return node->value();}
  allocator_type get_allocator()const{return *allocator_ptr();}

  explicit operator bool()const noexcept{return (node!=0);}

  [[nodiscard]]
  bool empty()const noexcept{return (node==0);}

  void swap(node_handle& x) noexcept(
      alloc_traits::propagate_on_container_swap::value||
      alloc_traits::is_always_equal::value)
  {
    if(!empty()){
      if(!x.empty()){
        if constexpr(
          alloc_traits::propagate_on_container_swap::value){
          using std::swap;
          swap(*allocator_ptr(),*x.allocator_ptr());
        }
      }
      else{
        x.move_construct_allocator(std::move(*this));
        destroy_allocator();
      }
    }
    else if(!x.empty()){
      move_construct_allocator(std::move(x));
      x.destroy_allocator();
    }
    std::swap(node,x.node);
  }

  friend void swap(node_handle& x,node_handle& y) noexcept(noexcept(x.swap(y)))
  {
    x.swap(y);
  }

private:
  node_handle(const node_handle&) = delete;
  node_handle& operator=(const node_handle&) = delete;

  template <typename,typename,typename>
  friend class boost::multi_index::multi_index_container;

  node_handle(Node* node_,const allocator_type& al):node(node_)
  {
    ::new (static_cast<void*>(allocator_ptr())) allocator_type(al);
  }

  void release_node()
  {
    if(!empty()){
      node=0;
      destroy_allocator();
    }
  }

  const allocator_type* allocator_ptr()const
  {
    return reinterpret_cast<const allocator_type*>(&space);
  }

  allocator_type* allocator_ptr()
  {
    return reinterpret_cast<allocator_type*>(&space);
  }

  void move_construct_allocator(node_handle&& x)
  {
    ::new (static_cast<void*>(allocator_ptr()))
      allocator_type(std::move(*x.allocator_ptr()));
  }

  void move_assign_allocator(node_handle&& x)
  {
    *allocator_ptr()=std::move(*x.allocator_ptr());
  }

  void destroy_allocator(){allocator_ptr()->~allocator_type();}

  void delete_node()
  {
    typedef typename rebind_alloc_for<
      allocator_type,Node
    >::type                                          node_allocator;
    typedef detail::allocator_traits<node_allocator> node_alloc_traits;
    typedef typename node_alloc_traits::pointer      node_pointer;

    alloc_traits::destroy(*allocator_ptr(),std::addressof(node->value()));
    node_allocator nal(*allocator_ptr());
    node_alloc_traits::deallocate(nal,static_cast<node_pointer>(node),1);
  }

  Node*                                 node;
  typename std::aligned_storage_t<
    sizeof(allocator_type),
    std::alignment_of_v<allocator_type>
  >                                     space;
};

/* node handle insert return type template class following
 * [container.insert.return] specs.
 */

template<typename Iterator,typename NodeHandle>
struct insert_return_type
{
  insert_return_type(
    Iterator position_,bool inserted_,NodeHandle&& node_):
    position(position_),inserted(inserted_),node(std::move(node_)){}
  insert_return_type(insert_return_type&& x):
    position(x.position),inserted(x.inserted),node(std::move(x.node)){}

  insert_return_type& operator=(insert_return_type&& x)
  {
    position=x.position;
    inserted=x.inserted;
    node=std::move(x.node);
    return *this;
  }

  Iterator   position;
  bool       inserted;
  NodeHandle node;

private:
  insert_return_type(const insert_return_type&) = delete;
  insert_return_type& operator=(const insert_return_type&) = delete;
};

} /* namespace multi_index::detail */

} /* namespace multi_index */

} /* namespace boost */

#endif
