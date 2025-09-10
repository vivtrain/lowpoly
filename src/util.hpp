#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/opencv.hpp>

namespace util {
  std::pair<double, double> getImageRange(int type);
  void sobelMagnitude(cv::InputArray src, cv::OutputArray dst);
  void nonMaxSuppress(
      cv::InputArray src,
      cv::OutputArray dst,
      const uint kSize,
      const double threshold);
  void adaptiveNonMaxSuppress(
      cv::InputArray src,
      cv::OutputArray dst,
      const int minKernelRadius,
      const int maxKernelRadius,
      const int edgeAOERadius,
      const double threshold);
  void salt(cv::Mat img, const float percent);
  cv::Scalar avgColorInPoly(cv::Mat img, std::vector<cv::Point> polygon);
}

