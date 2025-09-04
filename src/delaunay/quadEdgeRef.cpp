#include "delaunay/quadEdgeRef.hpp"
#include "delaunay/quadEdge.hpp"

QE::QuadEdgeRef* QE::QuadEdgeRef::rotation(int amount) {
  return &parentQE->refs[(index + amount) % Rotations::N_ROTATIONS];
}

QE::QuadEdgeRef* QE::QuadEdgeRef::nextCCW() {
  return &nextQE->refs[nextIndex];
}

