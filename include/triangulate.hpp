#pragma once

#ifndef TRIANGULATE_HPP__
#define TRIANGULATE_HPP__

#include <Eigen/Core>
#include <clip2tri/clip2tri.h>

namespace brep_sweep {
inline Eigen::Matrix<double, 3, 2> find_basis(const std::vector<Eigen::Vector3d> &loop) {
    Eigen::Vector3d base1 = (loop[1] - loop[0]).normalized();
    for (int i = 1; i < loop.size(); i++) {
        Eigen::Vector3d vec2 = loop[(i + 1) % loop.size()] - loop[i];
        if (base1.cross(vec2).norm() > 1e-6) {
            Eigen::Vector3d base2 = base1.cross(vec2).cross(base1).normalized();
            Eigen::Matrix<double, 3, 2> basis;
            basis << base1, base2;
            return basis;
        }
    }
    assert(false);
}

inline Eigen::Vector2d under_basis(const Eigen::Vector3d &vec, const Eigen::Vector3d &origin, const Eigen::Matrix<double, 3, 2> &basis) {
    return Eigen::Vector2d{(vec - origin).dot(basis.col(0)), (vec - origin).dot(basis.col(1))};
}

inline Eigen::Vector3d from_basis(const Eigen::Vector2d &vec, const Eigen::Vector3d &origin, const Eigen::Matrix<double, 3, 2> &basis) {
    return basis.col(0) * vec(0) + basis.col(1) * vec(1) + origin;
}

inline void triangulate(
    const std::vector<std::vector<Eigen::Vector3d>> &face,
    std::vector<Eigen::Vector3d> &result) {

    std::vector<std::vector<c2t::Point>> inner;
    std::vector<c2t::Point> outputTriangles; // Every 3 points is a triangle

    std::vector<c2t::Point> outer;

    Eigen::Vector3d origin            = face[0][0];
    Eigen::Matrix<double, 3, 2> basis = find_basis(face[0]);

    for (const auto &i : face[0]) {
        Eigen::Vector2d vec = under_basis(i, origin, basis);
        outer.emplace_back(vec(0), vec(1));
    }
    for (int k = 1; k < face.size(); k++) {
        std::vector<c2t::Point> inner_loop;
        for (const auto &i : face[k]) {
            Eigen::Vector2d vec = under_basis(i, origin, basis);
            inner_loop.emplace_back(vec(0), vec(1));
        }
        inner.emplace_back(std::move(inner_loop));
    }

    c2t::clip2tri clip2tri;
    clip2tri.triangulate(inner, outputTriangles, outer);

    for (const auto &i : outputTriangles) {
        result.emplace_back(from_basis(Eigen::Vector2d{i.x, i.y}, origin, basis));
    }
}

inline void construct_mesh(std::vector<Eigen::Vector3d> &vertices, Eigen::MatrixXd &V, Eigen::MatrixXi &F) {
    V.resize(vertices.size(), 3);
    F.resize(vertices.size() / 3, 3);
    for (int i = 0; i < vertices.size(); i++) {
        V.row(i) = vertices[i];
    }
    for (int i = 0; i < vertices.size() / 3; i++) {
        F.row(i) << i * 3, i * 3 + 1, i * 3 + 2;
    }
}

} // namespace brep_sweep

#endif