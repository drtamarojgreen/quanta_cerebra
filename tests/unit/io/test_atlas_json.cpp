#include "test_harness.hpp"
#include "io/atlas_json_parser.h"

#include <sstream>
#include <stdexcept>
#include <string>

BM_CASE(parses_example_array) {
    const char* src = R"([
        {
            "brain_activity": [
                { "region": "prefrontal_cortex", "intensity": 0.8 },
                { "region": "amygdala", "intensity": 0.6 },
                { "region": "occipital_lobe", "intensity": 0.9 }
            ],
            "timestamp_ms": 0
        }
    ])";
    auto frames = cerebra::parse_frames(src);
    BM_REQUIRE_EQ(frames.size(), std::size_t(1));
    BM_REQUIRE_EQ(frames[0].timestamp_ms, std::int64_t(0));
    BM_REQUIRE_EQ(frames[0].regions.size(), std::size_t(3));
    BM_REQUIRE_EQ(frames[0].regions[0].region, std::string("prefrontal_cortex"));
    BM_REQUIRE_NEAR(frames[0].regions[0].intensity, 0.8, 1e-9);
    BM_REQUIRE(!frames[0].regions[0].flows.empty());
}

BM_CASE(parses_single_frame_object) {
    auto f = cerebra::parse_single_frame(
        R"({"brain_activity":[{"region":"amygdala","intensity":0.5}],"timestamp_ms":42})");
    BM_REQUIRE_EQ(f.timestamp_ms, std::int64_t(42));
    BM_REQUIRE_EQ(f.regions.size(), std::size_t(1));
    BM_REQUIRE_NEAR(f.regions[0].intensity, 0.5, 1e-9);
}

BM_CASE(top_level_object_loaded_as_single_frame) {
    auto frames = cerebra::parse_frames(
        R"({"brain_activity":[{"region":"thalamus","intensity":0.4}],"timestamp_ms":7})");
    BM_REQUIRE_EQ(frames.size(), std::size_t(1));
    BM_REQUIRE_EQ(frames[0].timestamp_ms, std::int64_t(7));
}

BM_CASE(rejects_malformed_json) {
    bool threw = false;
    try { cerebra::parse_frames("[{"); } catch (const cerebra::JsonError&) { threw = true; }
    BM_REQUIRE(threw);
}

BM_CASE(rejects_trailing_garbage) {
    bool threw = false;
    try { cerebra::parse_frames("[] junk"); } catch (const cerebra::JsonError&) { threw = true; }
    BM_REQUIRE(threw);
}

BM_CASE(rejects_top_level_number) {
    bool threw = false;
    try { cerebra::parse_frames("42"); } catch (const cerebra::JsonError&) { threw = true; }
    BM_REQUIRE(threw);
}

BM_CASE(rejects_top_level_string) {
    bool threw = false;
    try { cerebra::parse_frames(R"("hello")"); } catch (const cerebra::JsonError&) { threw = true; }
    BM_REQUIRE(threw);
}

BM_CASE(rejects_unterminated_string) {
    bool threw = false;
    try { cerebra::parse_frames(R"([{"brain_activity":[{"region":"oops)"); }
    catch (const cerebra::JsonError&) { threw = true; }
    BM_REQUIRE(threw);
}

BM_CASE(rejects_bad_escape) {
    bool threw = false;
    try { cerebra::parse_frames(R"([{"brain_activity":[{"region":"\q","intensity":0}]}])"); }
    catch (const cerebra::JsonError&) { threw = true; }
    BM_REQUIRE(threw);
}

BM_CASE(rejects_trailing_comma) {
    bool threw = false;
    try {
        cerebra::parse_frames(R"([{"brain_activity":[],"timestamp_ms":0,}])");
    } catch (const cerebra::JsonError&) { threw = true; }
    BM_REQUIRE(threw);
}

BM_CASE(rejects_missing_brain_activity) {
    bool threw = false;
    try {
        cerebra::parse_frames(R"([{"timestamp_ms":0}])");
    } catch (const cerebra::JsonError&) { threw = true; }
    BM_REQUIRE(threw);
}

BM_CASE(rejects_brain_activity_wrong_type) {
    bool threw = false;
    try {
        cerebra::parse_frames(R"([{"brain_activity":null,"timestamp_ms":0}])");
    } catch (const cerebra::JsonError&) { threw = true; }
    BM_REQUIRE(threw);
}

BM_CASE(parse_single_frame_rejects_array) {
    bool threw = false;
    try { cerebra::parse_single_frame("[]"); }
    catch (const cerebra::JsonError&) { threw = true; }
    BM_REQUIRE(threw);
}

BM_CASE(empty_top_level_array_is_zero_frames) {
    auto frames = cerebra::parse_frames("[]");
    BM_REQUIRE_EQ(frames.size(), std::size_t(0));
}

BM_CASE(empty_brain_activity_is_zero_regions) {
    auto frames = cerebra::parse_frames(
        R"([{"brain_activity":[],"timestamp_ms":42}])");
    BM_REQUIRE_EQ(frames.size(), std::size_t(1));
    BM_REQUIRE_EQ(frames[0].timestamp_ms, std::int64_t(42));
    BM_REQUIRE(frames[0].regions.empty());
}

BM_CASE(parses_basic_string_escapes) {
    auto f = cerebra::parse_single_frame(
        R"({"brain_activity":[{"region":"a\nb\tc\\d\/e\"f","intensity":0.5}],"timestamp_ms":0})");
    BM_REQUIRE_EQ(f.regions[0].region, std::string("a\nb\tc\\d/e\"f"));
}

BM_CASE(parses_unicode_escape_ascii_range) {
    auto f = cerebra::parse_single_frame(
        R"({"brain_activity":[{"region":"A","intensity":0.5}],"timestamp_ms":0})");
    BM_REQUIRE_EQ(f.regions[0].region, std::string("A"));
}

BM_CASE(parses_unicode_escape_two_byte_utf8) {
    // é = U+00E9 (é) -> UTF-8 bytes 0xC3 0xA9
    auto f = cerebra::parse_single_frame(
        R"({"brain_activity":[{"region":"é","intensity":0.5}],"timestamp_ms":0})");
    BM_REQUIRE_EQ(f.regions[0].region, std::string("\xC3\xA9"));
}

BM_CASE(parses_unicode_escape_three_byte_utf8) {
    // 中 = U+4E2D (中) -> UTF-8 bytes 0xE4 0xB8 0xAD
    auto f = cerebra::parse_single_frame(
        R"({"brain_activity":[{"region":"中","intensity":0.5}],"timestamp_ms":0})");
    BM_REQUIRE_EQ(f.regions[0].region, std::string("\xE4\xB8\xAD"));
}

BM_CASE(rejects_bad_unicode_escape) {
    bool threw = false;
    try {
        cerebra::parse_frames(R"([{"brain_activity":[{"region":"\uZZZZ","intensity":0}]}])");
    } catch (const cerebra::JsonError&) { threw = true; }
    BM_REQUIRE(threw);
}

BM_CASE(rejects_short_unicode_escape) {
    bool threw = false;
    try {
        cerebra::parse_frames(R"([{"brain_activity":[{"region":"\u00")");
    } catch (const cerebra::JsonError&) { threw = true; }
    BM_REQUIRE(threw);
}

BM_CASE(parses_scientific_notation) {
    auto f = cerebra::parse_single_frame(
        R"({"brain_activity":[{"region":"x","intensity":5e-1}],"timestamp_ms":1e3})");
    BM_REQUIRE_NEAR(f.regions[0].intensity, 0.5, 1e-9);
    BM_REQUIRE_EQ(f.timestamp_ms, std::int64_t(1000));
}

BM_CASE(parses_signed_exponents) {
    auto f = cerebra::parse_single_frame(
        R"({"brain_activity":[{"region":"x","intensity":2.5E+2}],"timestamp_ms":0})");
    // 250 clamped to 1.0
    BM_REQUIRE_NEAR(f.regions[0].intensity, 1.0, 1e-9);
}

BM_CASE(clamps_upper_intensity) {
    auto frames = cerebra::parse_frames(
        R"([{"brain_activity":[{"region":"amygdala","intensity":1.7}],"timestamp_ms":0}])");
    BM_REQUIRE_NEAR(frames[0].regions[0].intensity, 1.0, 1e-9);
}

BM_CASE(clamps_lower_intensity) {
    auto frames = cerebra::parse_frames(
        R"([{"brain_activity":[{"region":"amygdala","intensity":-0.5}],"timestamp_ms":0}])");
    BM_REQUIRE_NEAR(frames[0].regions[0].intensity, 0.0, 1e-9);
}

BM_CASE(skips_entries_with_non_string_region) {
    auto frames = cerebra::parse_frames(R"([{
        "brain_activity":[
            {"region":42,"intensity":0.5},
            {"region":"amygdala","intensity":0.7}
        ],
        "timestamp_ms":0
    }])");
    BM_REQUIRE_EQ(frames[0].regions.size(), std::size_t(1));
    BM_REQUIRE_EQ(frames[0].regions[0].region, std::string("amygdala"));
}

BM_CASE(skips_non_object_brain_activity_entries) {
    auto frames = cerebra::parse_frames(R"([{
        "brain_activity":[
            "not an object",
            42,
            {"region":"amygdala","intensity":0.3}
        ],
        "timestamp_ms":0
    }])");
    BM_REQUIRE_EQ(frames[0].regions.size(), std::size_t(1));
    BM_REQUIRE_EQ(frames[0].regions[0].region, std::string("amygdala"));
}

BM_CASE(null_intensity_defaults_to_zero) {
    auto f = cerebra::parse_single_frame(
        R"({"brain_activity":[{"region":"amygdala","intensity":null}],"timestamp_ms":0})");
    BM_REQUIRE_EQ(f.regions.size(), std::size_t(1));
    BM_REQUIRE_NEAR(f.regions[0].intensity, 0.0, 1e-9);
}

BM_CASE(boolean_intensity_defaults_to_zero) {
    auto f = cerebra::parse_single_frame(
        R"({"brain_activity":[{"region":"amygdala","intensity":true}],"timestamp_ms":0})");
    BM_REQUIRE_NEAR(f.regions[0].intensity, 0.0, 1e-9);
}

BM_CASE(non_numeric_timestamp_defaults_to_zero) {
    auto f = cerebra::parse_single_frame(
        R"({"brain_activity":[{"region":"amygdala","intensity":0.5}],"timestamp_ms":"oops"})");
    BM_REQUIRE_EQ(f.timestamp_ms, std::int64_t(0));
}

BM_CASE(parses_from_stream) {
    std::istringstream iss(R"([{"brain_activity":[],"timestamp_ms":99}])");
    auto frames = cerebra::parse_frames_stream(iss);
    BM_REQUIRE_EQ(frames.size(), std::size_t(1));
    BM_REQUIRE_EQ(frames[0].timestamp_ms, std::int64_t(99));
}

BM_CASE(load_frames_file_missing_throws) {
    bool threw = false;
    try { cerebra::load_frames_file("/no/such/file.json"); }
    catch (const cerebra::JsonError&) { threw = true; }
    BM_REQUIRE(threw);
}

BM_CASE(handles_whitespace_variants) {
    auto frames = cerebra::parse_frames(
        "[\n\t{ \"brain_activity\" : [ ] , \"timestamp_ms\" : 0 }\r\n]");
    BM_REQUIRE_EQ(frames.size(), std::size_t(1));
}

BM_CASE(empty_input_rejected) {
    bool threw = false;
    try { cerebra::parse_frames("); }
    catch (const cerebra::JsonError&) { threw = true; }
    BM_REQUIRE(threw);
}

BM_CASE(empty_object_frame_rejected) {
    bool threw = false;
    try { cerebra::parse_frames("[{}]"); }
    catch (const cerebra::JsonError&) { threw = true; }
    BM_REQUIRE(threw);
}

// ---------- Targeted error-path tests (close gaps in json_parser.cpp) ----------

BM_CASE(rejects_unexpected_character_in_value) {
    // '$' is not a valid JSON value start; parse_value's final fail() fires.
    bool threw = false;
    try { cerebra::parse_frames("[$]"); }
    catch (const cerebra::JsonError&) { threw = true; }
    BM_REQUIRE(threw);
}

BM_CASE(rejects_missing_comma_in_object) {
    bool threw = false;
    try { cerebra::parse_frames(R"([{"a":1 "b":2}])"); }
    catch (const cerebra::JsonError&) { threw = true; }
    BM_REQUIRE(threw);
}

BM_CASE(rejects_missing_comma_in_array) {
    bool threw = false;
    try { cerebra::parse_frames("[1 2]"); }
    catch (const cerebra::JsonError&) { threw = true; }
    BM_REQUIRE(threw);
}

BM_CASE(parses_backspace_escape) {
    auto f = cerebra::parse_single_frame(
        R"({"brain_activity":[{"region":"a\bb","intensity":0.1}],"timestamp_ms":0})");
    BM_REQUIRE_EQ(f.regions[0].region, std::string("a\bb"));
}

BM_CASE(parses_formfeed_escape) {
    auto f = cerebra::parse_single_frame(
        R"({"brain_activity":[{"region":"a\fb","intensity":0.1}],"timestamp_ms":0})");
    BM_REQUIRE_EQ(f.regions[0].region, std::string("a\fb"));
}

BM_CASE(parses_carriage_return_escape) {
    auto f = cerebra::parse_single_frame(
        R"({"brain_activity":[{"region":"a\rb","intensity":0.1}],"timestamp_ms":0})");
    BM_REQUIRE_EQ(f.regions[0].region, std::string("a\rb"));
}

BM_CASE(unicode_escape_one_byte_utf8) {
    // Exercises the code<0x80 ASCII branch of the \u decoder. The C++
    // source uses "\\u0041" so the JSON parser receives the six-character
    // sequence A, which it must decode to 'A'.
    auto f = cerebra::parse_single_frame(
        "{\"brain_activity\":[{\"region\":\"\\u0041\",\"intensity\":0.1}],\"timestamp_ms\":0}");
    BM_REQUIRE_EQ(f.regions[0].region, std::string("A"));
}

BM_CASE(unicode_escape_two_byte_utf8_via_escape) {
    // é -> 'é' in UTF-8 (0xC3 0xA9). Exercises the 0x80..0x7FF branch.
    auto f = cerebra::parse_single_frame(
        "{\"brain_activity\":[{\"region\":\"\\u00e9\",\"intensity\":0.1}],\"timestamp_ms\":0}");
    BM_REQUIRE_EQ(f.regions[0].region, std::string("\xC3\xA9"));
}

BM_CASE(unicode_escape_three_byte_utf8_via_escape) {
    // 中 -> '中' in UTF-8 (0xE4 0xB8 0xAD). Exercises the >=0x800 branch.
    auto f = cerebra::parse_single_frame(
        "{\"brain_activity\":[{\"region\":\"\\u4e2d\",\"intensity\":0.1}],\"timestamp_ms\":0}");
    BM_REQUIRE_EQ(f.regions[0].region, std::string("\xE4\xB8\xAD"));
}

BM_CASE(parses_uppercase_hex_in_unicode_escape) {
    // Uppercase A-F hex branch of the decoder.
    auto f = cerebra::parse_single_frame(
        "{\"brain_activity\":[{\"region\":\"\\u00E9\",\"intensity\":0.1}],\"timestamp_ms\":0}");
    BM_REQUIRE_EQ(f.regions[0].region, std::string("\xC3\xA9"));
}

BM_CASE(parses_false_intensity_as_default) {
    // boolean intensity defaults to zero; also exercises the 'false' branch of
    // parse_bool which the existing 'true' case doesn't.
    auto f = cerebra::parse_single_frame(
        R"({"brain_activity":[{"region":"amygdala","intensity":false}],"timestamp_ms":0})");
    BM_REQUIRE_NEAR(f.regions[0].intensity, 0.0, 1e-9);
}

BM_CASE(rejects_partial_true_literal) {
    bool threw = false;
    try { cerebra::parse_frames("[tru]"); }
    catch (const cerebra::JsonError&) { threw = true; }
    BM_REQUIRE(threw);
}

BM_CASE(rejects_partial_false_literal) {
    bool threw = false;
    try { cerebra::parse_frames("[fals]"); }
    catch (const cerebra::JsonError&) { threw = true; }
    BM_REQUIRE(threw);
}

BM_CASE(rejects_partial_null_literal) {
    bool threw = false;
    try { cerebra::parse_frames("[nul]"); }
    catch (const cerebra::JsonError&) { threw = true; }
    BM_REQUIRE(threw);
}

BM_CASE(rejects_lone_minus_as_number) {
    // '-' alone enters parse_number, exits without digits, and std::stod throws
    // — which exercises the catch block in parse_number.
    bool threw = false;
    try { cerebra::parse_frames("[-]"); }
    catch (const cerebra::JsonError&) { threw = true; }
    BM_REQUIRE(threw);
}
