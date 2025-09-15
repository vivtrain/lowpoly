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
#include <chrono>

inline std::chrono::time_point<std::chrono::steady_clock> now() {
  return std::chrono::steady_clock::now();
}

inline double elapsed(
    std::chrono::time_point<std::chrono::steady_clock> since) {
  return std::chrono::duration_cast<std::chrono::milliseconds>(now() - since).count() / 1000.0;
}

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
    cerr << "CLI Error: " << e.what() << endl;
    exit(1);
  }
  const CliOptions &o(opts);

  // Read in an image from the specified path
  string basename = o.inputPath.substr(o.inputPath.find_last_of('/') + 1);
  cv::Mat img = cv::imread(o.inputPath);
  if (img.empty()) {
    cerr << "Image Error: A readable image was not found at " + o.inputPath
      << endl;
    exit(1);
  }

  // Set up the pipeline + interactive loop
  bool again = o.interactive;
  Pipeline pipeline;

  // Set up prompt for interactive
  string promptIntro = "\nIn any preview window:\n";
  string promptOpt_q = "▶ q: quit\n";
  string promptOpt_r = "▶ r: re-triangulate\n";
  string promptOpt_w = "▶ w: write output\n";
  string promptOpt_a = "▶ a: write all intermediate pipeline steps + output\n";
  string promptOpt_u = "▶ u: upscale input\n";
  string promptOpt_d = "▶ d: downscale input\n";
  string promptOpt_U = "▶ U: upscale output\n";
  string promptOpt_D = "▶ D: downscale output\n";
  string prompt = promptIntro
    + promptOpt_q
    + promptOpt_r
    + promptOpt_w
    + promptOpt_a
    + (o.targetInputWidth.has_value() ? "" : promptOpt_u)
    + (o.targetInputWidth.has_value() ? "" : promptOpt_d)
    + (o.targetOutputWidth.has_value() ? "" : promptOpt_U)
    + (o.targetOutputWidth.has_value() ? "" : promptOpt_D);

  // Interactive state machine to preview and adjust output
  do {
    if (o.interactive)
      printf("\e[2J\e[H"); // clear screen

    // Do all the processing
    try {
      auto start = now();
      pipeline.process(img, basename, o);
      if (!o.silent)
        printf("⧖ Processed in %f seconds\n", elapsed(start));
    } catch (const exception &e) {
      cerr << "Pipeline Error: " << e.what() << endl;
      exit(1);
    }

    if (o.interactive) {
      printf("%s", prompt.c_str());
      char key = '_';
      bool breakOuter;
      while (true) { // poll keyboard input in GUI window
        breakOuter = true;
        key = cv::waitKey(30);
        switch(key) {
          case 'q':
            cv::destroyAllWindows();
            exit(0);
          case 'r':
            break;
          case 'a':
            opts.all = true;
            // fall through
          case 'w':
            cv::destroyAllWindows();
            if (o.all) {
              if (!o.silent) {
                printf("Writing Sobel output to %s\n",
                    o.sobelPath.c_str());
                printf("Writing vertex image to %s\n",
                    o.vertexPath.c_str());
                printf("Writing triangulation to %s\n",
                    o.triangulatedPath.c_str());
              }
              cv::imwrite(o.sobelPath, pipeline.sobelImg);
              cv::imwrite(o.vertexPath, pipeline.vertexImg);
              cv::imwrite(o.triangulatedPath, pipeline.triangulatedImg);
            }
            if (!o.silent)
              printf("Writing lowpoly output to %s\n", o.outputPath.c_str());
            cv::imwrite(o.outputPath, pipeline.outputImg);
            exit(0);
          case 'u':
            if (!o.targetInputWidth.has_value())
              opts.preprocScale *= 2;
            break;
          case 'd':
            if (!o.targetInputWidth.has_value())
              opts.preprocScale /= 2;
            break;
          case 'U':
            if (!o.targetOutputWidth.has_value())
              opts.postprocScale *= 2;
            break;
          case 'D':
            if (!o.targetOutputWidth.has_value())
              opts.postprocScale /= 2;
            break;
          default:
            breakOuter = false;
            break;
        }
        if (breakOuter)
          break;
      }
    } else {
      if (o.all) {
        if (!o.silent) {
          printf("Writing Sobel output to %s\n",
              o.sobelPath.c_str());
          printf("Writing vertex image to %s\n",
              o.vertexPath.c_str());
          printf("Writing triangulation to %s\n",
              o.triangulatedPath.c_str());
        }
        cv::imwrite(o.sobelPath, pipeline.sobelImg);
        cv::imwrite(o.vertexPath, pipeline.vertexImg);
        cv::imwrite(o.triangulatedPath, pipeline.triangulatedImg);
      }
      if (!o.silent)
        printf("Writing lowpoly output to %s\n", o.outputPath.c_str());
      cv::imwrite(o.outputPath, pipeline.outputImg);
    }
  } while(again);
}

