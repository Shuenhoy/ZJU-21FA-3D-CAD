#include "sweeping.hpp"
#include <cassert>
#include <halfedge.hpp>
#include <euler_ops.hpp>

#include <iostream>
#include <fstream>
#include <vector>

#include <Eigen/Core>

#include <igl/opengl/glfw/Viewer.h>

#include "clip2tri/clip2tri.h"

using Sweeping = brep_sweep::Sweepping<Eigen::Vector3d>;

auto read_loops(const std::string &filename) {
    std::ifstream fin(filename);
    if (!fin) {
        std::cerr << "Could not open file " << filename << std::endl;
        exit(1);
    }

    int num_loops = 0;
    fin >> num_loops;
    assert(num_loops >= 1);
    std::vector<std::vector<Eigen::Vector3d>> loops;
    for (int i = 0; i < num_loops; i++) {
        int num_vertices = 0;
        fin >> num_vertices;
        assert(num_vertices >= 3);
        std::vector<Eigen::Vector3d> vertices;
        vertices.reserve(num_vertices);

        for (int j = 0; j < num_vertices; j++) {
            double x = 0, y = 0;
            fin >> x >> y;
            vertices.emplace_back(x, y, 0.0);
        }
        loops.emplace_back(std::move(vertices));
    }
    return loops;
}

auto sweep_faces(auto loops) {
    auto context           = Sweeping::init();
    auto [bot, top, solid] = Sweeping::create_outer_loop(context, loops[0]);
    auto outer             = bot->child;

    std::vector<decltype(outer)> hf_loops;
    std::vector<decltype(bot)> inner_faces;
    hf_loops.push_back(outer);

    for (int i = 1; i < loops.size(); i++) {
        // create inner loops
        auto inner = Sweeping::create_inner_loop(context, solid, loops[i], outer);
        hf_loops.push_back(inner->child);
        inner_faces.push_back(inner);
    }

    for (int i = 0; i < loops.size(); i++) {
        Sweeping::sweeping(context, solid, hf_loops[i], Eigen::Vector3d(0.0, 0.0, 1.0));
    }
    for (int i = 1; i < loops.size(); i++) {
        context.kfmrh(top, inner_faces[i - 1]);
    }
    auto faces = Sweeping::collect_faces(solid);
    std::cout << "solid with " << faces.size() << " faces" << std::endl;
    for (auto &&face : faces) {
        // print loop nums
        std::cout << "face with " << face.size() << " loops" << std::endl;
        for (auto &&loop : face) {
            std::cout << "loop with " << loop.size() << " vertices" << std::endl;
            for (auto &&vertex : loop) {
                std::cout << vertex.x() << " " << vertex.y() << " " << vertex.z() << std::endl;
            }
            std::cout << "-------------" << std::endl;
        }
    }

    return faces;
}

Eigen::Matrix<double, 3, 2> find_basis(const std::vector<Eigen::Vector3d> &loop) {
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

Eigen::Vector2d under_basis(const Eigen::Vector3d &vec, const Eigen::Vector3d &origin, const Eigen::Matrix<double, 3, 2> &basis) {
    return Eigen::Vector2d{(vec - origin).dot(basis.col(0)), (vec - origin).dot(basis.col(1))};
}

Eigen::Vector3d from_basis(const Eigen::Vector3d &vec, const Eigen::Vector3d &origin, const Eigen::Matrix<double, 3, 2> &basis) {
    return basis.col(0) * vec(0) + basis.col(1) * vec(1) + origin;
}

void triangulate(
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
        result.emplace_back(from_basis(Eigen::Vector3d(i.x, i.y, 0.0), origin, basis));
    }
}

void construct_mesh(std::vector<Eigen::Vector3d> &vertices, Eigen::MatrixXd &V, Eigen::MatrixXi &F) {
    V.resize(vertices.size(), 3);
    F.resize(vertices.size() / 3, 3);
    for (int i = 0; i < vertices.size(); i++) {
        V.row(i) = vertices[i];
    }
    for (int i = 0; i < vertices.size() / 3; i++) {
        F.row(i) << i * 3, i * 3 + 1, i * 3 + 2;
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }
    auto loops = read_loops(argv[1]);
    auto faces = sweep_faces(loops);
    std::vector<Eigen::Vector3d> vertices;
    for (const auto &face : faces) {
        triangulate(face, vertices);
    }
    Eigen::MatrixXd V;
    Eigen::MatrixXi F;
    construct_mesh(vertices, V, F);

    // Plot the mesh
    igl::opengl::glfw::Viewer viewer;
    viewer.data().set_mesh(V, F);
    viewer.data().set_face_based(true);
    viewer.launch();

    return 0;
}