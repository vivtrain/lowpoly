#include "imgutil.hpp"
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <vector>

namespace imgutil {

  std::pair<double, double> getImageRange(int type) {
    switch (CV_MAT_DEPTH(type)) {
      case CV_8U:  return {0.0, 255.0};
      case CV_8S:  return {-128.0, 127.0};      // two's complement range
      case CV_16U: return {0.0, 65535.0};
      case CV_16S: return {-32768.0, 32767.0};
      case CV_32S: return {-2147483648.0, 2147483647.0}; // rarely used as image
      case CV_32F: return {0.0, 1.0};           // convention
      case CV_64F: return {0.0, 1.0};           // convention
      default: CV_Error(cv::Error::StsUnsupportedFormat, "Unsupported type");
    }
  }

  void sobelMagnitude(cv::InputArray src, cv::OutputArray dst) {
    // Run a horizontal and vertical filter
    cv::Mat dstX, dstY;
    // Build the horizontal Sobel kernel
    const uint KERNEL_SIZE = 3;
    float kernelData[KERNEL_SIZE][KERNEL_SIZE] = {
      {-1, 0, 1},
      {-1, 0, 1},
      {-1, 0, 1},
    };
    // Normalize based on image type
    cv::Mat sobX(KERNEL_SIZE, KERNEL_SIZE, CV_32F, kernelData);
    float max = getImageRange(src.type()).second;
    sobX /= max;
    // Generate the vertical Soble kernel by simply transposing
    cv::Mat sobY;
    cv::transpose(sobX, sobY);
    // Convolve the kernels with src
    cv::filter2D(src, dstX, CV_32F, sobX);
    cv::filter2D(src, dstY, CV_32F, sobY);
    // Flatten the channels (weighted average)
    cv::cvtColor(dstX, dstX, cv::COLOR_BGR2GRAY);
    cv::cvtColor(dstY, dstY, cv::COLOR_BGR2GRAY);
    // Calculate Euclidean 2-Norm at each pixel
    cv::magnitude(dstX, dstY, dst);
  }

  inline int linearMap(int toMap, int inMin, int inMax, int outMin, int outMax) {
    if (inMax == inMin) return outMin; // Prevent division by zero
    return outMin + ((toMap - inMin) * (outMax - outMin)) / (inMax - inMin);
  }

  void adaptiveNonMaxSuppress(
      cv::InputArray src,
      cv::OutputArray dst,
      const std::pair<int, int> &kernelRange,
      const int edgeAOERadius,
      const double threshold) {

    cv::Mat srcMat = src.getMat();
    dst.create(src.size(), src.type());
    cv::Mat dstMat = dst.getMatRef();
    // Make a temp output buffer
    cv::Mat output = cv::Mat::zeros(src.size(), src.type());

    cv::Mat blurred = srcMat.clone();
    const int gkSize = edgeAOERadius*2 + 1;
    cv::GaussianBlur(blurred, blurred, cv::Size(gkSize, gkSize), edgeAOERadius);

    const int nRows = src.rows(), nCols = src.cols();
    const auto [min, max] = getImageRange(src.type());

    for (int r = 0; r < nRows; r++) {
      for (int c = 0; c < nCols; c++) {
        int kRadius = linearMap(
            blurred.at<uchar>(r, c),
            0, 255,
            kernelRange.first , kernelRange.second);
        int rMin = std::max(0, r - kRadius);
        int rMax = std::min(nRows - 1, r + kRadius);
        int cMin = std::max(0, c - kRadius);
        int cMax = std::min(nCols - 1, c + kRadius);
        // Look at the submatrix around the current pixel
        cv::Mat view = srcMat(cv::Range(rMin, rMax), cv::Range(cMin, cMax));
        // Find the max and its location
        double maxValue;
        cv::Point maxLoc;
        cv::minMaxLoc(view, nullptr, &maxValue, nullptr, &maxLoc);
        // If the current pixel is the max -> set to 1.0, else -> set to 0.0
        if (maxLoc.x == kRadius && maxLoc.y == kRadius && maxValue > threshold)
          output.at<float>(r, c) = max;
        else
          output.at<float>(r, c) = min;
      }
    }

    // Copy temporary buffer to dst
    output.copyTo(dstMat);
  }

  void nonMaxSuppress(
      cv::InputArray src,
      cv::OutputArray dst,
      const uint kSize,
      const double threshold) {
    // Enforce odd kernel size
    if (kSize % 2 != 1)
      CV_Error(cv::Error::StsBadArg, "kSize: kernel must be odd size");

    cv::Mat srcMat = src.getMat();
    dst.create(src.size(), src.type());
    cv::Mat dstMat = dst.getMatRef();
    // Make a temp output buffer
    cv::Mat output(src.size(), src.type());

    const int nRows = src.rows(), nCols = src.cols();
    const int kRadius = kSize / 2;
    const auto [min, max] = getImageRange(src.type());

    for (int r = 0; r < nRows; r++) {
      for (int c = 0; c < nCols; c++) {
        int rMin = std::max(0, r - kRadius);
        int rMax = std::min(nRows - 1, r + kRadius);
        int cMin = std::max(0, c - kRadius);
        int cMax = std::min(nCols - 1, c + kRadius);
        // Look at the submatrix around the current pixel
        cv::Mat view = srcMat(cv::Range(rMin, rMax), cv::Range(cMin, cMax));
        // Find the max and its location
        double maxValue;
        cv::Point maxLoc;
        cv::minMaxLoc(view, nullptr, &maxValue, nullptr, &maxLoc);
        // If the current pixel is the max -> set to 1.0, else -> set to 0.0
        if (maxLoc.x == kRadius && maxLoc.y == kRadius && maxValue > threshold)
          output.at<float>(r, c) = max;
        else
          output.at<float>(r, c) = min;
      }
    }

    // Copy temporary buffer to dst
    output.copyTo(dstMat);
  }

  void salt(cv::Mat img, const float percent) {
    const int nRows = img.rows, nCols = img.cols;
    const int nGrains = percent * nRows * nCols;
    const int max = getImageRange(img.type()).second;
    cv::RNG rng(time(nullptr));
    for (int i = 0; i < nGrains; i++) {
      int r = rng.uniform(0, nRows);
      int c = rng.uniform(0, nCols);
      img.at<float>(r, c) = max;
    }
  }

  cv::Scalar avgColorInPoly(cv::Mat img, std::vector<cv::Point> polygon) {
    cv::Rect boundingBox = cv::boundingRect(polygon);
    cv::Mat view(img, boundingBox);
    cv::Mat mask = cv::Mat::zeros(boundingBox.size(), CV_8UC1);
    for (auto &point : polygon)
      point -= boundingBox.tl();
    cv::fillConvexPoly(mask, polygon, cv::Scalar(255));
    cv::Scalar avgColor = cv::mean(view, mask);
    return avgColor;
  }

}
