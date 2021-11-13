#pragma once

#include <cassert>
#include <vector>
#ifndef SWEEPING_HPP__
#define SWEEPING_HPP__

#include "euler_ops.hpp"
#include "halfedge.hpp"

#include <Eigen/Core>

namespace brep_sweep {
template <typename Vec = Eigen::Vector3d>
struct Sweepping {
    using Context  = EulerContext<Vec>;
    using HalfEdge = typename Context::HalfEdge;
    using Loop     = typename Context::Loop;
    using Vertex   = typename Context::Vertex;
    using Solid    = typename Context::Solid;
    using Face     = typename Context::Face;

    static void *sweeping(
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

        for (auto &v : old_vertices) {
            Context::mev(v->data + dir, v, loop);
        }
        for (int i = 0; i < old_vertices.size(); i++) {
            Vertex *v      = old_vertices[i];
            Vertex *v_next = old_vertices[(i + 1) % old_vertices.size()];
            Context::mef(v, v_next, loop);
        }
    }

    static std::tuple<Face *, Face *> create_outer_loop(
        Context &context,
        Solid *solid,
        const std::vector<Vec> &vertices) {
        assert(vertices.size() >= 3);
        auto [_1, first_v, bot] = context.mvfs(vertices[0]);
        Vertex *last_v          = first_v;
        for (int i = 1; i < vertices.size(); i++) {
            auto [v, _] = context.mev(vertices[i], last_v, bot->child);
            last_v      = v;
        }
        auto [top, _3] = context.mef(solid,
                                     last_v, first_v,
                                     bot->child);
        return {bot, top};
    }

    static Loop *create_inner_loop(
        Context &context,
        Solid *solid,
        const std::vector<Vec> &vertices,
        Loop *outer) {
        assert(vertices.size() >= 3);

        auto [he, first_v] = context.mev(vertices[0], outer->child->child, outer);
        Vertex *last_v     = first_v;
        for (int i = 1; i < vertices.size(); i++) {
            auto [nhe, nv] = context.mev(vertices[i - 1], last_v, outer);
            last_v         = nv;
        }
        context.kemr(he, outer);
        context.mef(solid, last_v, first_v);
    }
};

} // namespace brep_sweep

#endif