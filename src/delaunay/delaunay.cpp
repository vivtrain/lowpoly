#include "delaunay/delaunay.hpp"
#include <opencv2/core.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include <stdexcept>


Delaunay::Delaunay(std::vector<cv::Point> sites) : sites(sites) {
  if (sites.size() < 3)
    throw std::logic_error("You must triangulate at least three points");
}

bool Delaunay::isCCW(cv::Point a, cv::Point b, cv::Point c) {
  float xa = a.x, ya = a.y;
  float xb = b.x, yb = b.y;
  float xc = c.x, yc = c.y;
  float data[3][3] = {
    { xa, ya, 1 },
    { xb, yb, 1 },
    { xc, yc, 1 },
  };
  cv::Mat ccwTest(3, 3, CV_32F, data);
  return cv::determinant(ccwTest) > 0;
}

bool Delaunay::isLeftOf(cv::Point test, QE::QuadEdgeRef *edge) {
  if (!edge->origCoords.has_value() || !edge->termCoords().has_value())
    throw std::logic_error("Edge not between vertices.");
  return isCCW(test, edge->origCoords.value(), edge->termCoords().value());
}

bool Delaunay::isRightOf(cv::Point test, QE::QuadEdgeRef *edge) {
  if (!edge->origCoords.has_value() || !edge->termCoords().has_value())
    throw std::logic_error("Edge not between vertices.");
  return isCCW(test, edge->termCoords().value(), edge->origCoords.value());
}

bool Delaunay::isAbove(QE::QuadEdgeRef *test, QE::QuadEdgeRef *baseL) {
  if (!baseL->origCoords.has_value() || !baseL->termCoords().has_value())
    throw std::logic_error("Edge (baseL) not between vertices.");
  if (!test->origCoords.has_value() || !test->termCoords().has_value())
    throw std::logic_error("Edge (test) not between vertices.");
  return isRightOf(test->termCoords().value(), baseL);
}

bool Delaunay::inCircle(cv::Point a, cv::Point b, cv::Point c, cv::Point test) {
  float xa = a.x, ya = a.y, a2 = a.dot(a);
  float xb = b.x, yb = b.y, b2 = b.dot(b);
  float xc = c.x, yc = c.y, c2 = c.dot(c);
  float xt = test.x, yt = test.y, t2 = test.dot(test);
  float data[4][4] = {
    { xa, ya, a2, 1 },
    { xb, yb, b2, 1 },
    { xc, yc, c2, 1 },
    { xt, yt, t2, 1 },
  };
  cv::Mat inCircleMat(4, 4, CV_32F, data);
  double inCircleDet = cv::determinant(inCircleMat);
  return isCCW(a, b, c) ? inCircleDet > 0 : inCircleDet < 0;
}
