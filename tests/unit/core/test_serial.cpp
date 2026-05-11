#include <memory>

#include "../test_framework.hpp"
#include "io/serial_port.hpp"

using namespace cerebra;

TEST_CASE("memory serial port echoes fed bytes and records writes") {
  MemorySerialPort port;
  CHECK(port.open(SerialConfig{}));
  port.feed("hello");
  auto got = port.read(3);
  CHECK_EQ(std::string(got.begin(), got.end()), std::string("hel"));
  got = port.read(100);
  CHECK_EQ(std::string(got.begin(), got.end()), std::string("lo"));
  CHECK(port.read(10).empty());
  port.write_str("ack\n");
  CHECK_EQ(port.sent(), std::string("ack\n"));
}

TEST_CASE("parse_line decodes a single frame and rejects junk") {
  auto ok = SerialActivityStream::parse_line(
      R"({"brain_activity":[{"region":"amygdala","intensity":0.7}],"timestamp_ms":42})");
  CHECK(ok.has_value());
  CHECK_EQ(ok->timestamp_ms, static_cast<std::int64_t>(42));
  CHECK_NEAR(ok->intensity_of("amygdala"), 0.7, 1e-9);

  CHECK(!SerialActivityStream::parse_line("not json").has_value());
  CHECK(!SerialActivityStream::parse_line("   ").has_value());
  CHECK(!SerialActivityStream::parse_line("{ broken").has_value());
  CHECK(!SerialActivityStream::parse_line("[1,2,3]").has_value());  // array, not a frame object
}

TEST_CASE("activity stream reassembles line-delimited frames across reads") {
  auto mem_owner = std::make_unique<MemorySerialPort>();
  MemorySerialPort* mem = mem_owner.get();
  SerialActivityStream stream(std::move(mem_owner));
  CHECK(stream.open(SerialConfig{}));

  // First chunk: one full frame plus the start of a second.
  mem->feed(R"({"brain_activity":[{"region":"thalamus","intensity":0.5}],"timestamp_ms":0})");
  mem->feed("\n{\"brain_activity\":[{\"region\":\"insula\",");
  auto frames = stream.poll();
  CHECK_EQ(frames.size(), static_cast<std::size_t>(1));
  CHECK_NEAR(frames[0].intensity_of("thalamus"), 0.5, 1e-9);

  // Second chunk completes the partial frame and adds a malformed one.
  mem->feed("\"intensity\":0.8}],\"timestamp_ms\":100}\nGARBAGE-{not-json}\n");
  frames = stream.poll();
  CHECK_EQ(frames.size(), static_cast<std::size_t>(1));
  CHECK_NEAR(frames[0].intensity_of("insula"), 0.8, 1e-9);
  CHECK_EQ(stream.frames_decoded(), static_cast<std::size_t>(2));
  CHECK(stream.parse_errors() >= 1);
}

TEST_CASE("activity stream forwards a JSON state request to the device") {
  auto mem_owner = std::make_unique<MemorySerialPort>();
  MemorySerialPort* mem = mem_owner.get();
  SerialActivityStream stream(std::move(mem_owner));
  stream.open(SerialConfig{});
  stream.request_state("rem_sleep");
  // Object keys are emitted in sorted order by JsonValue::dump().
  CHECK_EQ(mem->sent(), std::string("{\"command\":\"set_state\",\"state\":\"rem_sleep\"}\n"));
  // The payload parses back to the expected command.
  auto cmd = cerebra::JsonValue::parse(mem->sent());
  CHECK_EQ(cmd["command"].as_string(), std::string("set_state"));
  CHECK_EQ(cmd["state"].as_string(), std::string("rem_sleep"));

  // take_sent() drains the buffer.
  CHECK(!mem->take_sent().empty());
  CHECK(mem->sent().empty());
  CHECK(mem->take_sent().empty());

  // send_json transmits an arbitrary newline-terminated JSON payload.
  cerebra::JsonValue::Object ping;
  ping.emplace("command", cerebra::JsonValue("ping"));
  stream.send_json(cerebra::JsonValue(std::move(ping)));
  CHECK_EQ(mem->sent(), std::string("{\"command\":\"ping\"}\n"));
}

TEST_CASE("native serial port factory returns an unopened port") {
  auto port = SerialPort::create_native();
  CHECK(port != nullptr);
  CHECK(!port->is_open());
  // Opening a bogus device must fail gracefully (not throw).
  bool opened = false;
  CHECK_NOTHROW(opened = port->open(SerialConfig{"/dev/this-device-does-not-exist", 115200, 10}));
  CHECK(!opened);
}

TEST_CASE("activity stream decodes several frames from a single read") {
  auto mem_owner = std::make_unique<MemorySerialPort>();
  MemorySerialPort* mem = mem_owner.get();
  SerialActivityStream stream(std::move(mem_owner));
  stream.open(SerialConfig{});
  mem->feed(
      "{\"brain_activity\":[{\"region\":\"amygdala\",\"intensity\":0.1}],\"timestamp_ms\":0}\n"
      "{\"brain_activity\":[{\"region\":\"amygdala\",\"intensity\":0.2}],\"timestamp_ms\":1}\n"
      "{\"brain_activity\":[{\"region\":\"amygdala\",\"intensity\":0.3}],\"timestamp_ms\":2}\n");
  // The in-memory port hands out at most 4096 bytes per read; a couple of polls
  // drains everything.
  std::vector<BrainActivitySample> all;
  for (int i = 0; i < 4; ++i) {
    auto f = stream.poll();
    all.insert(all.end(), f.begin(), f.end());
  }
  CHECK_EQ(all.size(), static_cast<std::size_t>(3));
  CHECK_EQ(stream.frames_decoded(), static_cast<std::size_t>(3));
  CHECK_EQ(stream.parse_errors(), static_cast<std::size_t>(0));
  CHECK_NEAR(all[2].intensity_of("amygdala"), 0.3, 1e-9);
}

TEST_CASE("activity stream tolerates CRLF endings, blank lines and whitespace") {
  auto mem_owner = std::make_unique<MemorySerialPort>();
  MemorySerialPort* mem = mem_owner.get();
  SerialActivityStream stream(std::move(mem_owner));
  stream.open(SerialConfig{});
  mem->feed("\n   \n\t\n"
            "{\"brain_activity\":[{\"region\":\"thalamus\",\"intensity\":0.5}],\"timestamp_ms\":0}\r\n"
            "  \r\n");
  auto frames = stream.poll();
  CHECK_EQ(frames.size(), static_cast<std::size_t>(1));
  CHECK_NEAR(frames[0].intensity_of("thalamus"), 0.5, 1e-9);
  // Blank / whitespace-only lines are not counted as parse errors.
  CHECK_EQ(stream.parse_errors(), static_cast<std::size_t>(0));
}

TEST_CASE("activity stream does nothing useful before the port is open") {
  auto mem_owner = std::make_unique<MemorySerialPort>();
  MemorySerialPort* mem = mem_owner.get();
  SerialActivityStream stream(std::move(mem_owner));
  CHECK(!stream.is_open());
  CHECK(stream.poll().empty());           // no port -> no frames
  stream.request_state("focused");        // no port -> nothing written
  CHECK(mem->sent().empty());
}
