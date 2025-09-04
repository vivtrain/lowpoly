#include <cstdio>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/base.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include "util.hpp"

int main() {
  // Read in image
  std::string imgPath = "../data/me.jpg";
  std::string basename = imgPath.substr(imgPath.find_last_of('/') + 1);
  cv::Mat img = cv::imread(imgPath);
  cv::resize(img, img, cv::Size(img.cols/2, img.rows/2));
  cv::imshow(basename, img);

  // Apply Sobel
  cv::Mat dst;
  util::sobelMagnitude(img, dst);
  cv::imshow(basename + " - Sobel magnitude", dst);
  // Apply non-max suppression
  util::nonMaxSuppress(dst, dst, 11, 0.4);
  // Salt the image with extra vertices at random
  util::salt(dst, 0.001);
  cv::imshow(basename + " - Extracted vertices", dst);

  // Extract vertices from image
  std::vector<cv::Point> vertices {
    // Include corners
    {0, 0},
    {0, dst.rows - 1},
    {dst.cols - 1, 0},
    {dst.cols - 1, dst.rows - 1},
  };
  cv::findNonZero(dst, vertices);
  std::cout << vertices.size() << " vertices" << std::endl;

  while (cv::waitKey(30) != 'q')
    continue;
  cv::destroyAllWindows();
}

