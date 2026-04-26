#include "unity.h"
#include "colorfix.h"

TEST_CASE("Black returns black", "[colorfix]"){
    uint16_t input = 0x0000; // RGB565 black
    uint16_t expected = 0x0000; // Should still be black after color fix
    uint16_t output = display_colorfix(input);
    TEST_ASSERT_EQUAL_HEX16(expected, output);
}

TEST_CASE("White returns white", "[colorfix]"){
    uint16_t input = 0xFFFF; // RGB565 white
    uint16_t expected = 0xFFDF; // Should still be white after color fix (minus trailing bit in green channel, which will always be zero)
    uint16_t output = display_colorfix(input);
    TEST_ASSERT_EQUAL_HEX16(expected, output);
}

TEST_CASE("Red returns red", "[colorfix]"){
    uint16_t input = 0xF800; // RGB565 red
    uint16_t expected = 0xF800; // Should still be red after color fix
    uint16_t output = display_colorfix(input);
    TEST_ASSERT_EQUAL_HEX16(expected, output);
}

TEST_CASE("Green returns blue", "[colorfix]"){
    uint16_t input = 0x07E0; // RGB565 green
    uint16_t expected = 0x001F; // Should return blue after color fix
    uint16_t output = display_colorfix(input);
    TEST_ASSERT_EQUAL_HEX16(expected, output);
}

TEST_CASE("Blue returns green", "[colorfix]"){
    uint16_t input = 0x001F; // RGB565 blue
    uint16_t expected = 0x07C0; // Should return green after color fix (minus missing trailing bit, which will always be zero)
    uint16_t output = display_colorfix(input);
    TEST_ASSERT_EQUAL_HEX16(expected, output);
}