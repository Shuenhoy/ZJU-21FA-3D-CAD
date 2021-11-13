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

    template <typename T>
    using PtrSet = std::unordered_set<std::unique_ptr<T>, IdentityHash, PointerEqual>;

    PtrSet<Solid> solids;
    PtrSet<Face> faces;
    PtrSet<Vertex> vertices;
    PtrSet<HalfEdge> halfedges;
    PtrSet<Loop> loops;

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

    std::tuple<Vertex *, HalfEdge *> mev(
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

        HF::insert_edge_to_loop(loop, he1_ptr, he2_ptr);

        return {v2_ptr, he1_ptr};
    }

    std::tuple<Face *, HalfEdge *> mef(
        Solid *solid,
        Vertex *v1,
        Vertex *v2,
        Loop *loop) {

        auto [he1, he2]   = HalfEdge::create_twins();
        HalfEdge *he1_ptr = halfedges.insert(std::move(he1)).first->get();
        HalfEdge *he2_ptr = halfedges.insert(std::move(he2)).first->get();

        Face *nface = faces.insert(Face::create()).first->get();
        Loop *nloop = loops.insert(Loop::create()).first->get();
        HF::linked_insert(nface, nloop);

        HalfEdge *v1_next = HF::vertex_halfedge_in_loop(v1, loop);
        HalfEdge *v1_prev = v1_next->prev;

        HalfEdge *v2_next = HF::vertex_halfedge_in_loop(v2, loop);
        HalfEdge *v2_prev = v2_next->prev;

        he1_ptr->vertex = v1;
        he2_ptr->vertex = v2;

        HF::connect(v1_prev, he1_ptr);
        HF::connect(he1_ptr, v2_next);
        HF::connect(v2_prev, he2_ptr);
        HF::connect(he2_ptr, v1_next);

        nloop->halfedge = he2_ptr;
        loop->halfedge  = he1_ptr;

        HF::linked_insert(solid, nface);
        return {nface, he1_ptr};
    }

    Loop *kemr(Vertex &v1, Vertex *v2, Loop *loop) {
    }
    Loop *kemr(HalfEdge *he, Loop *loop) { // kill an edge and make a loop
        assert(HF::is_halfedge_in_loop(loop, he));

        HalfEdge *this_prev = he->prev;
        HalfEdge *this_next = he->twin->next;
        HalfEdge *that_next = he->next;
        HalfEdge *that_prev = he->twin->prev;

        HF::connect(this_prev, this_next);
        HF::connect(that_prev, that_next);

        Loop *nloop     = loops.insert(Loop::create()).first->get();
        loop->halfedge  = this_next;
        nloop->halfedge = that_next;
        HF::linked_insert_sibling(loop, nloop);

        return nloop;
    }
    void kfmrh(Face *f1, Face *f2) {
        HF::linked_insert(f1, f2->loop);
        f2->loop->face = f1;
        faces.erase(f2);
    }
};
}; // namespace brep_sweep
#endif