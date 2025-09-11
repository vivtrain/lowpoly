#include <cstdio>
#include <cstdlib>
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

using namespace std;
using namespace QuadEdge;

int main() {
  bool again = true;
  int upscale = 1, downscale = 1;
  // Read in an image from the specified path
  string imgPath = "../data/headshot.jpg";
  string basename = imgPath.substr(imgPath.find_last_of('/') + 1);
  cv::Mat img = cv::imread(imgPath);
  if (img.empty())
    CV_Error(cv::Error::StsObjectNotFound,
        "An image was not found at " + imgPath);
  cv::Size origSize(img.size());
  while (again) {
    printf("\n△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽ lowpoly generator △▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽\n");
    printf(
        "Image: %s\n"
        "Original size (w x h): (%d, %d)\n"
        "Downscale factor (preprocess): %d -> (%d, %d)\n"
        "Upscale factor (postprocess): %d -> (%d, %d)\n",
        imgPath.c_str(),
        origSize.width, origSize.height,
        downscale,
          origSize.width/downscale, origSize.height/downscale,
        upscale,
          origSize.width/downscale*upscale, origSize.height/downscale*upscale
    );
    cv::Mat downscaled;
    cv::resize(img, downscaled, cv::Size(img.cols/downscale, img.rows/downscale));
    cv::imshow(basename, downscaled);

    // Apply Sobel edge detector
    cv::Mat sobelImg;
    util::sobelMagnitude(downscaled, sobelImg);
    cv::imshow(basename + " - Sobel magnitude", sobelImg);

    // Apply non-max suppression
    cv::Mat vertexImg;
    util::adaptiveNonMaxSuppress(sobelImg, vertexImg, 2, 7, 5, 0.4);
    // Salt the image with extra vertices at random
    util::salt(vertexImg, 0.001);
    // Include the corners
    float maxValue = util::getImageRange(vertexImg.type()).second;
    vertexImg.at<float>({0, 0}) = 
    vertexImg.at<float>({0, vertexImg.rows - 1}) = 
    vertexImg.at<float>({vertexImg.cols - 1, 0}) = 
    vertexImg.at<float>({vertexImg.cols - 1, vertexImg.rows - 1}) = maxValue;
    cv::imshow(basename + " - Extracted vertices", vertexImg);

    // Extract vertices from the vertex image
    vector<cv::Point> vertices;
    cv::findNonZero(vertexImg, vertices);
    printf("• %zu Vertices\n", vertices.size());
    // Construct the Delaunay triangulation of the vertex set
    QuadEdgeRef *triangulation = Delaunay::triangulate(vertices);
    vector<vector<cv::Point>> ogTriangles
      = Delaunay::extractTriangles(triangulation), upscaledTris(ogTriangles);
    printf("△ %zu Triangles\n", ogTriangles.size());

    // Scale the output back up
    for (auto &triangle : upscaledTris)
      for (auto &point : triangle)
        point *= upscale;
    cv::Size outputSize(downscaled.cols * upscale, downscaled.rows * upscale);

    cv::Mat triangulated = cv::Mat::zeros(outputSize, CV_8UC3);
    cv::drawContours(triangulated, upscaledTris, -1, cv::Scalar(200, 100, 100), 1, cv::LINE_AA);
    for (const auto &triangle : upscaledTris) {
      for (auto &point : triangle)
        cv::circle(
            triangulated, point,
            2, cv::Scalar(255, 0, 255), cv::FILLED, cv::LINE_AA);
    }
    cv::imshow(basename + " - Triangulated", triangulated);

    // Mark any areas not triangulated bright red (known bug)
    cv::Mat output(outputSize, CV_8UC3, cv::Scalar(0, 0, 255));
    cv::RNG rng(time(nullptr));
    for (uint i = 0; i < upscaledTris.size(); i++) {
      const auto &ogTri = ogTriangles[i];
      const auto &upTri = upscaledTris[i];
      cv::Scalar color = util::avgColorInPoly(downscaled, ogTri);
      cv::fillConvexPoly(output, upTri, color, cv::LINE_AA);
    }
    cv::imshow(basename + " - Output", output);

    // Interactive state machine to adjust output
    printf("\nIn any preview window:\n"
        "▶ q: quit\n"
        "▶ r: re-triangulate\n"
        "▶ w: write output\n"
        "▶ u: decrease input downscale (min of 1)\n"
        "▶ d: increase input downscale\n"
        "▶ U: increase output upscale\n"
        "▶ D: decrease output upscale (min of 1)\n");
    char key = '_';
    while (true) {
      key = cv::waitKey(50);
      bool doBreak = false;
      switch(key) {
        case 'q':
          cv::destroyAllWindows();
          doBreak = true;
          again = false;
          break;
        case 'r':
          doBreak = true;
          break;
        case 'w':
          {
            string newPath = imgPath;
            size_t i = newPath.find(".jpg");
            newPath.insert(i, "_lowpoly");
            cv::imwrite(newPath, output);
            doBreak = true;
            again = false;
            break;
          }
        case 'u':
          downscale = std::max(1, downscale - 1);
          doBreak = true;
          break;
        case 'd':
          downscale++;
          doBreak = true;
          break;
        case 'U':
          upscale++;
          doBreak = true;
          break;
        case 'D':
          upscale = std::max(1, upscale - 1);
          doBreak = true;
          break;
        default:
          break;
      }
      if (doBreak)
        break;
    }
  }
}

