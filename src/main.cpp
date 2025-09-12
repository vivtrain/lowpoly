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
#include "cli_parser.h"
#include "delaunay/delaunay.h"
#include "delaunay/quad_edge_ref.h"
#include "img_util.h"

using namespace std;
using namespace quadedge;

int main(int argc, char *argv[]) {

  // Parse command-line arguments
  CliOptions opts;
  opts.parse(argc, argv);
  const CliOptions &o(opts);

  bool again = true;
  // Read in an image from the specified path
  string basename = o.inputPath.substr(o.inputPath.find_last_of('/') + 1);
  cv::Mat img = cv::imread(o.inputPath);
  if (img.empty())
    CV_Error(cv::Error::StsObjectNotFound,
        "A readable image was not found at " + o.inputPath);
  cv::Size origSize(img.size());
  while (again) {
    cv::Size inputSize(
        origSize.width * o.preprocScale,
        origSize.height * o.preprocScale);
    cv::Size outputSize(
        inputSize.width * o.postprocScale,
        inputSize.height * o.postprocScale);
    printf("\e[2J\e[H\n"
        "▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△ lowpoly generator "
        "△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽\n\n");
    printf(
        "Image: %s\n"
        "Original size (w x h): (%d, %d)\n"
        "Pre-process scaling: %.3f -> (%d, %d)\n"
        "Post-process scaling: %.3f -> (%d, %d)\n",
        o.inputPath.c_str(),
        origSize.width, origSize.height,
        o.preprocScale, inputSize.width, inputSize.height,
        o.postprocScale, outputSize.width, outputSize.height
    );

    // Scale the input
    cv::Mat inputImg;
    cv::resize(img, inputImg, inputSize);
    cv::imshow(basename, inputImg);
    printf("▲ Scaled for processing\n");

    // Apply Sobel edge detector
    cv::Mat sobelImg;
    imgutil::sobelMagnitude(inputImg, sobelImg);
    printf("▲ Edges extracted\n");
    cv::imshow(basename + " - Sobel magnitude", sobelImg);

    // Apply non-max suppression
    cv::Mat vertexImg;
    imgutil::adaptiveNonMaxSuppress(
        sobelImg, vertexImg,
        o.anmsKernelRange, o.edgeAOE, o.edgeThreshold);
    // Salt the image with extra vertices at random
    imgutil::salt(vertexImg, o.saltPercent);
    // Include the corners
    float maxValue = imgutil::getImageRange(vertexImg.type()).second;
    vertexImg.at<float>({0, 0}) = 
    vertexImg.at<float>({0, vertexImg.rows - 1}) = 
    vertexImg.at<float>({vertexImg.cols - 1, 0}) = 
    vertexImg.at<float>({vertexImg.cols - 1, vertexImg.rows - 1}) = maxValue;
    cv::imshow(basename + " - Extracted vertices", vertexImg);

    // Extract vertices from the vertex image
    vector<cv::Point> vertices;
    cv::findNonZero(vertexImg, vertices);
    printf("• %zu Vertices extracted\n", vertices.size());
    // Construct the Delaunay triangulation of the vertex set
    QuadEdgeRef *triangulation = delaunay::triangulate(vertices);
    vector<vector<cv::Point>> ogTriangles
      = delaunay::extractTriangles(triangulation), upscaledTris(ogTriangles);
    printf("△ %zu Triangles generated\n", ogTriangles.size());

    // Scale the output back up
    for (auto &triangle : upscaledTris)
      for (auto &point : triangle)
        point *= o.postprocScale;

    // Build the triangulated image (just for show)
    cv::Mat triangulated = cv::Mat::zeros(outputSize, CV_8UC3);
    cv::drawContours(
        triangulated, upscaledTris,
        -1, cv::Scalar(200, 100, 100), 1, cv::LINE_AA);
    for (const auto &triangle : upscaledTris)
      for (auto &point : triangle)
        cv::circle(
            triangulated, point,
            2, cv::Scalar(255, 0, 255), cv::FILLED, cv::LINE_AA);
    printf("▲ Triangulated\n");
    cv::imshow(basename + " - Triangulated", triangulated);

    // Mark any areas not triangulated bright red (known bug)
    cv::Mat output(outputSize, CV_8UC3, cv::Scalar(0, 0, 255));

    // Generate the final lowpoly output
    for (uint i = 0; i < upscaledTris.size(); i++) {
      const auto &ogTri = ogTriangles[i];
      const auto &upTri = upscaledTris[i];
      cv::Scalar color = imgutil::avgColorInPoly(inputImg, ogTri);
      cv::fillConvexPoly(output, upTri, color, cv::LINE_AA);
    }
    printf("▲ Output generated\n");
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
            cv::imwrite(o.outputPath, output);
            doBreak = true;
            again = false;
            break;
          }
        case 'u':
          opts.preprocScale *= 2;
          doBreak = true;
          break;
        case 'd':
          opts.preprocScale /= 2;
          doBreak = true;
          break;
        case 'U':
          opts.postprocScale *= 2;
          doBreak = true;
          break;
        case 'D':
          opts.postprocScale /= 2;
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

