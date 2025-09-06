#ifndef QUAD_EDGE_REF_HPP
#define QUAD_EDGE_REF_HPP

#include <opencv2/core/types.hpp>
#include <optional>

namespace QE {
  
  struct QuadEdgeRef {
    inline QuadEdgeRef(QuadEdgeRef *nextCCW=nullptr, QuadEdgeRef *nextRot=nullptr)
      : nextCCW(nextCCW), nextRot(nextRot), tailCoords(std::nullopt) {}
    QuadEdgeRef* &dual();
    QuadEdgeRef* &converse();
    QuadEdgeRef* &prevCCW();
    QuadEdgeRef* &traverseCCW();
    std::optional<cv::Point> headCoords();

    QuadEdgeRef *nextCCW;
    QuadEdgeRef *nextRot;
    std::optional<cv::Point> tailCoords;
  };

  QuadEdgeRef *makeQuadEdge(cv::Point tail, cv::Point head);
  void splice(QuadEdgeRef *a, QuadEdgeRef *b);
  QuadEdgeRef *makeTriangle(cv::Point a, cv::Point b, cv::Point c);
  QuadEdgeRef *connect(QuadEdgeRef *a, QuadEdgeRef *b);
  void sever(QuadEdgeRef *edge);
  QuadEdgeRef *insertPoint(QuadEdgeRef *polygonEdge, cv::Point point);
  void flip(QuadEdgeRef *edge);
  void freeGraph(QuadEdgeRef *edge);

}

#endif // !QUAD_EDGE_REF_HPP
