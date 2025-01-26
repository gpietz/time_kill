#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #ifndef WIN32_MEAN_AND_LEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #ifndef NOGDI
        #define NOGDI
    #endif
    #include <windows.h>
#endif

#include <expected>
#include <memory>
#include <optional>
#include <variant>
#include <vector>
#include <string>
#include <stdexcept>
#include <system_error>

namespace time_kill {
    template<typename T>
    using UniquePtr = std::unique_ptr<T>;

    template<typename T, typename ... Args>
    constexpr UniquePtr<T> createUniquePtr(Args&& ... args) {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template<typename T>
    using SharedPtr = std::shared_ptr<T>;

    template<typename T, typename  ... Args>
    constexpr SharedPtr<T> createSharedPtr(Args&& ... args) {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    // Basic Types
    using String = std::string;
    using StringView = std::string_view;

    template<typename T>
    using Vector = std::vector<T>;

    // Error Handling
    template<typename T, typename E>
    using Result = std::expected<T, E>;

    template<typename T, typename E>
    inline Result<T, E> createSuccess(T value) {
        return std::expected<T, E>(std::move(value));
    }

    template<typename T, typename E>
    inline Result<T, E> createFailure(E error) {
        return std::unexpected<E>(std::move(error));
    }

    template<typename T>
    using Optional = std::optional<T>;

    template<typename T>
    Optional<T> createOptional(T value) {
        return std::optional<T>(value);
    }

    // Numeric types
    using uchar_t = unsigned char;
    using u8  = std::uint8_t;
    using u16 = std::uint16_t;
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;
    using i8  = std::int8_t;
    using i16 = std::int16_t;
    using i32 = std::int32_t;
    using i64 = std::int64_t;
    using usize = std::size_t;

    // Debugger Helper
    using Error = std::system_error;
}
