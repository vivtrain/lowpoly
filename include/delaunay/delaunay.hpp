#ifndef DELAUNAY_HPP
#define DELAUNAY_HPP

#include <opencv2/core/types.hpp>

class Delaunay {
  public:
  private:
    bool inCircle(cv::Point a, cv::Point b, cv::Point c, cv::Point test);
};

#endif // !DELAUNAY_HPP

