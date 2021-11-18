#pragma once

#ifndef SWEEPING_HPP__
#define SWEEPING_HPP__

#include "euler_ops.hpp"
#include "halfedge.hpp"
#include "sweep_expr.hpp"

#include <Eigen/Core>
#include <cassert>
#include <vector>
#include <iostream>
namespace brep_sweep {

struct Sweepping {
    using Vec      = std::tuple<Eigen::Vector2d, Eigen::Vector3d>;
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
        double z,
        Expression &expr);

    static Context init();

    static std::tuple<Face *, Face *, Solid *> create_outer_loop(
        Context &context,
        const std::vector<Eigen::Vector2d> &vertices,
        Expression &expr);

    static Face *create_inner_loop(
        Context &context,
        Solid *solid,
        const std::vector<Eigen::Vector2d> &vertices,
        Loop *outer,
        Expression &expr);

    static std::vector<std::vector<std::vector<Eigen::Vector3d>>> collect_faces(
        Solid *solid);
};

} // namespace brep_sweep

#endif