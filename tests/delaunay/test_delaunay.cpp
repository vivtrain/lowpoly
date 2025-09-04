#include <iostream>
#include "delaunay/quadEdgeRef.hpp"

int main () {
  std::cout << "hello world!" << std::endl;
  QE::QuadEdgeRef qer;
  cv::Point p = qer.coords;
  std::cout << "(" << p.x << ", " << p.y << ")" << std::endl;
}
