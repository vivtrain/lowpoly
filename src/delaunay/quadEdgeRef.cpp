#include "delaunay/quadEdgeRef.hpp"
#include <cassert>
#include <optional>
#include <unordered_set>

namespace QE {

  QuadEdgeRef* &QuadEdgeRef::dual() {
    assert(nextRot != nullptr);
    return nextRot;
  }

  QuadEdgeRef* &QuadEdgeRef::converse() {
    assert(nextRot != nullptr);
    assert(nextRot->nextRot != nullptr);
    return nextRot->nextRot;
  }

  QuadEdgeRef* &QuadEdgeRef::prevCCW() {
    assert(nextRot != nullptr);
    assert(nextRot->nextCCW != nullptr);
    assert(nextRot->nextCCW->nextRot != nullptr);
    return nextRot->nextCCW->nextRot;
  }

  QuadEdgeRef* &QuadEdgeRef::traverseCCW() {
    QuadEdgeRef *dc = dual()->converse();
    assert(dc != nullptr);
    assert(dc->nextCCW != nullptr);
    assert(dc->nextCCW->nextRot != nullptr);
    return dc->nextCCW->nextRot;
  }

  std::optional<cv::Point> QuadEdgeRef::headCoords() {
    return converse()->tailCoords;
  }

  QuadEdgeRef* makeQuadEdge(cv::Point tail, cv::Point head) {
    // Create four refs
    QuadEdgeRef *self = new QuadEdgeRef();
    QuadEdgeRef *dual = new QuadEdgeRef();
    QuadEdgeRef *converse = new QuadEdgeRef();
    QuadEdgeRef *dualConverse = new QuadEdgeRef();

    // Save the payload coordinate data
    self->tailCoords = tail;
    converse->tailCoords = head;

    // Arrange the four edges into a cycle
    self->nextRot = dual;
    dual->nextRot = converse;
    converse->nextRot = dualConverse;
    dualConverse->nextRot = self;

    // Edges between vertices are their own neighbors about the tail
    self->nextCCW = self;
    converse->nextCCW = converse;

    // Dual edges (between faces) form a cycle (only one face exists so far)
    dual->nextCCW = dualConverse;
    dualConverse->nextCCW = dual;

    return self;
  }

  void splice(QuadEdgeRef *a, QuadEdgeRef *b) {
    std::swap(a->nextCCW->nextRot, b->nextCCW->nextRot);
    std::swap(a->nextCCW, b->nextCCW);
  }

  QuadEdgeRef *makeTriangle(cv::Point a, cv::Point b, cv::Point c) {
    QuadEdgeRef *ab = makeQuadEdge(a, b);
    QuadEdgeRef *bc = makeQuadEdge(b, c);
    QuadEdgeRef *ca = makeQuadEdge(c, a);
    splice(ab->converse(), bc);
    splice(bc->converse(), ca);
    splice(ca->converse(), ab);
    return ab;
  }

  QuadEdgeRef *connect(QuadEdgeRef *a, QuadEdgeRef *b) {
    assert(a->tailCoords.has_value());
    assert(b->headCoords().has_value());
    assert(b->tailCoords.has_value());
    assert(b->headCoords().has_value());
    QuadEdgeRef *newEdge
      = makeQuadEdge(a->headCoords().value(), b->tailCoords.value());
    splice(newEdge, a->traverseCCW());
    splice(newEdge->converse(), b);
    return newEdge;
  }

  void sever(QuadEdgeRef *edge) {
    splice(edge, edge->prevCCW());
    splice(edge->converse(), edge->converse()->prevCCW());
  }
  
  QuadEdgeRef *insertPoint(QuadEdgeRef *polygonEdge, cv::Point point) {
    assert(polygonEdge->tailCoords.has_value());
    QuadEdgeRef *firstSpoke
      = makeQuadEdge(polygonEdge->tailCoords.value(), point);
    splice(firstSpoke, polygonEdge);
    QuadEdgeRef *spoke = firstSpoke;
    do {
      spoke = connect(polygonEdge, spoke->converse());
      spoke->dual()->tailCoords.reset();
      spoke->dual()->converse()->tailCoords.reset();
      polygonEdge = spoke->prevCCW();
    } while (polygonEdge->nextCCW != firstSpoke);
    return firstSpoke;
  }

  void flip(QuadEdgeRef *edge) {
    QuadEdgeRef *prevCCW = edge->prevCCW();
    QuadEdgeRef *conversePrevCCW = edge->converse()->prevCCW();
    splice(edge, prevCCW);
    splice(edge->converse(), conversePrevCCW);
    splice(edge, prevCCW->traverseCCW());
    splice(edge->converse(), conversePrevCCW->traverseCCW());
    edge->tailCoords = prevCCW->headCoords();
    edge->headCoords() = conversePrevCCW->headCoords();
  }

  void freeGraph_recurse(QuadEdgeRef *edge,
      std::unordered_set<QuadEdgeRef*> &marked) {
    if (marked.find(edge) != marked.end())
      return;
    marked.insert(edge);
    freeGraph_recurse(edge->nextRot, marked);
    freeGraph_recurse(edge->nextCCW, marked);
    delete edge;
  }

  void freeGraph(QuadEdgeRef *edge) {
    std::unordered_set<QuadEdgeRef*> marked;
    freeGraph_recurse(edge, marked);
  }

}

