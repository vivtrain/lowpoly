#ifndef QUAD_EDGE_HPP
#define QUAD_EDGE_HPP

#include "delaunay/quadEdgeRef.hpp"

namespace QE {

  struct QuadEdge {
    QuadEdge(
        QuadEdgeRef self,
        QuadEdgeRef dual,
        QuadEdgeRef converse,
        QuadEdgeRef dualConverse) : refs{self, dual, converse, dualConverse} {} 
    QuadEdgeRef refs[4];
  };

}

#endif // !QUAD_EDGE_HPP
