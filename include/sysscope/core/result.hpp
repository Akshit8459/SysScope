#pragma once

#include <string>
#include <utility>
#include <stdexcept>

namespace sysscope::core {

template <typename T>
class Result {
public:
    Result(T val) : is_ok_(true), value_(std::move(val)) {}

    static Result<T> error(std::string err_msg) {
        Result<T> res;
        res.is_ok_ = false;
        res.error_msg_ = std::move(err_msg);
        return res;
    }

    [[nodiscard]] bool is_ok() const noexcept { return is_ok_; }
    [[nodiscard]] bool is_err() const noexcept { return !is_ok_; }

    [[nodiscard]] const T& value() const {
        if (!is_ok_) {
            throw std::runtime_error("Attempted to access value of an error Result: " + error_msg_);
        }
        return value_;
    }

    [[nodiscard]] T value_or(T fallback) const {
        return is_ok_ ? value_ : std::move(fallback);
    }

    [[nodiscard]] const std::string& error_message() const noexcept {
        return error_msg_;
    }

private:
    Result() = default;
    bool is_ok_{false};
    T value_{};
    std::string error_msg_{};
};

} // namespace sysscope::core
