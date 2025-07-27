#pragma once
#include <functional>

struct Context {
  // Placeholder for context data
};

using SemanticAction = std::function<void(Context &)>;

struct Rule {
  SemanticAction action;
  void operator[](SemanticAction &&a) {
    action = std::forward<SemanticAction>(a);
  }
};
