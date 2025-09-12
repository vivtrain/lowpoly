#ifndef DELAUNAY_HPP
#define DELAUNAY_HPP

#include "delaunay/quad_edge_ref.h"
#include <opencv2/core/types.hpp>
#include <vector>

namespace delaunay {
  bool inCircle(cv::Point a, cv::Point b, cv::Point c, cv::Point test);
  bool isCCW(cv::Point a, cv::Point b, cv::Point c);
  bool isLeftOf(cv::Point test, quadedge::QuadEdgeRef *edge);
  bool isRightOf(cv::Point test, quadedge::QuadEdgeRef *edge);
  bool isAbove(quadedge::QuadEdgeRef *test, quadedge::QuadEdgeRef *baseL);
  quadedge::QuadEdgeRef* triangulate(std::vector<cv::Point> points);
  std::vector<std::vector<cv::Point>>
    extractTriangles(quadedge::QuadEdgeRef *edge);
}

#endif // !DELAUNAY_HPP

