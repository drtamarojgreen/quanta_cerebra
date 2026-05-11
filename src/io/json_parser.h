#pragma once

#include "core/state_manager.h"
#include "core/atlas_core.h"
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <string_view>
#include <stdexcept>

namespace cerebra {

/**
 * A general-purpose JSON value container. 
 */
class JsonValue {
public:
    using Array = std::vector<JsonValue>;
    using Object = std::map<std::string, JsonValue>;
    using Data = std::variant<std::monostate, bool, double, std::string, Array, Object>;

    JsonValue() : data_(std::monostate{}) {}
    JsonValue(bool b) : data_(b) {}
    JsonValue(double d) : data_(d) {}
    JsonValue(std::string s) : data_(std::move(s)) {}
    JsonValue(Array a) : data_(std::move(a)) {}
    JsonValue(Object o) : data_(std::move(o)) {}

    bool is_null()   const { return std::holds_alternative<std::monostate>(data_); }
    bool is_bool()   const { return std::holds_alternative<bool>(data_); }
    bool is_number() const { return std::holds_alternative<double>(data_); }
    bool is_string() const { return std::holds_alternative<std::string>(data_); }
    bool is_array()  const { return std::holds_alternative<Array>(data_); }
    bool is_object() const { return std::holds_alternative<Object>(data_); }

    bool as_bool(bool fallback = false) const { return is_bool() ? std::get<bool>(data_) : fallback; }
    double as_number(double fallback = 0.0) const { return is_number() ? std::get<double>(data_) : fallback; }
    const std::string& as_string() const { static const std::string e; return is_string() ? std::get<std::string>(data_) : e; }
    const Array& as_array() const { static const Array e; return is_array() ? std::get<Array>(data_) : e; }
    const Object& as_object() const { static const Object e; return is_object() ? std::get<Object>(data_) : e; }

    const JsonValue& operator[](const std::string& key) const;
    const JsonValue& operator[](std::size_t index) const;
    bool contains(const std::string& key) const;

    static JsonValue parse(std::string_view json);
    std::string dump() const;

private:
    Data data_;
};

class JsonParseError : public std::runtime_error {
public:
    JsonParseError(const std::string& msg, std::size_t pos) 
        : std::runtime_error(msg + " at character " + std::to_string(pos)), pos_(pos) {}
    std::size_t position() const { return pos_; }
private:
    std::size_t pos_;
};

// Unified JSON Mapping Functions
std::vector<BrainFrame> parse_json_frames(std::string_view json);
BrainFrame              parse_single_json_frame(std::string_view json);
RegionAtlas            parse_json_atlas(std::string_view json);
RegionAtlas            load_json_atlas_file(const std::string& path);

} // namespace cerebra
