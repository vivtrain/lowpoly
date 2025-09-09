#ifndef DELAUNAY_HPP
#define DELAUNAY_HPP

#include "delaunay/quadEdgeRef.hpp"
#include <opencv2/core/types.hpp>
#include <vector>

namespace Delaunay {
  bool inCircle(cv::Point a, cv::Point b, cv::Point c, cv::Point test);
  bool isCCW(cv::Point a, cv::Point b, cv::Point c);
  bool isLeftOf(cv::Point test, QE::QuadEdgeRef *edge);
  bool isRightOf(cv::Point test, QE::QuadEdgeRef *edge);
  bool isAbove(QE::QuadEdgeRef *test, QE::QuadEdgeRef *baseL);
  QE::QuadEdgeRef* triangulate(std::vector<cv::Point> points);
  std::vector<std::vector<cv::Point>> extractTriangles(QE::QuadEdgeRef *edge);
}

#endif // !DELAUNAY_HPP

