#ifndef QUAD_EDGE_REF_HPP
#define QUAD_EDGE_REF_HPP

#include <opencv2/core/types.hpp>
#include <optional>
#include <vector>

namespace QuadEdge {

  struct QuadEdgeRef {
    inline QuadEdgeRef(QuadEdgeRef *nextSpoke=nullptr, QuadEdgeRef *rot=nullptr)
      : onext(nextSpoke), rot(rot), origCoords(std::nullopt) {}
    QuadEdgeRef* &sym();
    QuadEdgeRef* &oprev();
    QuadEdgeRef* &lnext();
    QuadEdgeRef* &rprev();
    std::optional<cv::Point> &termCoords();
    std::pair<cv::Point, cv::Point> points();

    QuadEdgeRef *onext;
    QuadEdgeRef *rot;
    std::optional<cv::Point> origCoords;
  };

  void printEndpoints(QuadEdgeRef *edge, const char *label);
  QuadEdgeRef *makeQuadEdge(cv::Point tail, cv::Point head);
  void splice(QuadEdgeRef *a, QuadEdgeRef *b);
  QuadEdgeRef *makeTriangle(cv::Point a, cv::Point b, cv::Point c);
  QuadEdgeRef *makePolygon(std::vector<cv::Point> points);
  QuadEdgeRef *connect(QuadEdgeRef *a, QuadEdgeRef *b);
  void sever(QuadEdgeRef *edge);
  QuadEdgeRef *insertPoint(QuadEdgeRef *polygonEdge, cv::Point point);
  void flip(QuadEdgeRef *edge);
  void freeGraph(QuadEdgeRef *edge);

}

#endif // !QUAD_EDGE_REF_HPP
