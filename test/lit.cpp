#include "lit.h"

#include <gtest/gtest.h>
#include <optional>
#include <string>
#include <tuple>

TEST(LitTest, Basic) {
  Lit<'a'> lit_a = Lit<'a'>{};

  static_assert(std::is_same_v<Lit<'a'>::AttrType, char>,
                "Lit<'a'> should have AttrType as char");

  std::string input = "a";
  auto b = input.begin();
  auto e = input.end();
  auto r = lit_a.match(b, e);

  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(*r, 'a');
}
