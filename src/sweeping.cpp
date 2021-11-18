#include "sweeping.hpp"

namespace brep_sweep {

void Sweepping::sweeping(
    Context &context,
    Solid *solid,
    Loop *loop,
    double z,
    Expression &expr) {

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
        Eigen::Vector2d orv  = std::get<0>(v->data);
        Eigen::Vector3d newv = expr.eval(z, orv.x(), orv.y());
        auto [nv, _]         = context.mev(std::make_tuple(orv, newv), v, loop);
        new_vertices.push_back(nv);
    }
    for (int i = 0; i < new_vertices.size(); i++) {
        Vertex *v      = new_vertices[i];
        Vertex *v_next = new_vertices[(i + 1) % new_vertices.size()];
        context.mef(solid, v, v_next, loop);
    }
}
Sweepping::Context Sweepping::init() {
    return {};
}
std::tuple<Sweepping::Face *, Sweepping::Face *, Sweepping::Solid *> Sweepping::create_outer_loop(
    Context &context,
    const std::vector<Eigen::Vector2d> &vertices,
    Expression &expr) {
    assert(vertices.size() >= 3);
    auto [solid, first_v, bot] = context.mvfs(std::make_tuple(
        vertices[0],
        expr.eval(0, vertices[0].x(), vertices[0].y())));

    Vertex *last_v = first_v;
    for (int i = 1; i < vertices.size(); i++) {
        auto [v, _] = context.mev(std::make_tuple(
                                      vertices[i],
                                      expr.eval(0, vertices[i].x(), vertices[i].y())),
                                  last_v, bot->child);
        last_v      = v;
    }
    auto [top, _3] = context.mef(solid,
                                 last_v, first_v,
                                 bot->child);
    return {bot, top, solid};
}
Sweepping::Face *Sweepping::create_inner_loop(
    Context &context,
    Solid *solid,
    const std::vector<Eigen::Vector2d> &vertices,
    Loop *outer,
    Expression &expr) {
    assert(vertices.size() >= 3);

    auto [first_v, he] = context.mev(std::make_tuple(
                                         vertices[0],
                                         expr.eval(0, vertices[0].x(), vertices[0].y())),
                                     outer->child->vertex, outer);
    Vertex *last_v     = first_v;
    for (int i = 1; i < vertices.size(); i++) {
        auto [nv, nhe] = context.mev(std::make_tuple(
                                         vertices[i],
                                         expr.eval(0, vertices[i].x(), vertices[i].y())),
                                     last_v, outer);
        last_v         = nv;
    }
    Loop *nl    = context.kemr(he, outer);
    auto [f, _] = context.mef(solid, last_v, first_v, nl);
    return f;
}
std::vector<std::vector<std::vector<Eigen::Vector3d>>> Sweepping::collect_faces(
    Solid *solid) {
    std::vector<std::vector<std::vector<Eigen::Vector3d>>> result;

    // loop over each face
    Face *face = solid->child;
    do {
        std::vector<std::vector<Eigen::Vector3d>> face_loops;
        Loop *loop = face->child;
        do {
            HalfEdge *he = loop->child;
            std::vector<Eigen::Vector3d> vertices;
            do {
                vertices.push_back(std::get<1>(he->vertex->data));
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
} // namespace brep_sweep