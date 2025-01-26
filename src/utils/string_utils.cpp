#include "string_utils.hpp"
#include <algorithm>
#include <locale>

namespace time_kill::utils {
    constexpr bool StringUtils::isNullOrWhitespace(const char* str) {
        return str == nullptr || isNullOrWhitespace(StringView(str));
    }

    constexpr bool StringUtils::isNullOrWhitespace(StringView view ) {
        return view.empty() || std::ranges::all_of(view, [](unsigned char c) {
           return std::isspace(c);
        });
    }

    String StringUtils::toUpperCase(const String& text) {
        std::string result = text;
        std::locale loc("");
        std::ranges::transform(result, result.begin(),[&loc](const unsigned char c) {
            return std::toupper(c, loc);
        });
        return result;
    }

    String StringUtils::padLeft(const String& input, const size_t totalWidth, const char paddingChar) {
        if (input.length() >= totalWidth) {
            return input;
        }
        return std::string(totalWidth - input.length(), paddingChar) + input;
    }

    String StringUtils::padRight(const String& input, const size_t totalWidth, const char paddingChar) {
        if (input.length() >= totalWidth) {
            return input;
        }
        return input + std::string(totalWidth - input.length(), paddingChar);
    }
}
