#pragma once

#ifndef HALFEDGE_HPP__
#define HALFEDGE_HPP__

#include <memory>
#include <unordered_set>
#include <variant>

namespace brep_sweep {

template <typename T, typename U>
concept LinkedListParent = requires(T *t, U *u) {
    { t->child } -> std::same_as<U *>;
};

template <typename U>
concept LinkedListChild = requires(U *u) {
    { u->prev } -> std::same_as<U *>;
    { u->next } -> std::same_as<U *>;
};

template <typename Vdata>
struct HalfEdgeStructure {
    struct Solid;
    struct Face;
    struct Loop;
    struct HalfEdge;
    struct Vertex;
    struct Edge;

    struct Solid {
        Face *child;

        template <typename... T>
        static std::unique_ptr<Solid> create(T &&...ts) {
            return std::make_unique<Solid>(std::forward<T>(ts)...);
        }
    };
    struct Face {
        Loop *child;
        Face *prev, *next;

        template <typename... T>
        static std::unique_ptr<Face> create(T &&...ts) {
            return std::make_unique<Face>(std::forward<T>(ts)...);
        }
    };
    struct Loop {
        Edge *child;
        Loop *prev, *next;

        template <typename... T>
        static std::unique_ptr<Loop> create(T &&...ts) {
            return std::make_unique<Loop>(std::forward<T>(ts)...);
        }
    };

    struct Edge {
        HalfEdge *child;
        Edge *prev, *next;

        template <typename... T>
        static std::unique_ptr<Edge> create(T &&...ts) {
            return std::make_unique<Edge>(std::forward<T>(ts)...);
        }
    };

    struct HalfEdge {
        Vertex *vertex;
        Loop *loop;
        Edge *edge;
        HalfEdge *next;
        HalfEdge *prev;
        HalfEdge *twin;

        template <typename... T>
        static std::unique_ptr<HalfEdge> create(T &&...ts) {
            return std::make_unique<HalfEdge>(std::forward<T>(ts)...);
        }

        static std::tuple<std::unique_ptr<HalfEdge>, std::unique_ptr<HalfEdge>>
        create_twins() {
            auto e1  = create();
            auto e2  = create();
            e1->twin = e2.get();
            e2->twin = e1.get();
            return std::make_tuple(std::move(e1), std::move(e2));
        }
    };
    struct Vertex {
        HalfEdge *edge;
        Vdata data;

        template <typename... T>
        static std::unique_ptr<Vertex> create(T &&...ts) {
            return std::make_unique<Vertex>(std::forward<T>(ts)...);
        }
    };
    template <LinkedListChild T>
    static void connect(T *n1, T *n2) {
        n1->next = n2;
        n2->prev = n1;
    }

    template <LinkedListChild T>
    static void linked_insert_sibling(T *n1, T *n2) {
        T *n1next = n1->next;
        n1->next  = n2;
        n2->prev  = n1;
        n2->next  = n1next;
    }
    static void insert_edge_to_loop(Loop *loop,
                                    Edge *edge) {

        if (loop->child == nullptr) {
            loop->child = edge;
            edge->prev  = edge;
            edge->next  = edge;

            connect(edge->child, edge->child->twin);
            connect(edge->child->twin, edge->child);

        } else {
            Vertex *start = edge->child->vertex;
            HalfEdge *he  = loop->child->child;
            do {
                if (he->vertex == start) {
                    edge->prev           = he->prev->edge;
                    he->prev->edge->next = edge;
                    edge->next           = he->edge;
                    he->edge->prev       = edge;

                    connect_he(he->prev, edge->child);
                    connect_he(edge->child, edge->child->twin);
                    connect_he(edge->child->twin, he);

                    return;
                }
                he = he->next;
            } while (he != loop->child->child);
            throw std::runtime_error("insert_edge_to_loop: vertex not in loop");
        }
    }

    template <LinkedListChild U, LinkedListParent<U> T>
    static void linked_insert(T *parent, U *to_be_insert) {
        if (parent->child == nullptr) {
            parent->child      = to_be_insert;
            to_be_insert->prev = to_be_insert;
            to_be_insert->next = to_be_insert;
        } else {
            to_be_insert->prev        = parent->child->prev;
            to_be_insert->next        = parent->child;
            parent->child->prev->next = to_be_insert;
            parent->child->prev       = to_be_insert;
        }
    }
};

} // namespace brep_sweep

#endif