//  (C) Copyright 2015 - 2017 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef VISIT_STRUCT_BOOST_FUSION_HPP_INCLUDED
#define VISIT_STRUCT_BOOST_FUSION_HPP_INCLUDED

#include <visit_struct/visit_struct.hpp>

#include <boost/mpl/range_c.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/zip.hpp>
#include <boost/fusion/include/at_c.hpp>
#include <boost/fusion/include/adapted.hpp>
#include <boost/fusion/include/mpl.hpp>

#include <utility>

namespace visit_struct {

namespace traits {

namespace fusion = boost::fusion;
namespace mpl = boost::mpl;

template <typename S>
struct visitable<S,
                 typename std::enable_if<
                   std::is_same<typename mpl::sequence_tag<S>::type,
                                fusion::fusion_sequence_tag
                   >::value
                 >::type>
{

private:
  // Accessor type for fusion structure S
  template <int idx>
  struct accessor {
    VISIT_STRUCT_CONSTEXPR auto operator()(S & s) const ->
    decltype(fusion::at_c<idx>(s)) {
      return fusion::at_c<idx>(s);
    }

    VISIT_STRUCT_CONSTEXPR auto operator()(const S & s) const ->
    decltype(fusion::at_c<idx>(s)) {
      return fusion::at_c<idx>(s);
    }

    VISIT_STRUCT_CONSTEXPR auto operator()(S && s) const ->
    decltype(std::move(fusion::at_c<idx>(s))) {
      return std::move(fusion::at_c<idx>(s));
    }
  };

  // T is a const / ref qualified version of S
  // V should be a forwarding reference here, we should not be copying visitors
  template <typename V, typename T>
  struct fusion_visitor {
    V visitor;
    T struct_instance;

    explicit fusion_visitor(V v, T t) : visitor(std::forward<V>(v)), struct_instance(std::forward<T>(t)) {}

    template <typename Index>
    VISIT_STRUCT_CXX14_CONSTEXPR void operator()(Index) const {
      using accessor_t = accessor<Index::value>;
      std::forward<V>(visitor)(fusion::extension::struct_member_name<S, Index::value>::call(), accessor_t()(std::forward<T>(struct_instance)));
    }
  };

  template <typename V>
  struct fusion_visitor_types {
    V visitor;

    explicit fusion_visitor_types(V v) : visitor(std::forward<V>(v)) {}

    template <typename Index>
    VISIT_STRUCT_CXX14_CONSTEXPR void operator()(Index) const {
      using current_type = typename fusion::result_of::value_at<S, Index>::type;
      std::forward<V>(visitor)(fusion::extension::struct_member_name<S, Index::value>::call(), visit_struct::type_c<current_type>{});
    }
  };

  template <typename V>
  struct fusion_visitor_accessors {
    V visitor;

    explicit fusion_visitor_accessors(V v) : visitor(std::forward<V>(v)) {}

    template <typename Index>
    VISIT_STRUCT_CXX14_CONSTEXPR void operator()(Index) const {
      using accessor_t = accessor<Index::value>;
      std::forward<V>(visitor)(fusion::extension::struct_member_name<S, Index::value>::call(), accessor_t());
    }
  };

public:
  static VISIT_STRUCT_CONSTEXPR const size_t field_count = fusion::result_of::size<S>::value;

  template <typename V>
  static void apply(V && v, const S & s) {
    using Indices = mpl::range_c<unsigned, 0, fusion::result_of::size<S>::value >;
    using fv_t = fusion_visitor<decltype(std::forward<V>(v)), const S &>;
    fv_t fv{std::forward<V>(v), s};
    fusion::for_each(Indices(), fv);
  }

  template <typename V>
  static void apply(V && v, S & s) {
    using Indices = mpl::range_c<unsigned, 0, fusion::result_of::size<S>::value >;
    using fv_t = fusion_visitor<decltype(std::forward<V>(v)), S &>;
    fv_t fv{std::forward<V>(v), s};
    fusion::for_each(Indices(), fv);
  }

  template <typename V>
  static void apply(V && v, S && s) {
    using Indices = mpl::range_c<unsigned, 0, fusion::result_of::size<S>::value >;
    using fv_t = fusion_visitor<decltype(std::forward<V>(v)), S &&>;
    fv_t fv{std::forward<V>(v), std::move(s)};
    fusion::for_each(Indices(), fv);
  }

  template <typename V>
  static void visit_types(V && v) {
    using Indices = mpl::range_c<unsigned, 0, fusion::result_of::size<S>::value >;
    using fv_t = fusion_visitor_types<decltype(std::forward<V>(v))>;
    fv_t fv{std::forward<V>(v)};
    fusion::for_each(Indices(), fv);
  }

  template <typename V>
  static void visit_accessors(V && v) {
    using Indices = mpl::range_c<unsigned, 0, fusion::result_of::size<S>::value >;
    using fv_t = fusion_visitor_accessors<decltype(std::forward<V>(v))>;
    fv_t fv{std::forward<V>(v)};
    fusion::for_each(Indices(), fv);
  }


  template <int idx>
  static VISIT_STRUCT_CONSTEXPR auto get_value(std::integral_constant<int, idx>, S & s)
    -> decltype(fusion::at_c<idx>(s))
  {
    return fusion::at_c<idx>(s);
  }

  template <int idx>
  static VISIT_STRUCT_CONSTEXPR auto get_value(std::integral_constant<int, idx>, const S & s)
    -> decltype(fusion::at_c<idx>(s))
  {
    return fusion::at_c<idx>(s);
  }

  template <int idx>
  static VISIT_STRUCT_CONSTEXPR auto get_value(std::integral_constant<int, idx>, S && s)
    -> decltype(std::move(fusion::at_c<idx>(s)))
  {
    return std::move(fusion::at_c<idx>(s));
  }

  template <int idx>
  static VISIT_STRUCT_CONSTEXPR const char * get_name(std::integral_constant<int, idx>) {
    return fusion::extension::struct_member_name<S, idx>::call();
  }

  template <int idx>
  static VISIT_STRUCT_CONSTEXPR auto get_accessor(std::integral_constant<int, idx>) ->
    accessor<idx>
  {
    return {};
  }

  static VISIT_STRUCT_CONSTEXPR const bool value = true;
};

} // end namespace traits

} // end namespace visit_struct

#endif // VISIT_STRUCT_BOOST_FUSION_HPP_INCLUDED
