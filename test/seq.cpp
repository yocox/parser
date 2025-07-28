#include "seq.h"
#include "lit.h"

#include <gtest/gtest.h>
#include <optional>
#include <string>
#include <tuple>

TEST(SeqTest, MatchSequence) {
  Seq<Lit<'a'>, Lit<'b'>, Lit<'c'>> seq(Lit<'a'>{}, Lit<'b'>{}, Lit<'c'>{});

  static_assert(std::is_same_v<Seq<Lit<'a'>, Lit<'b'>, Lit<'c'>>::AttrType,
                               std::tuple<char, char, char>>,
                "Seq should have AttrType as a tuple of char");

  std::string input = "abc";
  auto b = input.begin();
  auto e = input.end();
  auto r = seq.match(b, e);

  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(std::get<0>(*r), 'a');
  EXPECT_EQ(std::get<1>(*r), 'b');
  EXPECT_EQ(std::get<2>(*r), 'c');
}

TEST(SeqTest, RuleRule) {
  auto lit_a = Lit<'a'>{};
  auto lit_b = Lit<'b'>{};
  auto combined_seq = lit_a >> lit_b;

  std::string input = "ab";
  auto b = input.begin();
  auto e = input.end();
  auto r = combined_seq.match(b, e);

  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(std::get<0>(*r), 'a');
  EXPECT_EQ(std::get<1>(*r), 'b');
}

TEST(SeqTest, SeqRule) {
  auto lit_a = Lit<'a'>{};
  auto lit_b = Lit<'b'>{};
  auto lit_c = Lit<'c'>{};
  auto combined_seq = (lit_a >> lit_b) >> lit_c;

  std::string input = "abc";
  auto b = input.begin();
  auto e = input.end();
  auto r = combined_seq.match(b, e);

  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(std::get<0>(*r), 'a');
  EXPECT_EQ(std::get<1>(*r), 'b');
  EXPECT_EQ(std::get<2>(*r), 'c');
}

TEST(SeqTest, RuleSeq) {
  auto lit_a = Lit<'a'>{};
  auto lit_b = Lit<'b'>{};
  auto lit_c = Lit<'c'>{};
  auto combined_seq = lit_a >> (lit_b >> lit_c);

  std::string input = "abc";
  auto b = input.begin();
  auto e = input.end();
  auto r = combined_seq.match(b, e);

  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(std::get<0>(*r), 'a');
  EXPECT_EQ(std::get<1>(*r), 'b');
  EXPECT_EQ(std::get<2>(*r), 'c');
}

TEST(SeqTest, SeqSeq) {
  auto lit_a = Lit<'a'>{};
  auto lit_b = Lit<'b'>{};
  auto lit_c = Lit<'c'>{};
  auto lit_d = Lit<'d'>{};
  auto lit_ab = lit_a >> lit_b;
  auto lit_cd = lit_c >> lit_d;
  auto combined_seq = lit_ab >> lit_cd;

  std::string input = "abcd";
  auto b = input.begin();
  auto e = input.end();
  auto r = combined_seq.match(b, e);

  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(std::get<0>(*r), 'a');
  EXPECT_EQ(std::get<1>(*r), 'b');
  EXPECT_EQ(std::get<2>(*r), 'c');
  EXPECT_EQ(std::get<3>(*r), 'd');
}