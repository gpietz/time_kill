#pragma once

#include "prerequisites.hpp"

namespace time_kill::utils {
    class StringUtils {
    public:
        static constexpr bool isNullOrWhitespace(const char* str);
        static constexpr bool isNullOrWhitespace(StringView view );
        static String toUpperCase(const String& text);
        static String padLeft(const String& input, const size_t totalWidth, const char paddingChar);
        static String padRight(const String& input, const size_t totalWidth, const char paddingChar);
    };
}
