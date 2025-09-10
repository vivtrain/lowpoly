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
#include "delaunay/delaunay.hpp"
#include "delaunay/quadEdgeRef.hpp"
#include "util.hpp"

int main() {
  // Read in image
  std::string imgPath = "../data/me.jpg";
  std::string basename = imgPath.substr(imgPath.find_last_of('/') + 1);
  cv::Mat img = cv::imread(imgPath);
  cv::resize(img, img, cv::Size(img.cols/2, img.rows/2));
  printf("%s image size: %d x %d\n", basename.c_str(), img.cols, img.rows);
  cv::imshow(basename, img);

  // Apply Sobel
  cv::Mat vertImg;
  util::sobelMagnitude(img, vertImg);
  cv::imshow(basename + " - Sobel magnitude", vertImg);
  // Apply non-max suppression
  util::nonMaxSuppress(vertImg, vertImg, 11, 0.4);
  // Salt the image with extra vertices at random
  util::salt(vertImg, 0.001);
  cv::imshow(basename + " - Extracted vertices", vertImg);

  // Extract vertices from image
  std::vector<cv::Point> vertices;
  cv::findNonZero(vertImg, vertices);
  // Include corners
  vertices.push_back({0, 0});
  vertices.push_back({0, vertImg.rows - 1});
  vertices.push_back({vertImg.cols - 1, 0});
  vertices.push_back({vertImg.cols - 1, vertImg.rows - 1});
  std::cout << vertices.size() << " Vertices" << std::endl;

  QE::QuadEdgeRef *triangulation = Delaunay::triangulate(vertices);
  std::vector<std::vector<cv::Point>> triangles 
    = Delaunay::extractTriangles(triangulation);
  printf("%zu Triangles: \n", triangles.size());
  for (auto &triangle : triangles) {
    for (auto &point : triangle)
      printf("(%d, %d) ", point.x, point.y);
    printf("\n");
  }

  cv::Mat triangulated(vertImg.size(), vertImg.type());
  printf("triangulated image size: %d x %d\n", img.cols, img.rows);
  cv::drawContours(triangulated, triangles, -1, cv::Scalar(1, 1, 1));
  cv::imshow(basename + " - Triangulated", triangulated);

  cv::Mat output(img.size(), CV_8UC3, cv::Scalar(0, 0, 255));
  cv::RNG rng(time(nullptr));
  for (uint i = 0; i < triangles.size(); i++) {
    const auto &triangle = triangles[i];
    cv::Scalar color = util::avgColorInPoly(img, triangle);
    cv::fillConvexPoly(
        output,
        triangle,
        color,
        cv::LINE_AA);
    printf("%d/%zu triangles\n", i, triangles.size());
  }
  cv::imshow(basename + " - Output", output);

  while (cv::waitKey(30) != 'q')
    continue;
  cv::destroyAllWindows();
}

