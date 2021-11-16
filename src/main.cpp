#include "sweeping.hpp"
#include <cassert>
#include <halfedge.hpp>
#include <euler_ops.hpp>

#include <iostream>
#include <fstream>
#include <vector>

#include <Eigen/Core>

using Sweeping = brep_sweep::Sweepping<Eigen::Vector3d>;

// one arg: filename
int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }
    std::ifstream fin(argv[1]);
    if (!fin) {
        std::cerr << "Could not open file " << argv[1] << std::endl;
        return 1;
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

    auto context           = Sweeping::init();
    auto [bot, top, solid] = Sweeping::create_outer_loop(context, loops[0]);
    auto outer             = bot->child;

    std::vector<decltype(outer)> hf_loops;
    std::vector<decltype(bot)> inner_faces;
    hf_loops.push_back(outer);

    for (int i = 1; i < num_loops; i++) {
        // create inner loops
        auto inner = Sweeping::create_inner_loop(context, solid, loops[i], outer);
        hf_loops.push_back(inner->child);
        inner_faces.push_back(inner);
    }

    for (int i = 0; i < num_loops; i++) {
        Sweeping::sweeping(context, solid, hf_loops[i], Eigen::Vector3d(0.0, 0.0, 1.0));
    }
    for (int i = 1; i < num_loops; i++) {
        context.kfmrh(top, inner_faces[i - 1]);
    }
    auto faces = Sweeping::collect_faces(solid);

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

    return 0;
}