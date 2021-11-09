#pragma once

#ifndef HALFEDGE_HPP__
#define HALFEDGE_HPP__

#include <memory>
#include <unordered_set>
#include <variant>

namespace brep_sweep {
template <typename Vdata,
          typename Sdata = std::monostate,
          typename Fdata = std::monostate,
          typename Ldata = std::monostate,
          typename Edata = std::monostate>
struct HalfEdgeStructure {
    struct Solid;
    struct Face;
    struct Loop;
    struct HalfEdge;
    struct Vertex;

    struct Solid {
        Face *face;
        Sdata data;
    };
    struct Face {
        Loop *loop;
        Face *prev, *next;
        Fdata data;
    };
    struct Loop {
        HalfEdge *edge;
        Loop *prev, *next;
        Ldata data;
    };
    struct HalfEdge {
        Vertex *vertex;
        Loop *loop;
        Face *face;
        HalfEdge *next;
        HalfEdge *prev;
        HalfEdge *twin;

        Edata data;
    };
    struct Vertex {
        HalfEdge *edge;
        Vdata data;
    };
};
} // namespace brep_sweep

#endif