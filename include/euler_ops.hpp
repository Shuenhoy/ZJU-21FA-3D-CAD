#pragma once

#ifndef EULER_OPS_HPP_
#define EULER_OPS_HPP_

#include <memory>
#include <unordered_set>
#include <tuple>

#include "halfedge.hpp"
#include "common.hpp"

namespace brep_sweep {

template <typename Vdata>
struct EulerContext {
    using HF       = HalfEdgeStructure<Vdata>;
    using Solid    = typename HF::Solid;
    using Face     = typename HF::Face;
    using Vertex   = typename HF::Vertex;
    using HalfEdge = typename HF::HalfEdge;
    using Loop     = typename HF::Loop;
    using Edge     = typename HF::Edge;

    template <typename T>
    using PtrSet = std::unordered_set<std::unique_ptr<T>, IdentityHash, PointerEqual>;

    PtrSet<Solid> solids;
    PtrSet<Face> faces;
    PtrSet<Vertex> vertices;
    PtrSet<HalfEdge> halfedges;
    PtrSet<Loop> loops;
    PtrSet<Edge> edges;

    std::tuple<Solid *, Vertex *, Face *>
    mvfs(RefTo<Vdata> auto &&vdata) {

        Vertex *v_ptr = vertices.insert(
                                    Vertex::create(std::forward<Vdata>(vdata)))
                            .first->get();
        Face *f_ptr  = faces.insert(Face::create()).first->get();
        Solid *s_ptr = solids.insert(Solid::create()).first->get();
        Loop *l_ptr  = loops.insert(Loop::create()).first->get();

        HF::linked_insert(f_ptr, l_ptr);
        HF::linked_insert(s_ptr, f_ptr);

        return std::make_tuple(s_ptr, v_ptr, f_ptr);
    }

    std::tuple<Vertex *, Edge *> mev(
        RefTo<Vdata> auto &&vdata,
        Vertex *v1_ptr,
        Loop *loop) {

        Vertex *v2_ptr = vertices.insert(
                                     Vertex::create(std::forward<Vdata>(vdata)))
                             .first->get();
        auto [he1, he2]   = HalfEdge::create_twins();
        HalfEdge *he1_ptr = halfedges.insert(std::move(he1)).first->get();
        HalfEdge *he2_ptr = halfedges.insert(std::move(he2)).first->get();

        he1_ptr->vertex = v1_ptr;
        he2_ptr->vertex = v2_ptr;

        Edge *e_ptr     = edges.insert(Edge::create()).first->get();
        e_ptr->halfedge = he1_ptr;
        he1_ptr->edge   = e_ptr;
        he2_ptr->edge   = e_ptr;
        HF::insert_edge_to_loop(loop, e_ptr);
    }

    std::tuple<Edge *, Face *> mef(
        Vertex *v1,
        Vertex *v2,
        Loop *loop) {

        auto [he1, he2]   = HalfEdge::create_twins();
        HalfEdge *he1_ptr = halfedges.insert(std::move(he1)).first->get();
        HalfEdge *he2_ptr = halfedges.insert(std::move(he2)).first->get();

        HalfEdge *v1_prev = v1->halfedge->twins;
        HalfEdge *v1_next = v1_prev->twins;
        HalfEdge *v2_prev = v2->halfedge->twins;
        HalfEdge *v2_next = v2_prev->twins;

        he1_ptr->vertex = v1;
        he2_ptr->vertex = v2;

        Edge *e_ptr     = edges.insert(Edge::create()).first->get();
        e_ptr->halfedge = he1_ptr;
        he1_ptr->edge   = e_ptr;
        he2_ptr->edge   = e_ptr;

        v1_prev->next = he1_ptr;
        he1_ptr->next = v2_next;
        v2_next->prev = he2_ptr;
        he2_ptr->prev = v1_next;
    }

    Loop *kemr(Vertex &v1, Vertex *v2, Loop *loop) {
    }
    Loop *kemr(Edge *e, Loop *loop) { // kill an edge and make a loop
        HalfEdge *he = loop->edge->child;
        do {
            if (he->vertex == e->child->vertex ||
                he->vertex == e->child->twins->vertex) {

                HalfEdge *this_prev = he->prev;
                HalfEdge *this_next = he->twin->next;
                HalfEdge *that_next = he->next;
                HalfEdge *that_prev = he->twin->prev;

                HF::connect(this_prev, this_next);
                HF::connect(that_prev, that_next);

                Loop *l_ptr = loops.insert(Loop::create()).first->get();
                l_ptr->edge = that_next->edge;

                HF::linked_insert_sibling(loop, l_ptr);

                edges.erase(e);

                return;
            }
            he = he->next;
        } while (he != loop->edge->child);
        throw std::runtime_error("kemr: edge not found in loop");
    }
    void kfmrh(Face *f1, Face *f2) {
        HF::linked_insert(f1, f2->loop);
        f2->loop->face = f1;
        faces.erase(f2);
    }
};
}; // namespace brep_sweep
#endif