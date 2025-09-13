#include <cstdio>
#include <cstdlib>
#include <exception>
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

#include "cli_parser.h"
#include "img_util.h"
#include "pipeline.h"

using namespace std;

int main(int argc, char *argv[]) {

  // Parse command-line arguments
  CliOptions opts;
  try {
    opts.parse(argc, argv);
  } catch (const exception &e) {
    cerr << "Error: " << e.what() << endl;
    exit(1);
  }
  const CliOptions &o(opts);

  // Read in an image from the specified path
  string basename = o.inputPath.substr(o.inputPath.find_last_of('/') + 1);
  cv::Mat img = cv::imread(o.inputPath);
  if (img.empty()) {
    cerr << "Error: A readable image was not found at " + o.inputPath << endl;
    exit(1);
  }

  // Set up the pipeline + interactive loop
  bool again = !o.nonInteractive;
  Pipeline pipeline;

  // Interactive state machine to preview and adjust output
  do {
    if (!o.nonInteractive)
      printf("\e[2J\e[H"); // clear screen

    // Do all the processing
    try {
      pipeline.process(img, basename, o);
    } catch (const exception &e) {
      cerr << "Error: " << e.what() << endl;
      exit(1);
    }

    if (!o.nonInteractive) {
      printf("\nIn any preview window:\n"
          "▶ q: quit\n"
          "▶ r: re-triangulate\n"
          "▶ w: write output\n"
          "▶ u: decrease input downscale (min of 1)\n"
          "▶ d: increase input downscale\n"
          "▶ U: increase output upscale\n"
          "▶ D: decrease output upscale (min of 1)\n");
    }
    char key = o.nonInteractive ? 'w' : '_';
    bool breakOuter;
    while (true) {
      breakOuter = true;
      key = cv::waitKey(30);
      switch(key) {
        case 'q':
          cv::destroyAllWindows();
          exit(0);
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
          breakOuter = false;
          break;
      }
      if (breakOuter)
        break;
    }
  } while(again);
}

