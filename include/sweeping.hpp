#pragma once

#ifndef SWEEPING_HPP__
#define SWEEPING_HPP__

#include "euler_ops.hpp"
#include "halfedge.hpp"

#include <Eigen/Core>
#include <cassert>
#include <vector>
#include <iostream>
namespace brep_sweep {
template <typename Vec = Eigen::Vector3d>
struct Sweepping {
    using Context  = EulerContext<Vec>;
    using HalfEdge = typename Context::HalfEdge;
    using Loop     = typename Context::Loop;
    using Vertex   = typename Context::Vertex;
    using Solid    = typename Context::Solid;
    using Face     = typename Context::Face;

    static void sweeping(
        Context &context,
        Solid *solid,
        Loop *loop,
        const Vec &dir) {

        HalfEdge *he = loop->child;

        std::vector<Vertex *> old_vertices;

        do {
            HalfEdge *next = he->next;
            old_vertices.push_back(he->vertex);
            he = next;
        } while (he != loop->child);
        assert(old_vertices.size() >= 3);
        std::cout << old_vertices.size() << std::endl;
        std::vector<Vertex *> new_vertices;

        for (auto &v : old_vertices) {
            auto [nv, _] = context.mev(v->data + dir, v, loop);
            new_vertices.push_back(nv);
        }
        for (int i = 0; i < new_vertices.size(); i++) {
            Vertex *v      = new_vertices[i];
            Vertex *v_next = new_vertices[(i + 1) % new_vertices.size()];
            context.mef(solid, v, v_next, loop);
        }
    }

    static Context init() {
        return Context();
    }

    static std::tuple<Face *, Face *, Solid *> create_outer_loop(
        Context &context,
        const std::vector<Vec> &vertices) {
        assert(vertices.size() >= 3);
        auto [solid, first_v, bot] = context.mvfs(vertices[0]);
        Vertex *last_v             = first_v;
        for (int i = 1; i < vertices.size(); i++) {
            auto [v, _] = context.mev(vertices[i], last_v, bot->child);
            last_v      = v;
        }
        auto [top, _3] = context.mef(solid,
                                     last_v, first_v,
                                     bot->child);
        return {bot, top, solid};
    }

    static Face *create_inner_loop(
        Context &context,
        Solid *solid,
        const std::vector<Vec> &vertices,
        Loop *outer) {
        assert(vertices.size() >= 3);

        auto [first_v, he] = context.mev(vertices[0], outer->child->vertex, outer);
        Vertex *last_v     = first_v;
        for (int i = 1; i < vertices.size(); i++) {
            auto [nv, nhe] = context.mev(vertices[i], last_v, outer);
            last_v         = nv;
        }
        Loop *nl    = context.kemr(he, outer);
        auto [f, _] = context.mef(solid, last_v, first_v, nl);
        return f;
    }

    static std::vector<std::vector<std::vector<Vec>>> collect_faces(
        Solid *solid) {
        std::vector<std::vector<std::vector<Vec>>> result;

        // loop over each face
        Face *face = solid->child;
        do {
            std::vector<std::vector<Vec>> face_loops;
            Loop *loop = face->child;
            do {
                HalfEdge *he = loop->child;
                std::vector<Vec> vertices;
                do {
                    vertices.push_back(he->vertex->data);
                    he = he->next;
                } while (he != loop->child);
                face_loops.emplace_back(std::move(vertices));
                loop = loop->next;
            } while (loop != face->child);
            face = face->next;
            result.emplace_back(std::move(face_loops));
        } while (face != solid->child);
        return result;
    }
};

} // namespace brep_sweep

#endif