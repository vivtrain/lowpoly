#include "delaunay/quadEdgeRef.hpp"
#include <cassert>
#include <optional>

namespace QE {

  static std::vector<QuadEdgeRef*> allEdges;

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

    allEdges.push_back(self);
    allEdges.push_back(dual);
    allEdges.push_back(converse);
    allEdges.push_back(dualConverse);

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

  void freeAll() {
    for (QuadEdgeRef *e : allEdges)
      delete e;
  }

}

