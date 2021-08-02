//  tuple.hpp - Boost Tuple Library --------------------------------------

// Copyright (C) 1999, 2000 Jaakko Jarvi (jaakko.jarvi@cs.utu.fi)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

// -----------------------------------------------------------------

#ifndef BOOST_MULTI_INDEX_CONS_TUPLE_HPP
#define BOOST_MULTI_INDEX_CONS_TUPLE_HPP
#pragma once

#include <tuple>
#include <type_traits>
#include <utility>
#include <cstddef>

namespace boost::multi_index {

struct cons_null { };

template <class HT, class TT> struct cons;

template <typename... Types> class cons_tuple;

template<class T> struct cons_size;

template<class T>
inline constexpr std::size_t cons_size_v = cons_size<T>::value;

namespace detail {

// -- generate error template, referencing to non-existing members of this
// template is used to produce compilation errors intentionally
template<class T> class generate_error;

template<std::size_t N>
struct drop_front {
  template<class Tuple>
  struct apply {
    using next = typename drop_front<N-1>::template apply<Tuple>;
    using type = typename next::type::tail_type;
    static const type& call(const Tuple& tup)
    {
      return next::call(tup).tail;
    }
  };
}; // drop_front<N>

template<>
struct drop_front<0> {
  template<class Tuple>
  struct apply {
    using type = Tuple;
    static const type& call(const Tuple& tup) {
      return tup;
    }
  };
}; // drop_front<0>

} // detail

// -cons type accessors ----------------------------------------
// typename cons_element<N,T>::type gets the type of the
// Nth element of T, first element is at index 0
// -------------------------------------------------------

template<std::size_t N, class T>
struct cons_element {
  using type = typename detail::drop_front<N>::template
                  apply<T>::type::head_type;
}; // cons_element<N, T>

template<std::size_t N, class T>
struct cons_element<N, const T> {
private:
  using unqualified_type = typename detail::drop_front<N>::template
                              apply<T>::type::head_type;
public:
  using type = std::add_const_t<unqualified_type>;
}; // cons_element<N, const T>

// -get function templates -----------------------------------------------
// Usage: get<N>(aTuple)

// -- some traits classes for get functions

// Access traits lifted from detail namespace to be part of the interface,
// (Joel de Guzman's suggestion). Rationale: get functions are part of the
// interface, so should the way to express their return types be.

template <class T>
struct access_traits {
  using const_type     = const T&;
  using non_const_type = T&;
  using parameter_type = const std::remove_cv_t<T>&;

// Used as the cons_tuple constructors parameter types
// Rationale: non-reference cons_tuple element types can be cv-qualified.
// It should be possible to initialize such types with temporaries,
// and when binding temporaries to references, the reference must
// be non-volatile and const. 8.5.3. (5)
}; // access_traits<T>

template <class T>
struct access_traits<T&> {
  using const_type     = T&;
  using non_const_type = T&;
  using parameter_type = T&;
}; // access_traits<T&>

// get function for non-const cons-lists, returns a reference to the element

template<std::size_t N, class HT, class TT>
inline typename access_traits<
                  typename cons_element<N, cons<HT, TT>>::type
                >::non_const_type
get(cons<HT, TT>& c) {
  using impl = typename detail::drop_front<N>::template
                  apply<cons<HT, TT>>;
  using cons_element = typename impl::type;
  return const_cast<cons_element&>(impl::call(c)).head;
} // get non-const

// get function for const cons-lists, returns a const reference to
// the element. If the element is a reference, returns the reference
// as such (that is, can return a non-const reference).
template<std::size_t N, class HT, class TT>
inline typename access_traits<
                  typename cons_element<N, cons<HT, TT>>::type
                >::const_type
get(const cons<HT, TT>& c) {
  using impl = typename detail::drop_front<N>::template
                  apply<cons<HT, TT>>;
  return impl::call(c).head;
} // get const

// -- the cons template  --------------------------------------------------
namespace detail {

//  These helper templates wrap void types and plain function types.
//  The reationale is to allow one to write cons_tuple types with those types
//  as elements, even though it is not possible to instantiate such object.
//  E.g: using some_type = cons_tuple<void>; // ok
//  but: some_type x; // fails

template <class T>
class non_storeable_type {
  non_storeable_type();
};

template <class T> struct wrap_non_storeable_type {
  using type = std::conditional_t<
    std::is_function_v<T>, non_storeable_type<T>, T
  >;
};

template <>
struct wrap_non_storeable_type<void> {
  using type = non_storeable_type<void>;
};

} // detail

template <class HT, class TT>
struct cons {

  using head_type = HT;
  using tail_type = TT;

  using stored_head_type = typename
      detail::wrap_non_storeable_type<head_type>::type;

  stored_head_type head;
  tail_type        tail;

  typename access_traits<stored_head_type>::non_const_type
  get_head() { return head; }

  typename access_traits<tail_type>::non_const_type
  get_tail() { return tail; }

  typename access_traits<stored_head_type>::const_type
  get_head() const { return head; }

  typename access_traits<tail_type>::const_type
  get_tail() const { return tail; }

  cons() : head(), tail() {}
  // cons() : head(detail::default_arg<HT>::f()), tail() {}

  // The argument for head is not strictly needed, but it prevents
  // array type elements. This is good, since array type elements
  // cannot be supported properly in any case (no assignment,
  // copy works only if the tails are exactly the same type, ...)

  cons(typename access_traits<stored_head_type>::parameter_type h,
       const tail_type& t)
    : head(h), tail(t) {}

  template <typename T1, typename... Types>
  cons(T1& t1, Types&... args)
    : head(t1), tail(args...)
    {}

  template <typename... Types>
  cons(const cons_null& /*t1*/, Types&... args)
    : head(), tail(args...)
    {}

  cons(const cons& u) : head(u.head), tail(u.tail) {}

  template <class HT2, class TT2>
  cons(const cons<HT2, TT2>& u) : head(u.head), tail(u.tail) {}

  template <class HT2, class TT2>
  cons& operator=(const cons<HT2, TT2>& u) {
    head = u.head;
    tail = u.tail;
    return *this;
  }

  // must define assignment operator explicitly, implicit version is
  // illformed if HT is a reference (12.8. (12))
  cons& operator=(const cons& u) {
    head = u.head;
    tail = u.tail;
    return *this;
  }

  template <class T1, class T2>
  cons& operator=(const std::pair<T1, T2>& u) {
    static_assert(cons_size_v<cons> == 2); // check length = 2
    head = u.first;
    tail.head = u.second;
    return *this;
  }

  // get member functions (non-const and const)
  template <std::size_t N>
  typename access_traits<
             typename cons_element<N, cons<HT, TT>>::type
           >::non_const_type
  get() {
    return boost::multi_index::get<N>(*this); // delegate to non-member get
  }

  template <std::size_t N>
  typename access_traits<
             typename cons_element<N, cons<HT, TT>>::type
           >::const_type
  get() const {
    return boost::multi_index::get<N>(*this); // delegate to non-member get
  }
}; // cons<class HT, class TT>

template <class HT>
struct cons<HT, cons_null> {

  using head_type = HT;
  using tail_type = cons_null;
  using self_type = cons<HT, cons_null>;

  using stored_head_type =
                    typename detail::wrap_non_storeable_type<head_type>::type;

  stored_head_type head;

  typename access_traits<stored_head_type>::non_const_type
  get_head() { return head; }

  cons_null get_tail() { return cons_null(); }

  typename access_traits<stored_head_type>::const_type
  get_head() const { return head; }

  const cons_null get_tail() const { return cons_null(); }

  //  cons() : head(detail::default_arg<HT>::f()) {}
  cons() : head() {}

  cons(const cons_null&, ...) : head() {}

  cons(typename access_traits<stored_head_type>::parameter_type h) : head(h) {}

  template<class T1>
  cons(T1& t1, ...) : head(t1) {}

  cons(const cons& u) : head(u.head) {}

  template <class HT2>
  cons(const cons<HT2, cons_null>& u) : head(u.head) {}

  template <class HT2>
  cons& operator=(const cons<HT2, cons_null>& u ) {
    head = u.head;
    return *this;
  }

  // Must define assignment operator explicitely. Implicit version
  // is illformed if HT is a reference.
  cons& operator=(const cons& u) { head = u.head; return *this; }

  template <std::size_t N>
  typename access_traits<
             typename cons_element<N, self_type>::type
            >::non_const_type
  get() {
    return boost::multi_index::get<N>(*this);
  }

  template <std::size_t N>
  typename access_traits<
             typename cons_element<N, self_type>::type
           >::const_type
  get() const {
    return boost::multi_index::get<N>(*this);
  }

}; // cons<HT, cons_null>

// templates for finding out the length of the cons_tuple -------------------

template<class T>
struct cons_size
  : std::integral_constant<std::size_t,
         1 + cons_size_v<typename T::tail_type>>
{ };

template<>
struct cons_size<cons_tuple<>> : std::integral_constant<std::size_t, 0>
{ };

template<>
struct cons_size<cons_tuple<> const> : std::integral_constant<std::size_t, 0>
{ };

template<>
struct cons_size<cons_null> : std::integral_constant<std::size_t, 0>
{ };

template<>
struct cons_size<const cons_null> : std::integral_constant<std::size_t, 0>
{ };

namespace detail {

// Tuple to cons mapper --------------------------------------------------
template <typename...> struct map_tuple_to_cons;

template <typename T0, typename... Types>
struct map_tuple_to_cons<T0, Types...> {
  using type = cons<T0, typename map_tuple_to_cons<Types...>::type>;
};

// The empty cons_tuple is a cons_null
template <>
struct map_tuple_to_cons<> {
  using type = cons_null;
};

} // detail

// -------------------------------------------------------------------
// -- cons_tuple ------------------------------------------------------
template <typename... Types>
class cons_tuple
  : public detail::map_tuple_to_cons<Types...>::type
{
public:
  using inherited = typename detail::map_tuple_to_cons<Types...>::type;
  using head_type = typename inherited::head_type;
  using tail_type = typename inherited::tail_type;

// access_traits<T>::parameter_type takes non-reference types as const T&
  cons_tuple() : inherited() {}

  explicit
  cons_tuple(typename access_traits<Types>::parameter_type... args)
    : inherited(args...) {}

  template<class U1, class U2>
  cons_tuple(const cons<U1, U2>& p) : inherited(p) {}

  template <class U1, class U2>
  cons_tuple& operator=(const cons<U1, U2>& k) {
    inherited::operator=(k);
    return *this;
  }

  template <class U1, class U2>
  cons_tuple& operator=(const std::pair<U1, U2>& k) {
    static_assert(cons_size_v<cons_tuple> == 2);// check_length = 2
    this->head = k.first;
    this->tail.head = k.second;
    return *this;
  }

}; // cons_tuple

// The empty cons_tuple
template <>
class cons_tuple<> : public cons_null {
public:
  using inherited = cons_null;
}; // cons_tuple<>

template<typename>
struct is_cons_tuple : std::false_type { };

template<typename... Types>
struct is_cons_tuple<cons_tuple<Types...>> : std::true_type { };

template <typename T>
inline constexpr bool is_cons_tuple_v = is_cons_tuple<T>::value;

// Swallows any assignment   (by Doug Gregor)
namespace detail {

struct swallow_assign;

using ignore_t = void (detail::swallow_assign::*)();

struct swallow_assign {
  swallow_assign(ignore_t(*)(ignore_t)) {}
  template<typename T>
  const swallow_assign& operator=(const T&) const { return *this; }
}; // swallow_assign

} // detail

// "ignore" allows cons_tuple positions to be ignored when using "tie".
inline detail::ignore_t ignore(detail::ignore_t) { return nullptr; }

// ---------------------------------------------------------------------------
// The call_traits for make_cons_tuple
// Honours the reference_wrapper class.

// Must be instantiated with plain or const plain types (not with references)

// from template<class T> foo(const T& t) : make_tuple_traits<const T>::type
// from template<class T> foo(T& t) : make_tuple_traits<T>::type

// Conversions:
// T -> T,
// references -> compile_time_error
// reference_wrapper<T> -> T&
// const reference_wrapper<T> -> T&
// array -> const ref array

template<class T>
struct make_tuple_traits {
  using type = T;

  // commented away, see below  (JJ)
  //  using type = typename IF<
  //  std::is_function_v<T>,
  //  T&,
  //  T>::type;

}; // make_tuple_traits

// The is_function test was there originally for plain function types,
// which can't be stored as such (we must either store them as references or
// pointers). Such a type could be formed if make_cons_tuple was called with a
// reference to a function.
// But this would mean that a const qualified function type was formed in
// the make_cons_tuple function and hence make_cons_tuple can't take a function
// reference as a parameter, and thus T can't be a function type.
// So is_function test was removed.
// (14.8.3. says that type deduction fails if a cv-qualified function type
// is created. (It only applies for the case of explicitly specifying template
// args, though?)) (JJ)

template<class T>
struct make_tuple_traits<T&> {
  using error = typename
     detail::generate_error<T&>::do_not_use_with_reference_type;
}; // make_tuple_traits<T>

// Arrays can't be stored as plain types; convert them to references.
// All arrays are converted to const. This is because make_cons_tuple takes its
// parameters as const T& and thus the knowledge of the potential
// non-constness of actual argument is lost.
template<class T, int n>
struct make_tuple_traits <T[n]> {
  using type = const T (&)[n];
};

template<class T, int n>
struct make_tuple_traits<const T[n]> {
  using type = const T (&)[n];
};

template<class T, int n>
struct make_tuple_traits<volatile T[n]> {
  using type = const volatile T (&)[n];
};

template<class T, int n>
struct make_tuple_traits<const volatile T[n]> {
  using type = const volatile T (&)[n];
};

template<class T>
struct make_tuple_traits<std::reference_wrapper<T>>{
  using type = T&;
};

template<class T>
struct make_tuple_traits<const std::reference_wrapper<T>>{
  using type = T&;
};

template<>
struct make_tuple_traits<detail::ignore_t(detail::ignore_t)> {
  using type = detail::swallow_assign;
};

namespace detail {

// A helper traits to make the make_cons_tuple functions shorter
// (Vesa Karvonen's suggestion)
template <typename... Types>
struct make_tuple_mapper {
  using type = cons_tuple<typename make_tuple_traits<Types>::type...>;
}; // make_tuple_mapper

} // detail

// -make_cons_tuple function templates -----------------------------------
inline cons_tuple<> make_cons_tuple() {
  return cons_tuple<>();
}

template<typename... Types>
inline typename detail::make_tuple_mapper<Types...>::type
make_cons_tuple(const Types&... args) {
  using t = typename detail::make_tuple_mapper<Types...>::type;
  return t(args...);
}

namespace detail {

template<class T>
struct tie_traits {
  using type = T&;
};

template<>
struct tie_traits<ignore_t(ignore_t)> {
  using type = swallow_assign;
};

template<>
struct tie_traits<void> {
  using type = cons_null;
};

template <typename... Types>
struct tie_mapper {
  using type = cons_tuple<typename tie_traits<Types>::type...>;
}; // tie_mapper

} // detail

// Tie function templates -------------------------------------------------
template<typename... Types>
inline typename detail::tie_mapper<Types...>::type
tie(Types&... args) {
  using t = typename detail::tie_mapper<Types...>::type;
  return t(args...);
}

template <typename... Types>
void swap(cons_tuple<Types...>& lhs, cons_tuple<Types...>& rhs);

inline void swap(cons_null&, cons_null&) {}

template<class HH>
inline void swap(cons<HH, cons_null>& lhs, cons<HH, cons_null>& rhs) {
  std::swap(lhs.head, rhs.head);
}

template<class HH, class TT>
inline void swap(cons<HH, TT>& lhs, cons<HH, TT>& rhs) {
  std::swap(lhs.head, rhs.head);
  boost::multi_index::swap(lhs.tail, rhs.tail);
}

template <typename... Types>
inline void swap(cons_tuple<Types...>& lhs, cons_tuple<Types...>& rhs) {
  using tuple_type = cons_tuple<Types...>;
  using base = typename tuple_type::inherited;
  boost::multi_index::swap(static_cast<base&>(lhs), static_cast<base&>(rhs));
}

} // boost::multi_index

namespace std {

template<typename... Types>
struct tuple_size<boost::multi_index::cons_tuple<Types...>>
  : boost::multi_index::cons_size<boost::multi_index::cons_tuple<Types...>>
{ };

template<size_t I, typename... Types>
struct tuple_element<I, boost::multi_index::cons_tuple<Types...>>
  : boost::multi_index::cons_element<I,
                                     boost::multi_index::cons_tuple<Types...>>
  { };

} // std

#endif
