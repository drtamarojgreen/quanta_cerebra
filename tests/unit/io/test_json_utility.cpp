#include "../test_framework.hpp"
#include "io/json_parser.h"

using cerebra::JsonValue;
using cerebra::JsonParseError;

TEST_CASE("json parses scalars") {
  CHECK(JsonValue::parse("true").as_bool() == true);
  CHECK(JsonValue::parse("false").as_bool() == false);
  CHECK(JsonValue::parse("null").is_null());
  CHECK_NEAR(JsonValue::parse("3.5").as_number(), 3.5, 1e-9);
  CHECK_NEAR(JsonValue::parse("-12").as_number(), -12.0, 1e-9);
  CHECK_NEAR(JsonValue::parse("1e3").as_number(), 1000.0, 1e-9);
  CHECK_EQ(JsonValue::parse("\"hi\\nthere\").as_string(), std::string("hi\nthere"));
}

TEST_CASE("json parses the project's frame format") {
  const char* doc = R"([
    { "brain_activity": [
        { "region": "prefrontal_cortex", "intensity": 0.8 },
        { "region": "amygdala", "intensity": 0.6 }
      ],
      "timestamp_ms": 0 }
  ])";
  JsonValue v = JsonValue::parse(doc);
  CHECK(v.is_array());
  CHECK_EQ(v.as_array().size(), static_cast<std::size_t>(1));
  const JsonValue& frame = v.as_array()[0];
  CHECK(frame.is_object());
  CHECK_EQ(frame["timestamp_ms"].as_int(), static_cast<std::int64_t>(0));
  const JsonValue& act = frame["brain_activity"];
  CHECK(act.is_array());
  CHECK_EQ(act.as_array().size(), static_cast<std::size_t>(2));
  CHECK_EQ(act.as_array()[0]["region"].as_string(), std::string("prefrontal_cortex"));
  CHECK_NEAR(act.as_array()[1]["intensity"].as_number(), 0.6, 1e-9);
}

TEST_CASE("json missing keys return null without throwing") {
  JsonValue v = JsonValue::parse("{\"a\":1}");
  CHECK(v["b"].is_null());
  CHECK(!v.contains("b"));
  CHECK(v.contains("a"));
}

TEST_CASE("json rejects malformed input") {
  CHECK_THROWS(JsonValue::parse("{"));
  CHECK_THROWS(JsonValue::parse("[1,]"));
  CHECK_THROWS(JsonValue::parse("nul"));
  CHECK_THROWS(JsonValue::parse("{\"a\" 1}"));
  CHECK_THROWS(JsonValue::parse("\"unterminated"));
  CHECK_THROWS(JsonValue::parse("1 2"));
}

TEST_CASE("json round-trips through dump") {
  JsonValue v = JsonValue::parse("{\"a\":[1,2,3],\"b\":\"x\",\"c\":true}");
  JsonValue again = JsonValue::parse(v.dump());
  CHECK_EQ(again["a"].as_array().size(), static_cast<std::size_t>(3));
  CHECK_EQ(again["b"].as_string(), std::string("x"));
  CHECK(again["c"].as_bool());
}

TEST_CASE("json decodes every escape sequence") {
  CHECK_EQ(JsonValue::parse("\"\\\"\\\\\\/\\b\\f\\n\\r\\t\").as_string(),
           std::string("\"\\/\b\f\n\r\t"));
  // \uXXXX: ASCII, 2-byte UTF-8, and 3-byte UTF-8 code points.
  CHECK_EQ(JsonValue::parse("\"\\u0041\").as_string(), std::string("A"));
  CHECK_EQ(JsonValue::parse("\"\\u00e9\").as_string(), std::string("\xC3\xA9"));        // 'e' acute
  CHECK_EQ(JsonValue::parse("\"\\u20AC\").as_string(), std::string("\xE2\x82\xAC"));    // euro sign
  CHECK_EQ(JsonValue::parse("\"a\\u0009b\").as_string(), std::string("a\tb"));
  // Bad escapes are rejected.
  CHECK_THROWS(JsonValue::parse("\"\\x41\"));
  CHECK_THROWS(JsonValue::parse("\"\\u12\"));
  CHECK_THROWS(JsonValue::parse("\"\\uZZZZ\"));
  CHECK_THROWS(JsonValue::parse("\"abc\\\"));  // dangling escape -> unterminated string
}

TEST_CASE("json parses number forms and integer accessors") {
  CHECK_NEAR(JsonValue::parse("0").as_number(), 0.0, 1e-12);
  CHECK_NEAR(JsonValue::parse("-0.25").as_number(), -0.25, 1e-12);
  CHECK_NEAR(JsonValue::parse("1e3").as_number(), 1000.0, 1e-9);
  CHECK_NEAR(JsonValue::parse("1E2").as_number(), 100.0, 1e-9);
  CHECK_NEAR(JsonValue::parse("-2.5e-1").as_number(), -0.25, 1e-12);
  CHECK_NEAR(JsonValue::parse("3.0e+2").as_number(), 300.0, 1e-9);
  CHECK_EQ(JsonValue::parse("42").as_int(), static_cast<std::int64_t>(42));
  CHECK_EQ(JsonValue::parse("-7").as_int(), static_cast<std::int64_t>(-7));
  // Type mismatches fall back rather than throw.
  CHECK_EQ(JsonValue::parse("\"hi\").as_int(99), static_cast<std::int64_t>(99));
  CHECK(JsonValue::parse("true").as_string().empty());
  CHECK_THROWS(JsonValue::parse("-"));
  CHECK_THROWS(JsonValue::parse(".5"));
  CHECK_THROWS(JsonValue::parse("+1"));
}

TEST_CASE("json dump escapes control characters and renders whole numbers without a point") {
  CHECK_EQ(JsonValue(3.0).dump(), std::string("3"));
  CHECK_EQ(JsonValue(-12).dump(), std::string("-12"));
  CHECK_EQ(JsonValue(true).dump(), std::string("true"));
  CHECK_EQ(JsonValue(nullptr).dump(), std::string("null"));
  std::string s("x\x01y\tz");
  std::string dumped = JsonValue(s).dump();
  CHECK(dumped.find("\\u0001") != std::string::npos);
  CHECK(dumped.find("\\t") != std::string::npos);
  // ...and it round-trips back to the original bytes.
  CHECK_EQ(JsonValue::parse(dumped).as_string(), s);
}

TEST_CASE("json handles empty containers, deep nesting and stray whitespace") {
  CHECK(JsonValue::parse("  [ ]  ").as_array().empty());
  CHECK(JsonValue::parse("\n{\t}\n").as_object().empty());
  JsonValue deep = JsonValue::parse("[[[[[1]]]]]");
  const JsonValue* p = &deep;
  for (int i = 0; i < 5; ++i) {
    CHECK(p->is_array());
    CHECK_EQ(p->as_array().size(), static_cast<std::size_t>(1));
    p = &p->as_array()[0];
  }
  CHECK_NEAR(p->as_number(), 1.0, 1e-12);
  CHECK_THROWS(JsonValue::parse("   "));     // whitespace only -> end of input
  CHECK_THROWS(JsonValue::parse("));
}
