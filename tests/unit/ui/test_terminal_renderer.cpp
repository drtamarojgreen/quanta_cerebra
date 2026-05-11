#include "test_harness.hpp"
#include "ui/terminal_renderer.h"
#include "ui/visual_themes.h"

BM_CASE(grayscale_block_endpoints) {
    BM_REQUIRE_EQ(cerebra::grayscale_block(0.0), ' ');
    BM_REQUIRE_EQ(cerebra::grayscale_block(1.0), '@');
}

BM_CASE(utf8_block_endpoints) {
    BM_REQUIRE(std::string(cerebra::utf8_intensity_block(0.0)) == " ");
    BM_REQUIRE(std::string(cerebra::utf8_intensity_block(1.0)) == "\xE2\x96\x88");
}

BM_CASE(themes_have_unique_names) {
    const auto& list = cerebra::available_themes();
    BM_REQUIRE(list.size() >= 3);
    for (std::size_t i = 0; i < list.size(); ++i) {
        for (std::size_t j = i + 1; j < list.size(); ++j) {
            BM_REQUIRE(list[i].name != list[j].name);
        }
    }
}

BM_CASE(terminal_size_minimums) {
    auto s = cerebra::terminal_size();
    BM_REQUIRE(s.cols >= 40);
    BM_REQUIRE(s.rows >= 12);
}
