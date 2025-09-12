#include "delaunay/quad_edge_ref.h"
#include <cassert>
#include <optional>
#include <stdexcept>
#include <unordered_set>

namespace quadedge {

  void printEndpoints(QuadEdgeRef *edge, const char *label) {
    assert(edge->origCoords.has_value());
    assert(edge->termCoords().has_value());
    printf("%s: tailCoords (%d, %d), headCoords (%d, %d)\n",
        label,
        edge->origCoords->x,
        edge->origCoords->y,
        edge->termCoords()->x,
        edge->termCoords()->y);
  }

  QuadEdgeRef* &QuadEdgeRef::sym() {
    assert(rot != nullptr);
    assert(rot->rot != nullptr);
    return rot->rot;
  }

  QuadEdgeRef* &QuadEdgeRef::oprev() {
    assert(rot != nullptr);
    assert(rot->onext != nullptr);
    assert(rot->onext->rot != nullptr);
    return rot->onext->rot;
  }

  QuadEdgeRef* &QuadEdgeRef::lnext() {
    QuadEdgeRef *dc = rot->sym();
    assert(dc->onext != nullptr);
    assert(dc->onext->rot != nullptr);
    return dc->onext->rot;
  }

  QuadEdgeRef* &QuadEdgeRef::rprev() {
    return sym()->onext;
  }

  std::optional<cv::Point> &QuadEdgeRef::termCoords() {
    return sym()->origCoords;
  }

  std::pair<cv::Point, cv::Point> QuadEdgeRef::points() {
    assert(origCoords.has_value());
    assert(termCoords().has_value());
    return { origCoords.value(), termCoords().value() };
  }

  QuadEdgeRef* makeQuadEdge(cv::Point tail, cv::Point head) {
    // Create four refs
    QuadEdgeRef *self = new QuadEdgeRef();
    QuadEdgeRef *selfRot = new QuadEdgeRef();
    QuadEdgeRef *selfRot2 = new QuadEdgeRef();
    QuadEdgeRef *selfRot3 = new QuadEdgeRef();

    // Save the payload coordinate data
    self->origCoords = tail;
    selfRot2->origCoords = head;

    // Arrange the four edges into a cycle
    self->rot = selfRot;
    selfRot->rot = selfRot2;
    selfRot2->rot = selfRot3;
    selfRot3->rot = self;

    // Edges between vertices are their own neighbors about the tail
    self->onext = self;
    selfRot2->onext = selfRot2;

    // Dual edges (between faces) form a cycle (only one face exists so far)
    selfRot->onext = selfRot3;
    selfRot3->onext = selfRot;

    return self;
  }

  void swapONexts(QuadEdgeRef* &a, QuadEdgeRef* &b) {
    std::swap(a->onext, b->onext);
  }
  void splice(QuadEdgeRef *a, QuadEdgeRef *b) {
    assert(
      (a->origCoords.has_value() && a->termCoords().has_value() &&
       b->origCoords.has_value() && b->termCoords().has_value()) ||
      (!a->origCoords.has_value() && !a->termCoords().has_value() &&
       !b->origCoords.has_value() && !b->termCoords().has_value())
    );
    swapONexts(a->onext->rot, b->onext->rot);
    swapONexts(a, b);
  }

  QuadEdgeRef *makeTriangle(cv::Point a, cv::Point b, cv::Point c) {
    QuadEdgeRef *ab = makeQuadEdge(a, b);
    QuadEdgeRef *bc = makeQuadEdge(b, c);
    QuadEdgeRef *ca = makeQuadEdge(c, a);
    splice(ab->sym(), bc);
    splice(bc->sym(), ca);
    splice(ca->sym(), ab);
    return ab;
  }

  QuadEdgeRef *makePolygon(std::vector<cv::Point> points) {
    if (points.size() < 3)
      throw std::logic_error("Polygons must have at least three vertices.");
    QuadEdgeRef *firstEdge = makeQuadEdge(points[0], points[1]);
    QuadEdgeRef *edge = firstEdge, *lastEdge = nullptr;
    for (uint i = 2; i < points.size(); i++) {
      lastEdge = makeQuadEdge(points[i-1], points[i]);
      splice(edge->sym(), lastEdge);
      edge = lastEdge;
    }
    lastEdge = makeQuadEdge(points.back(), points.front());
    splice(edge->sym(), lastEdge);
    splice(lastEdge->sym(), firstEdge);
    return firstEdge;
  }

  QuadEdgeRef *connect(QuadEdgeRef *a, QuadEdgeRef *b) {
    assert(a->origCoords.has_value());
    assert(b->termCoords().has_value());
    assert(b->origCoords.has_value());
    assert(b->termCoords().has_value());
    QuadEdgeRef *newEdge
      = makeQuadEdge(a->termCoords().value(), b->origCoords.value());
    splice(newEdge, a->lnext());
    splice(newEdge->sym(), b);
    return newEdge;
  }

  void sever(QuadEdgeRef *edge) {
    splice(edge, edge->oprev());
    splice(edge->sym(), edge->sym()->oprev());
    delete edge;
  }

  QuadEdgeRef *insertPoint(QuadEdgeRef *polygonEdge, cv::Point point) {
    assert(polygonEdge->origCoords.has_value());
    QuadEdgeRef *firstSpoke
      = makeQuadEdge(polygonEdge->origCoords.value(), point);
    splice(firstSpoke, polygonEdge);
    QuadEdgeRef *spoke = firstSpoke;
    do {
      spoke = connect(polygonEdge, spoke->sym());
      spoke->rot->origCoords.reset();
      spoke->rot->sym()->origCoords.reset();
      polygonEdge = spoke->oprev();
    } while (polygonEdge->onext != firstSpoke);
    return firstSpoke;
  }

  void flip(QuadEdgeRef *edge) {
    QuadEdgeRef *prev = edge->oprev();
    QuadEdgeRef *symPrev = edge->sym()->oprev();
    splice(edge, prev);
    splice(edge->sym(), symPrev);
    splice(edge, prev->lnext());
    splice(edge->sym(), symPrev->lnext());
    edge->origCoords = prev->termCoords();
    edge->termCoords() = symPrev->termCoords();
  }

  void freeGraph_recurse(QuadEdgeRef *edge,
      std::unordered_set<QuadEdgeRef*> &marked) {
    if (marked.find(edge) != marked.end())
      return;
    marked.insert(edge);
    freeGraph_recurse(edge->rot, marked);
    freeGraph_recurse(edge->onext, marked);
    delete edge;
  }

  void freeGraph(QuadEdgeRef *edge) {
    std::unordered_set<QuadEdgeRef*> marked;
    freeGraph_recurse(edge, marked);
  }

}

