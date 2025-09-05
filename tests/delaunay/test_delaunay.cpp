#include <cassert>
#include <iostream>
#include "delaunay/quadEdgeRef.hpp"

int main () {
  std::cout << "Testing QuadEdgeRef..." << std::endl;
  QE::QuadEdgeRef *quadEdge = QE::makeQuadEdge({0,0}, {1,2});

  QE::QuadEdgeRef *self = quadEdge;
  assert(self != nullptr);
  QE::QuadEdgeRef *rot = self->nextRot;
  assert(rot != nullptr);
  QE::QuadEdgeRef *sym = rot->nextRot;
  assert(sym != nullptr);
  QE::QuadEdgeRef *tor = sym->nextRot;
  assert(tor != nullptr);
  assert(tor->nextRot == self);
  std::cout << "Verified circularity of rotations" << std::endl;

  assert(self->nextCCW == self);
  assert(sym->nextCCW == sym);
  std::cout << "Verified circularity of real edges CCW" << std::endl;

  assert(rot->nextCCW == tor);
  assert(tor->nextCCW == rot);
  std::cout << "Verified circularity of dual edges CCW" << std::endl;

  QE::freeGraph(quadEdge);

  std::cout << "ALL TESTS PASSED!" << std::endl;
}
