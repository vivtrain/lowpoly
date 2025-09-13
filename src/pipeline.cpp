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

  float inScale = o.targetInputWidth.has_value()
    ? static_cast<float>(o.targetInputWidth.value()) / origSize.width
    : o.preprocScale;
  float outScale = o.targetOutputWidth.has_value()
    ? static_cast<float>(o.targetOutputWidth.value()) / origSize.width
    : o.postprocScale * inScale;

  const cv::Size inputSize(
      origSize.width * inScale,
      origSize.height * inScale);
  const cv::Size outputSize(
      origSize.width * outScale,
      origSize.height * outScale);
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
        inScale, inputSize.width, inputSize.height,
        outScale, outputSize.width, outputSize.height
  );

  // Scale the input
  cv::resize(img, inputImg, inputSize);
  if (!o.silent)
    printf("▲ Scaled for processing\n");
  if (o.interactive)
    cv::imshow(basename, inputImg);

  // Apply Sobel edge detector
  imgutil::sobelMagnitude(inputImg, sobelImg);
  if (!o.silent)
    printf("▲ Edges extracted\n");
  if (o.interactive)
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
  if (o.interactive)
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
      point = point / inScale * outScale;

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
  if (o.interactive)
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
  if (o.interactive)
    cv::imshow(basename + " - Output", outputImg);

}

