#pragma once

#ifndef EULER_OPS_HPP_
#define EULER_OPS_HPP_

#include <memory>
#include <unordered_set>

#include "halfedge.hpp"

namespace brep_sweep {

template <typename ...T>
struct EulerContext {
    using HF = HalfEdgeStructure<T...>;
    using Solid = typename HF::Solid;
    using Face = typename HF::Face;
    using Vertex = typename HF::Vertex;
    using HalfEdge = typename HF::HalfEdge;
    using Loop = typename HF::Loop;

    std::unordered_set<Solid> solids;
    std::unordered_set<Face> faces;
    std::unordered_set<Loop> loops;
    std::unordered_set<HalfEdge> halfedges;
    std::unordered_set<Vertex> vertices;
};
}; // namespace brep_sweep
#endif