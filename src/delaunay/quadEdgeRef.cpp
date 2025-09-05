#include "delaunay/quadEdgeRef.hpp"
#include <cassert>
#include <optional>
#include <unordered_set>

namespace QE {

  QuadEdgeRef* QuadEdgeRef::dual() {
    assert(nextRot != nullptr);
    return nextRot;
  }

  QuadEdgeRef* QuadEdgeRef::converse() {
    assert(nextRot != nullptr);
    assert(nextRot->nextRot != nullptr);
    return nextRot->nextRot;
  }

  QuadEdgeRef* QuadEdgeRef::nextCW() {
    assert(nextRot != nullptr);
    assert(nextRot->nextCCW != nullptr);
    assert(nextRot->nextCCW->nextRot != nullptr);
    return nextRot->nextCCW->nextRot;
  }

  QuadEdgeRef* QuadEdgeRef::traverseCCW() {
    QuadEdgeRef *dc = dual()->converse();
    assert(dc != nullptr);
    assert(dc->nextCCW != nullptr);
    assert(dc->nextCCW->nextRot != nullptr);
    return dc->nextCCW->nextRot;
  }

  std::optional<cv::Point> QuadEdgeRef::headCoords() {
    return converse()->tailCoords;
  }

  QuadEdgeRef *makeQuadEdge(cv::Point tail, cv::Point head) {
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

