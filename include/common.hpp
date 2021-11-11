#pragma once

#ifndef COMMON_HPP_
#define COMMON_HPP_

#include <type_traits>
#include <memory>

namespace brep_sweep {

template <typename TR, typename T>
concept RefTo = std::is_convertible_v<std::remove_cvref_t<TR>, T>;

struct IdentityHash {
    template <class U>
    size_t operator()(const U &ptr) const {
        return std::hash<U>{}(ptr);
    }
};
struct PointerEqual {
    bool operator()(const auto &lhs, const auto &rhs) const {
        return std::to_address(lhs) == std::to_address(rhs);
    }
};

} // namespace brep_sweep

#endif