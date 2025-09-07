#ifndef DELAUNAY_HPP
#define DELAUNAY_HPP

#include <opencv2/core/types.hpp>

class Delaunay {
  public:
    bool inCircle(cv::Point a, cv::Point b, cv::Point c, cv::Point test);
    bool isCCW(cv::Point a, cv::Point b, cv::Point c);
  private:
};

#endif // !DELAUNAY_HPP

