---
marp: true
theme: gaia
paginate: true
title: Parsing in Modern C++
---

<!-- _class: lead -->
# Parsing in Modern C++

---

# 大綱

- 什麼是 parsing？
- 夢想：最終目標的界面
- 觀察：最單純的 recursive descent parser
- 設計：前往目標的地圖
- 實現：艱難的執行

---

## 什麼是 parsing？

「對文字進行分析，確認其語法結構」

![width:1000px](https://upload.wikimedia.org/wikipedia/commons/d/db/Parsing-example.png)

---

## 什麼是 parsing？

程式語言的通常會設計的很容易 parse，自然語言通常很難 parse

![](https://blog.ml.cmu.edu/wp-content/uploads/2021/07/counterexample_motivation_annotated-970x455.png)

---

## 夢想：最終目標的界面

```cpp
auto two_ints = int_ >> ',' >> int_;
optiona<tuple<int, char, int>> result = parse("12,34", two_ints);
if (result) {
    std::cout << std::get<0>(*result)  // 12
              << std::get<2>(*result); // 23
}
```

```cpp
auto num = double_ | int_;
optional<variant<double, int>> result = parse("12.23", num);
if (result) {
    std::visit([](auto& v) {
        std::cout << v;        // 12.34
    }, *result);
}
```

---

## 夢想：最終目標的界面

```cpp
auto MACQ = str_("MACQ");
auto type3 = MACQ  >>  CM     >> (AM|VR) >> BM
           | MACQ  >> (CM|BM) >> (AM|VR) >> MACC
           | MAC2H >>  CM     >> (AM|VR) >> BM
           | MAC2H >> (CM|BM) >> (AM|VR) >> MACC
           | MAC1H >>  CM     >> (AM|VR) >> BM
           | MAC1H >> (CM|BM) >> (AM|VR) >> MACC
           | MAXPQ >> (CM|BM)            >> MACC
           | MINPQ >> (CM|BM)            >> MACC;
auto inst = type0 | type1 | type2 | ...;
auto vliw = (inst % char_lit(',')) >> char_lit(';');
auto program = vliw % char_lit('\n');
```

---

## 夢想：最終目標的界面

```cpp
optional<Program> result = parse(input_txt, program);
if (result) {
    for (VLIW& vliw: *result) {
        for (Inst& inst: vliw) {
            // ...
        }
    }
}

using Inst = variant<InstType0, InstType1, ...>; // variant 是因為 |
using VLIW = vector<Inst>;     // vector 是因為 %
using Program = vector<VLIW>;  // vector 是因為 %
```

---

## 觀察：最單純的 recursive descent parser

Parser with text input

```cpp
optional<int>    parse_int(string_view input);              // 123
optional<double> parse_double(string_view input);           // 0.
optional<char>   parse_char(string_view input);             // a
optional<none>   parse_char_lit(string_view input, char c); // char c
optional<char>   parse_quoted_char(string_view input);      // 'a'
```

Parser with iter

```cpp
optional<int> parse_int(string_view input);     // 123
optional<int> parse_int(iter& begin, iter end); // 123, begin is ref
```

---

## 觀察：最單純的 recursive descent parser
Recursive descent parser，以 `qouted_char` 為例

```cpp
optional<char> parse_quoted_char(iter& b, iter e) { // 'a'
    auto orig_b = b;
    auto r0 = parse_char_lit(begin, end, '\'');
    if (!r0) { b = orig_b; return nullopt; }

    auto r1 = parse_char(begin, end);
    if (!r1) { b = orig_b; return nullopt; }

    auto r2 = parse_char_lit(begin, end, '\'');
    if (!r2) { b = orig_b; return nullopt; }

    return *r1;
}
```

---

## 觀察：最單純的 recursive descent parser
嘗試組合 sequence of rules

```cpp
using ParserType = function<optional<???>(iter& b, iter e)>>;
using Rules = vector<ParserType>;

optional<???> parse_rule_seq(iter& b, iter e, Rules& rules) {
    auto orig_b = b;
    for (auto& p: rules) {
        auto r = p(b, e);
        if (!r) { b = orig_b; return nullopt; }
    }
    return ???;
}

parse_rule_seq(b, e, {parse_char_lit(???), parse_char(???), parse_char_lit(???)});
```

---

## 觀察：最單純的 recursive descent parser
嘗試組合 alternative rules

```cpp
optional<???> parse_rule_or(iter& b, iter e, Rules& rules) {
    for (auto& p: rules) {
        auto r = p(b, e);
        if (r) { return *r; }
    }
    return nullopt;
}
```

---

## 設計：前往目標的地圖

|  |  |
| --- | --- |
| Rule | 一個 class，用來描述一種可複用的語法結構 |
| Attribute | 一種 rule 所對應的 parse 成功以後得到的值 |
| `>>`, `\|`, `%`, `!`, `*`, `+`, `-`, ... | operator，用來組合 rules（跟 attribute），得到新的 rule |
| `Seq<R1, R2>`, `Or<R1, R2>`, `Repeat<Rule, Sep>`, ... | combined rule class，某一種 parsing 邏輯的實作 |

---

## 設計：前往目標的地圖

```cpp
class Rule {
    using AttrType = ...;
    optional<AttrType> match(iter& b, iter e);
};

template<R1, R2>
class Seq: Rule {
    using AttrType = tuple<R1::AttrType, R2::AttrType>;
    optional<AttrType> match(iter& b, iter e) { ... }
};

template<IsRule R1, IsRule R2>
Seq<R1, R2> operator>>(R1& lhs, R2& rhs) { ... }
```

---

## 設計：前往目標的地圖

Action Items:

- Primitive rules: `int_`, `char_lit`, ...
- Combined rules: `Seq<...>`, `Or<...>`, ...
- Operators: `>>`, `|`, `*`, `+`, `%`, ...

---

## 實現：艱難的執行 `CharLit<C>`, `Int`

```cpp
template <char C>
struct CharLit: Rule {
    using AttrType = char;
    optional <AttrType> matcn(iter& b, iter e) {
        if (b == e) { return nullopt; }
        if (*b != C) { return nullopt; }
        return *b++;
    }
}
```
```cpp
struct Int: Rule {
    using AttrType = int;
    optional <AttrType> matcn(iter& b, iter e) {...}
};
```

---

## 實現：艱難的執行 `Seq<...>`

```cpp
template <typename Head, typename... Tail>
struct Seq: Rule {
    tuple<Head, Tail...>& rules;
    using AttrType = tuple<Head::AttrType, Tail::AttrType...>;
    optional<AttrType> match(iter& b, iter e) {
        auto orig_b = b;
        auto rhead = get<0>(rules).matcn(b, e);
        if (!rhead) { b = orig_b; return nullopt; }
        constexpr if (sizeof...(Tail) == 0) {
            return make_tuple(*rhead);
        } else {
            auto rtail = Seq<Tail...>(tuple_tail(rules)).matcn(b, e);
            if (!rtail) { b = orig_b; return nullopt; }
            auto r = tuple_cat(make_tuple(*rhead), *rtail);
            return r;
        }
    }
}
```

---

## 實現：艱難的執行 `operator>>`

```cpp
template <IsRule L, IsRule R>
auto operator>>(L& lhs, R& rhs) {
    return Seq<L, R>(lhs, rhs);
}
```

---

## 實現：艱難的執行 `operator>>`
Flatten

```cpp
auto a = x >> y >> z; // Seq<Seq<x, y>, z>
//       ^^^^^^              ^^^^^^^^^
//       ^^^^^^^^^^^
```

但我們想要 `Seq<x, y, z>`

```cpp
auto operator>>(Rule, Rule);
auto operator>>(Rule, Seq<>);
auto operator>>(Seq<>, Rule);
auto operator>>(Seq<>, Seq<>);
```

---

## 實現：艱難的執行 `operator>>`

更多複合 rule

| Rule | Attribute |
| --- | --- |
| `sequence` | `std::tuple` |
| `or` | `std::variant` |
| `star` | `std::vector` |
| `plus` | `std::vector` |
| `optional` | `std::optional` |

---


## 實現：艱難的執行 `operator>>`
更多 rules

* `char_lit<'c'>`
* `char_range<'a', 'z'>`
* `int_`
* `double_`
* `quoted_string`

---

## 夢想：最終目標的界面

```cpp
auto type3 = MACQ  >>  CM     >> (AM|VR) >> BM
           | MACQ  >> (CM|BM) >> (AM|VR) >> MACC
           | MAC2H >>  CM     >> (AM|VR) >> BM
           | MAC2H >> (CM|BM) >> (AM|VR) >> MACC
           | MAC1H >>  CM     >> (AM|VR) >> BM
           | MAC1H >> (CM|BM) >> (AM|VR) >> MACC
           | MAXPQ >> (CM|BM)            >> MACC
           | MINPQ >> (CM|BM)            >> MACC;
auto inst = type0 | type1 | type2 | ...;
auto vliw = (inst % char_lit(',')) >> char_lit(';');
auto program = vliw % char_lit('\n');
```
```cpp
auto r = parse(input, program);
```



---

# Generator

Generate 是 parse 的反操作

```cpp
auto type3 = MACQ  >>  CM     >> (AM|VR) >> BM
           | MACQ  >> (CM|BM) >> (AM|VR) >> MACC
           | MAC2H >>  CM     >> (AM|VR) >> BM
           | MAC2H >> (CM|BM) >> (AM|VR) >> MACC
           | MAC1H >>  CM     >> (AM|VR) >> BM
           | MAC1H >> (CM|BM) >> (AM|VR) >> MACC
           | MAXPQ >> (CM|BM)            >> MACC
           | MINPQ >> (CM|BM)            >> MACC;
```

---

```cpp
template <typename Head, template... Tail>
struct Seq: Rule {
    tuple<Head, Tail...>& rules;
    using AttrType = tuple<Head::AttrType, Tail::AttrType...>;
    optional<AttrType> match(iter& b, iter e) { ... }

    // For generation
    void gen(ostream& os) {  // Seq<...> 的 gen()
        std::visit([&](auto&&... r) {
            r.gen(os);       // 是呼叫每一個 sub-rule 的 gen()
        }, rules);
    }
}
```

---

## Known Issues

- Attribute 能是自定義型別嗎？不要 `std::tuple`, `std::variant`
    - `Seq<AttrType, R0, R1, ...>`
    - 用 `std::make_with_tuple<AttrType>(some_tuple)`
- 效能問題，一直不斷重複的建構 `std::tuple`
    - 在 stack 上配置一塊「不初始化」的空間
    - 每次把一個 elem 的位址傳給 sub-parser::match
    - 用 placement `new` 在指定的 address 上面直接建構物件
    
---

## Reference

* [boost spirit](https://www.boost.org/doc/libs/1_88_0/libs/spirit/doc/html/index.html)
* [boost parser](https://www.boost.org/doc/libs/1_88_0/doc/html/parser.html)
* [C++ compile time regex](https://github.com/hanickadot/compile-time-regular-expressions)
* [PoC of this slide](https://github.com/yocox/parser)
* [yrp](https://github.com/yocox/yrp)
