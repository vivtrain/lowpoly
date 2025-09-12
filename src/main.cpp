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
#include "img_util.h"
#include "pipeline.h"

int main(int argc, char *argv[]) {

  // Parse command-line arguments
  CliOptions opts;
  opts.parse(argc, argv);
  const CliOptions &o(opts);

  // Read in an image from the specified path
  std::string basename = o.inputPath.substr(o.inputPath.find_last_of('/') + 1);
  cv::Mat img = cv::imread(o.inputPath);
  if (img.empty())
    CV_Error(cv::Error::StsObjectNotFound,
        "A readable image was not found at " + o.inputPath);

  // Set up the pipeline + interactive loop
  bool again = !o.nonInteractive;
  Pipeline pipeline;

  // Interactive state machine to preview and adjust output
  while (again) {
    if (!o.nonInteractive)
      printf("\e[2J\e[H"); // clear screen

    // Do all the processing
    pipeline.process(img, basename, o);

    if (!o.nonInteractive) {
      printf("\nIn any preview window:\n"
          "▶ q: quit\n"
          "▶ r: re-triangulate\n"
          "▶ w: write output\n"
          "▶ u: decrease input downscale (min of 1)\n"
          "▶ d: increase input downscale\n"
          "▶ U: increase output upscale\n"
          "▶ D: decrease output upscale (min of 1)\n");
      char key = '_';
      key = cv::waitKey();
      switch(key) {
        case 'q':
          cv::destroyAllWindows();
          again = false;
          break;
        case 'r':
          break;
        case 'w':
          {
            cv::imwrite(o.outputPath, pipeline.outputImg);
            again = false;
            break;
          }
        case 'u':
          opts.preprocScale *= 2;
          break;
        case 'd':
          opts.preprocScale /= 2;
          break;
        case 'U':
          opts.postprocScale *= 2;
          break;
        case 'D':
          opts.postprocScale /= 2;
          break;
        default:
          break;
      }
    } else {
      cv::imwrite(o.outputPath, pipeline.outputImg);
    }
  }
}

