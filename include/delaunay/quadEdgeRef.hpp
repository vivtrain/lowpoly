#ifndef QUAD_EDGE_REF_HPP
#define QUAD_EDGE_REF_HPP

#include <opencv2/core/types.hpp>

namespace QE {
  enum Rotations {
    IDENTITY = 0,
    DUAL = 1,
    CONVERSE = 2,
    DUAL_CONVERSE = 3,
    N_ROTATIONS = 4,
  };
  struct QuadEdge; // Forward declare!

  struct QuadEdgeRef {
    QuadEdgeRef(
        QuadEdge *parentQE=nullptr, int index=-1,
        QuadEdge *nextQE=nullptr, int nextIndex=-1) :
      parentQE(parentQE), index(index),
      nextQE(nextQE), nextIndex(nextIndex) {}
    QuadEdgeRef *rotation(int amount);
    QuadEdgeRef *nextCCW();
    
    QuadEdge *parentQE;
    int index;
    QuadEdge *nextQE;
    int nextIndex;
    cv::Point coords;
  };

}

#endif // !QUAD_EDGE_REF_HPP
