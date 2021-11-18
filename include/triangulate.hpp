#pragma once

#ifndef TRIANGULATE_HPP__
#define TRIANGULATE_HPP__

#include <Eigen/Core>
#include <map>
#include <common.hpp>

namespace brep_sweep {
Eigen::Matrix<double, 3, 2> find_basis(const std::vector<Eigen::Vector3d> &loop);

inline Eigen::Vector2d under_basis(const Eigen::Vector3d &vec, const Eigen::Vector3d &origin, const Eigen::Matrix<double, 3, 2> &basis) {
    return Eigen::Vector2d{(vec - origin).dot(basis.col(0)), (vec - origin).dot(basis.col(1))};
}

inline Eigen::Vector3d from_basis(const Eigen::Vector2d &vec, const Eigen::Vector3d &origin, const Eigen::Matrix<double, 3, 2> &basis) {
    return basis.col(0) * vec(0) + basis.col(1) * vec(1) + origin;
}

void triangulate(
    const std::vector<std::vector<Eigen::Vector3d>> &face,
    std::map<Eigen::Vector3d, int> &vertices_map,
    std::vector<Eigen::Vector3d> &vertices, std::vector<Eigen::Vector3i> &faces,
    std::vector<int> &sub_faces);

void construct_mesh(const std::vector<Eigen::Vector3d> &vertices,
                    const std::vector<Eigen::Vector3i> &faces,
                    const std::vector<int> &sub_faces,
                    Eigen::MatrixXd &V, Eigen::MatrixXi &F, Eigen::MatrixXd &CN);

} // namespace brep_sweep

#endif