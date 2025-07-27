#pragma once
#include <optional>
#include <string>

#include "rule.h"

template <char C> struct Lit : Rule {
  using AttrType = char;
  using IterType = std::string::iterator;
  static std::optional<AttrType> match(IterType &b, IterType e) {
    if (b != e && *b == C) {
      ++b;      // Consume the character 'a'
      return C; // Return the matched character
    }
    return std::nullopt; // No match
  }
};
