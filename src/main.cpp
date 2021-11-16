#include "sweeping.hpp"
#include <cassert>
#include <halfedge.hpp>
#include <euler_ops.hpp>

#include <iostream>
#include <fstream>
#include <vector>

#include <Eigen/Core>

#include <igl/opengl/glfw/Viewer.h>

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

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }
    auto loops = read_loops(argv[1]);
    auto faces = sweep_faces(loops);

    const Eigen::MatrixXd V = (Eigen::MatrixXd(8, 3) << 0.0, 0.0, 0.0,
                               0.0, 0.0, 1.0,
                               0.0, 1.0, 0.0,
                               0.0, 1.0, 1.0,
                               1.0, 0.0, 0.0,
                               1.0, 0.0, 1.0,
                               1.0, 1.0, 0.0,
                               1.0, 1.0, 1.0)
                                  .finished();
    const Eigen::MatrixXi F = (Eigen::MatrixXi(12, 3) << 1, 7, 5,
                               1, 3, 7,
                               1, 4, 3,
                               1, 2, 4,
                               3, 8, 7,
                               3, 4, 8,
                               5, 7, 8,
                               5, 8, 6,
                               1, 5, 6,
                               1, 6, 2,
                               2, 6, 8,
                               2, 8, 4)
                                  .finished()
                                  .array() -
                              1;

    // Plot the mesh
    igl::opengl::glfw::Viewer viewer;
    viewer.data().set_mesh(V, F);
    viewer.data().set_face_based(true);
    viewer.launch();

    return 0;
}