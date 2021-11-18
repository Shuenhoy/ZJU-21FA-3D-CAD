#pragma once

#include <limits>
#include <map>
#ifndef TRIANGULATE_HPP__
#define TRIANGULATE_HPP__

#include <Eigen/Core>
#include <clip2tri/clip2tri.h>
#include <igl/per_face_normals.h>

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
    std::map<Eigen::Vector3d, int> &vertices_map,
    std::vector<Eigen::Vector3d> &vertices, std::vector<Eigen::Vector3i> &faces,
    std::vector<int> &sub_faces) {

    std::vector<std::vector<c2t::Point>> inner;
    std::vector<c2t::Point> outputTriangles; // Every 3 points is a triangle

    std::vector<c2t::Point> outer;

    Eigen::Vector3d origin            = face[0][0];
    Eigen::Matrix<double, 3, 2> basis = find_basis(face[0]);

    std::vector<std::tuple<Eigen::Vector2d, Eigen::Vector3d>> flatten_vertices;

    for (const auto &i : face[0]) {
        Eigen::Vector2d vec = under_basis(i, origin, basis);
        outer.emplace_back(vec(0), vec(1));
        flatten_vertices.emplace_back(vec, i);
    }
    for (int k = 1; k < face.size(); k++) {
        std::vector<c2t::Point> inner_loop;
        for (const auto &i : face[k]) {
            Eigen::Vector2d vec = under_basis(i, origin, basis);
            inner_loop.emplace_back(vec(0), vec(1));
            flatten_vertices.emplace_back(vec, i);
        }
        inner.emplace_back(std::move(inner_loop));
    }

    c2t::clip2tri clip2tri;
    clip2tri.triangulate(inner, outputTriangles, outer);
    std::vector<Eigen::Vector3d> result;
    for (const auto &i : outputTriangles) {
        Eigen::Vector3d ans;
        double dis = std::numeric_limits<double>::infinity();
        Eigen::Vector2d v{i.x, i.y};
        for (auto &&[v2d, v3d] : flatten_vertices) {
            double d = (v2d - v).squaredNorm();
            if (d < dis) {
                dis = d;
                ans = v3d;
            }
        }
        result.emplace_back(ans);
    }

    for (int i = 0; i < result.size(); i += 3) {
        Eigen::Vector3i face;
        for (int j = 0; j < 3; j++) {
            auto it = vertices_map.find(result[i + j]);
            if (it == vertices_map.end()) {
                vertices_map[result[i + j]] = vertices.size();
                vertices.emplace_back(result[i + j]);
                face[j] = vertices.size() - 1;
            } else {
                face[j] = it->second;
            }
        }
        faces.emplace_back(face);
    }
    sub_faces.emplace_back(result.size() / 3);
}

inline void construct_mesh(std::vector<Eigen::Vector3d> &vertices,
                           std::vector<Eigen::Vector3i> &faces,
                           Eigen::MatrixXd &V, Eigen::MatrixXi &F) {
    V.resize(vertices.size(), 3);
    F.resize(faces.size(), 3);
    for (int i = 0; i < vertices.size(); i++) {
        V.row(i) = vertices[i];
    }
    for (int i = 0; i < faces.size(); i++) {
        F.row(i) = faces[i];
    }
}

} // namespace brep_sweep

#endif