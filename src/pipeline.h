#ifndef PIPELINE_H
#define PIPELINE_H

#include "cli_parser.h"
#include <opencv2/core/mat.hpp>
#include <string>

struct Pipeline {
  void process(
      cv::Mat img,
      const std::string &basename,
      const CliOptions &o);
  cv::Mat inputImg, sobelImg, vertexImg, triangulatedImg, outputImg;
};

#endif // !PIPELINE_H
