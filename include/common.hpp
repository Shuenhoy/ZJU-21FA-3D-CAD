#pragma once

#ifndef COMMON_HPP_
#define COMMON_HPP_

#include <type_traits>
#include <memory>
#include <Eigen/Core>
#include <algorithm>

namespace std {
template <>
struct less<Eigen::Vector3d> {
    bool operator()(const Eigen::Vector3d &a, const Eigen::Vector3d &b) const {
        assert(a.size() == b.size());
        for (size_t i = 0; i < a.size(); ++i) {
            if (a[i] < b[i])
                return true;
            if (a[i] > b[i])
                return false;
        }
        return false;
    }
};
} // namespace std

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