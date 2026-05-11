#include "io/json_parser.h"
#include <cctype>
#include <sstream>
#include <algorithm>
#include <fstream>

namespace cerebra {

namespace {

class RecursiveParser {
public:
    explicit RecursiveParser(std::string_view text) : text_(text) {}
    JsonValue parse() {
        skip_ws();
        auto val = parse_value();
        skip_ws();
        if (pos_ < text_.size()) throw JsonParseError("Extra data after JSON", pos_);
        return val;
    }
private:
    std::string_view text_;
    std::size_t pos_ = 0;

    void skip_ws() {
        while (pos_ < text_.size() && std::isspace(static_cast<unsigned char>(text_[pos_]))) pos_++;
    }

    char peek() { return pos_ < text_.size() ? text_[pos_] : '\0'; }
    char get() { return pos_ < text_.size() ? text_[pos_++] : '\0'; }

    JsonValue parse_value() {
        skip_ws();
        char c = peek();
        switch (c) {
            case '{': return parse_object();
            case '[': return parse_array();
            case '"': return JsonValue(parse_string());
            case 't': return parse_literal("true", true);
            case 'f': return parse_literal("false", false);
            case 'n': return parse_literal("null", JsonValue());
            default:
                if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) return parse_number();
                throw JsonParseError("Unexpected character", pos_);
        }
    }

    JsonValue parse_object() {
        pos_++; JsonValue::Object obj; skip_ws();
        if (peek() == '}') { pos_++; return JsonValue(std::move(obj)); }
        while (true) {
            skip_ws();
            std::string key = parse_string();
            skip_ws();
            if (get() != ':') throw JsonParseError("Expected ':'", pos_ - 1);
            obj.emplace(std::move(key), parse_value());
            skip_ws();
            char c = get();
            if (c == '}') break;
            if (c != ',') throw JsonParseError("Expected ',' or '}'", pos_ - 1);
        }
        return JsonValue(std::move(obj));
    }

    JsonValue parse_array() {
        pos_++; JsonValue::Array arr; skip_ws();
        if (peek() == ']') { pos_++; return JsonValue(std::move(arr)); }
        while (true) {
            arr.push_back(parse_value());
            skip_ws();
            char c = get();
            if (c == ']') break;
            if (c != ',') throw JsonParseError("Expected ',' or ']'", pos_ - 1);
        }
        return JsonValue(std::move(arr));
    }

    std::string parse_string() {
        if (get() != '"') throw JsonParseError("Expected '\"'", pos_ - 1);
        std::string out;
        while (pos_ < text_.size()) {
            char c = get();
            if (c == '"') return out;
            if (c == '\\') {
                char e = get();
                switch (e) {
                    case '"':  out += '"';  break;
                    case '\\': out += '\\'; break;
                    case 'n':  out += '\n'; break;
                    case 'r':  out += '\r'; break;
                    case 't':  out += '\t'; break;
                    default:   out += e;    break;
                }
            } else out += c;
        }
        throw JsonParseError("Unterminated string", pos_);
    }

    JsonValue parse_number() {
        std::size_t start = pos_;
        while (pos_ < text_.size() && (std::isdigit(static_cast<unsigned char>(text_[pos_])) || 
               text_[pos_] == '.' || text_[pos_] == '-' || text_[pos_] == '+' || 
               text_[pos_] == 'e' || text_[pos_] == 'E')) pos_++;
        return JsonValue(std::stod(std::string(text_.substr(start, pos_ - start))));
    }

    JsonValue parse_literal(std::string_view expected, JsonValue val) {
        if (text_.substr(pos_, expected.size()) == expected) { pos_ += expected.size(); return val; }
        throw JsonParseError("Invalid literal", pos_);
    }
};

cerebra::BrainFrame json_to_frame(const JsonValue& v) {
    cerebra::BrainFrame f;
    f.timestamp_ms = static_cast<std::int64_t>(v["timestamp_ms"].as_number());
    for (const auto& entry : v["brain_activity"].as_array()) {
        cerebra::RegionState rs;
        rs.region = entry["region"].as_string();
        rs.intensity = std::clamp(entry["intensity"].as_number(), 0.0, 1.0);
        for (const auto& [k, val] : entry["metrics"].as_object()) if (val.is_number()) rs.metrics[k] = val.as_number();
        rs.flows = default_flows_for(rs.region, rs.intensity);
        if (!rs.region.empty()) f.regions.push_back(std::move(rs));
    }
    return f;
}

} // namespace

JsonValue JsonValue::parse(std::string_view text) { return RecursiveParser(text).parse(); }

const JsonValue& JsonValue::operator[](const std::string& key) const {
    static const JsonValue kN; if (!is_object()) return kN;
    auto it = as_object().find(key); return it != as_object().end() ? it->second : kN;
}

const JsonValue& JsonValue::operator[](std::size_t index) const {
    static const JsonValue kN; if (!is_array() || index >= as_array().size()) return kN;
    return as_array()[index];
}

bool JsonValue::contains(const std::string& key) const { return is_object() && as_object().count(key); }

// Domain Mappings
std::vector<cerebra::BrainFrame> parse_json_frames(std::string_view json) {
    auto val = JsonValue::parse(json);
    std::vector<cerebra::BrainFrame> out;
    if (val.is_array()) for (const auto& f : val.as_array()) out.push_back(json_to_frame(f));
    else if (val.is_object()) out.push_back(json_to_frame(val));
    return out;
}

cerebra::BrainFrame parse_single_json_frame(std::string_view json) { return json_to_frame(JsonValue::parse(json)); }

RegionAtlas parse_json_atlas(std::string_view json) {
    auto root = JsonValue::parse(json);
    RegionAtlas atlas;
    if (root["extends_builtin"].as_bool()) atlas = RegionAtlas::builtin();
    
    for (const auto& r : root["regions"].as_array()) {
        cerebra::RegionDefinition d;
        d.id = r["id"].as_string();
        d.display_name = r["display_name"].as_string();
        d.slice_row = (int)r["slice"]["row"].as_number();
        d.slice_col = (int)r["slice"]["col"].as_number();
        d.slice_w = (int)r["slice"]["w"].as_number();
        d.slice_h = (int)r["slice"]["h"].as_number();
        d.proj_x = r["projection"]["x"].as_number();
        d.proj_y = r["projection"]["y"].as_number();
        d.proj_z = r["projection"]["z"].as_number();
        d.proj_radius = r["projection"]["radius"].as_number();
        for (const auto& [k, v] : r["flows"].as_object()) d.flows.push_back({k, v.as_number()});
        atlas.add_or_replace(std::move(d));
    }

    for (const auto& entry : root["pathways"].as_array()) {
        PathwayDefinition p;
        p.id = entry["id"].as_string(); p.name = entry["name"].as_string();
        for (const auto& n : entry["nodes"].as_array()) p.nodes.push_back(n.as_string());
        if (p.nodes.empty()) { p.nodes.push_back(entry["from"].as_string()); p.nodes.push_back(entry["to"].as_string()); }
        p.transmitter = entry["transmitter"].as_string();
        p.strength = entry["strength"].as_number();
        p.bidirectional = entry["bidirectional"].as_bool();
        atlas.add_or_replace_pathway(std::move(p));
    }

    for (const auto& entry : root["templates"].as_array()) {
        TemplateDefinition t;
        t.id = entry["id"].as_string();
        t.display_name = entry.contains("display_name") ? entry["display_name"].as_string() : t.id;
        for (const auto& [k, v] : entry["regions"].as_object()) t.intensities[k] = std::clamp(v.as_number(), 0.0, 1.0);
        atlas.add_or_replace_template(std::move(t));
    }
    return atlas;
}

RegionAtlas load_json_atlas_file(const std::string& path) {
    std::ifstream in(path); if (!in) throw std::runtime_error("Cannot open " + path);
    std::stringstream ss; ss << in.rdbuf(); return parse_json_atlas(ss.str());
}

} // namespace cerebra
