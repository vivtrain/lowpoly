#include "pipeline.h"
#include <opencv2/core/base.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <stdexcept>
#include "cli_parser.h"
#include "delaunay/delaunay.h"
#include "img_util.h"

using namespace std;
using namespace quadedge;

void Pipeline::process(
    cv::Mat img,
    const std::string &basename,
    const CliOptions &o) {

  const cv::Size origSize(img.size());
  const cv::Size inputSize(
      origSize.width * o.preprocScale,
      origSize.height * o.preprocScale);
  const cv::Size outputSize(
      inputSize.width * o.postprocScale,
      inputSize.height * o.postprocScale);
  if (inputSize.width == 0 || inputSize.height == 0
      || outputSize.width == 0 || outputSize.height == 0)
    throw std::domain_error("Image left empty after scaling");

  if (!o.silent)
    printf("\n"
        "▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△ lowpoly generator "
        "△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽\n\n");
  if (!o.silent)
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
  cv::resize(img, inputImg, inputSize);
  if (!o.silent)
    printf("▲ Scaled for processing\n");
  if (!o.nonInteractive)
    cv::imshow(basename, inputImg);

  // Apply Sobel edge detector
  imgutil::sobelMagnitude(inputImg, sobelImg);
  if (!o.silent)
    printf("▲ Edges extracted\n");
  if (!o.nonInteractive)
    cv::imshow(basename + " - Sobel magnitude", sobelImg);

  // Apply non-max suppression
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
  if (!o.nonInteractive)
    cv::imshow(basename + " - Extracted vertices", vertexImg);

  // Extract vertices from the vertex image
  vector<cv::Point> vertices;
  cv::findNonZero(vertexImg, vertices);
  if (!o.silent)
    printf("• %zu Vertices extracted\n", vertices.size());
  // Construct the Delaunay triangulation of the vertex set
  QuadEdgeRef *triangulation = delaunay::triangulate(vertices);
  vector<vector<cv::Point>> ogTriangles
    = delaunay::extractTriangles(triangulation), upscaledTris(ogTriangles);
  if (!o.silent)
    printf("△ %zu Triangles generated\n", ogTriangles.size());

  // Scale the output back up
  for (auto &triangle : upscaledTris)
    for (auto &point : triangle)
      point *= o.postprocScale;

  // Build the triangulated image (just for show)
  triangulatedImg = cv::Mat::zeros(outputSize, CV_8UC3);
  cv::drawContours(
      triangulatedImg, upscaledTris,
      -1, cv::Scalar(200, 100, 100), 1, cv::LINE_AA);
  for (const auto &triangle : upscaledTris)
    for (auto &point : triangle)
      cv::circle(
          triangulatedImg, point,
          2, cv::Scalar(255, 0, 255), cv::FILLED, cv::LINE_AA);
  if (!o.silent)
    printf("▲ Triangulated\n");
  if (!o.nonInteractive)
    cv::imshow(basename + " - Triangulated", triangulatedImg);

  // Mark any areas not triangulated bright red (known bug)
  outputImg = cv::Mat(outputSize, CV_8UC3, cv::Scalar(0, 0, 255));

  // Generate the final lowpoly output
  for (uint i = 0; i < upscaledTris.size(); i++) {
    const auto &ogTri = ogTriangles[i];
    const auto &upTri = upscaledTris[i];
    cv::Scalar color = imgutil::avgColorInPoly(inputImg, ogTri);
    cv::fillConvexPoly(outputImg, upTri, color, cv::LINE_AA);
  }
  if (!o.silent)
    printf("▲ Output generated\n");
  if (!o.nonInteractive)
    cv::imshow(basename + " - Output", outputImg);

}

