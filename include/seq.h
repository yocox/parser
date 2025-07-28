#pragma once
#include "rule.h"
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>

template <typename Head, typename... Tail>
std::tuple<Tail...> tuple_tail(std::tuple<Head, Tail...> &t) {
  return std::apply(
      [](auto &&, auto &&...tail) {
        return std::make_tuple(std::forward<decltype(tail)>(tail)...);
      },
      std::forward<std::tuple<Head, Tail...>>(t));
}

template <typename Head, typename... Tail> struct Seq : Rule {
  using AttrType =
      std::tuple<typename Head::AttrType, typename Tail::AttrType...>;
  using RuleType = std::tuple<Head, Tail...>;
  using RuleRefType = std::tuple<Head &, Tail &...>;
  using IterType = std::string::iterator;
  RuleType rules;
  Seq(const Head &head, const Tail &...tail) : rules{head, tail...} {}

  std::optional<AttrType> match(IterType &b, IterType e) {
    IterType orig_b = b;
    auto head_result = std::get<0>(rules).match(b, e);
    if (!head_result) {
      b = orig_b; // Reset iterator if match fails
      return std::nullopt;
    }
    if constexpr (sizeof...(Tail) == 0) {
      return std::make_tuple(
          std::move(*head_result)); // Return the matched head as a tuple
    } else {
      auto rule_tail = tuple_tail(rules);
      auto seq_tail =
          std::make_from_tuple<Seq<std::decay_t<Tail>...>>(rule_tail);

      auto tail_result = seq_tail.match(b, e);
      if (!tail_result) {
        b = orig_b; // Reset iterator if match fails
        return std::nullopt;
      }
      return std::tuple_cat(std::make_tuple(std::move(*head_result)),
                            std::move(*tail_result));
    }
  }
};

// Operator >> to construct a Seq from multiple rules, only for Rule-derived
// types

template <typename T>
concept IsRule = std::is_base_of_v<Rule, std::decay_t<T>>;

template <IsRule Head, IsRule Tail>
Seq<Head, Tail> operator>>(const Head &head, const Tail &tail) {
  return Seq<typename std::decay_t<Head>, typename std::decay_t<Tail>>(head,
                                                                       tail);
}

template <IsRule Tail, IsRule... Head>
Seq<Head..., Tail> operator>>(const Seq<Head...> &head, const Tail &tail) {
  return std::apply(
      [&](auto &&...head_rules) {
        return Seq<typename std::decay_t<decltype(head_rules)>...,
                   typename std::decay_t<Tail>>(
            std::forward<decltype(head_rules)>(head_rules)..., tail);
      },
      head.rules);
}

template <IsRule Head, IsRule... Tail>
Seq<Head, Tail...> operator>>(const Head &head, const Seq<Tail...> &tail) {
  return std::apply(
      [&](auto &&...tail_rules) {
        return Seq<typename std::decay_t<Head>,
                   typename std::decay_t<decltype(tail_rules)>...>(
            head, std::forward<decltype(tail_rules)>(tail_rules)...);
      },
      tail.rules);
}

template <IsRule... Head, IsRule... Tail>
Seq<Head..., Tail...> operator>>(const Seq<Head...> &head,
                                 const Seq<Tail...> &tail) {
  return std::apply(
      [&](auto &&...head_rules) {
        return std::apply(
            [&](auto &&...tail_rules) {
              return Seq<typename std::decay_t<decltype(head_rules)>...,
                         typename std::decay_t<decltype(tail_rules)>...>(
                  std::forward<decltype(head_rules)>(head_rules)...,
                  std::forward<decltype(tail_rules)>(tail_rules)...);
            },
            tail.rules);
      },
      head.rules);
}
