#include "delaunay/delaunay.hpp"
#include <opencv2/core.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>

bool Delaunay::inCircle(cv::Point a, cv::Point b, cv::Point c, cv::Point test) {
  int data[4][4] = {
    { a.x,    a.y,    a.dot(a),       1 },
    { b.x,    b.y,    b.dot(b),       1 },
    { c.x,    c.y,    c.dot(c),       1 },
    { test.x, test.y, test.dot(test), 1 },
  };
  cv::Mat inCircleTest(4, 4, CV_32S, data);
  return cv::determinant(inCircleTest) > 0;
}
