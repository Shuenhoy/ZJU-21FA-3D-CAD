#include <cassert>

#include <halfedge.hpp>
#include <euler_ops.hpp>
#include <sweeping.hpp>
#include <triangulate.hpp>
#include <sweep_expr.hpp>

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>

#include <Eigen/Core>

#include <igl/opengl/glfw/Viewer.h>

using Sweeping = brep_sweep::Sweepping;

auto read_loops(const std::string &filename) {
    std::ifstream fin(filename);
    if (!fin) {
        std::cerr << "Could not open file " << filename << std::endl;
        exit(1);
    }

    int num_loops = 0;
    fin >> num_loops;
    assert(num_loops >= 1);
    std::vector<std::vector<Eigen::Vector2d>> loops;
    for (int i = 0; i < num_loops; i++) {
        int num_vertices = 0;
        fin >> num_vertices;
        assert(num_vertices >= 3);
        std::vector<Eigen::Vector2d> vertices;
        vertices.reserve(num_vertices);

        for (int j = 0; j < num_vertices; j++) {
            double x = 0, y = 0;
            fin >> x >> y;
            vertices.emplace_back(x, y);
        }
        loops.emplace_back(std::move(vertices));
    }
    return loops;
}

auto sweep_faces(auto loops, brep_sweep::Expression &expr, double step_length, size_t num_steps) {
    auto context           = Sweeping::init();
    auto [bot, top, solid] = Sweeping::create_outer_loop(context, loops[0], expr);
    auto outer             = bot->child;

    std::vector<decltype(outer)> hf_loops;
    std::vector<decltype(bot)> inner_faces;
    hf_loops.push_back(outer);

    for (int i = 1; i < loops.size(); i++) {
        // create inner loops
        auto inner = Sweeping::create_inner_loop(context, solid, loops[i], outer, expr);
        hf_loops.push_back(inner->child);
        inner_faces.push_back(inner);
    }
    for (int i = 1; i < loops.size(); i++) {
        context.kfmrh(top, inner_faces[i - 1]);
    }
    for (int i = 1; i <= num_steps; i++) {
        double z  = i * step_length;
        auto loop = bot->child;
        do {
            Sweeping::sweeping(context, solid, loop, z, expr);

            loop = loop->next;
        } while (loop != bot->child);
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
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <face_file> <sweep_file> <step_length> <step>" << std::endl;
        return 1;
    }
    auto loops = read_loops(argv[1]);

    // read all text from argv[2]
    std::string expr;
    {
        std::ifstream fin(argv[2]);
        if (!fin) {
            std::cerr << "Could not open file " << argv[2] << std::endl;
            exit(1);
        }
        std::stringstream buffer;
        buffer << fin.rdbuf();
        expr = buffer.str();
    }

    auto step_length = std::stod(argv[3]);
    auto step        = std::stoi(argv[4]);

    brep_sweep::Expression sweep_brep(expr);

    auto faces = sweep_faces(loops, sweep_brep, step_length, step);

    std::vector<Eigen::Vector3d> vertices;
    std::vector<Eigen::Vector3i> facesf;
    std::vector<int> sub_faces;

    std::map<Eigen::Vector3d, int> vertices_map;

    for (const auto &face : faces) {
        brep_sweep::triangulate(face, vertices_map, vertices, facesf, sub_faces);
    }
    Eigen::MatrixXd V, CN;
    Eigen::MatrixXi F;
    brep_sweep::construct_mesh(vertices, facesf, sub_faces, V, F, CN);

    // Plot the mesh
    igl::opengl::glfw::Viewer viewer;
    viewer.data().set_mesh(V, F);
    viewer.data().set_normals(CN);
    viewer.launch();

    return 0;
}