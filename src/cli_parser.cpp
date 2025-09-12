#include "cli_parser.h"
#include "argparse/argparse.hpp"
#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <string>

using namespace std;

void CliOptions::parse(int argc, char* argv[]) {
  argparse::ArgumentParser parser("generate");
  parser.add_argument("input")
    .help("Path to input image");
  parser.add_argument("-o", "--output")
    .help("Output image path");
  parser.add_argument("-d", "--preproc-scale")
    .help("Initial preprocessing scale factor")
    .scan<'g', float>()
    .default_value(preprocScale);
  parser.add_argument("-s", "--target-input-width")
    .help("Scale the input image to this size before processing (overrides -d)")
    .scan<'i', int>();
  parser.add_argument("-U", "--postproc-scale")
    .help("Final postprocessing scale factor")
    .scan<'g', float>()
    .default_value(postprocScale);
  parser.add_argument("-S", "--target-output-width")
    .help("Scale the output image to this size after processing (overrides -U)")
    .scan<'i', int>();
  parser.add_argument("-t", "--edge-threshold")
    .help("Minimum edge strength on the interval [0.0, 1.0] (default 0.4)")
    .scan<'g', float>()
    .default_value(edgeThreshold);
  parser.add_argument("-a", "--edge-aoe")
    .help("Area of effect of edges in adaptive non-max suppression (default 5)")
    .scan<'i', int>()
    .default_value(edgeAOE);
  parser.add_argument("-k", "--anms-kernel-range")
    .help("Range of adaptive non-max suppression kernel radius (default 2-7)")
    .default_value(to_string(anmsKernelRange.first)
        + '-' + to_string(anmsKernelRange.second));
  parser.add_argument("-p", "--salt-percent")
    .help("Frequency of random salt added prior to triangulation (default 0.001)")
    .scan<'g', float>()
    .default_value(saltPercent);

  parser.parse_args(argc, argv);
  // input path
  string inPath = parser.get("input");
  if (!ifstream(inPath).good())
    throw invalid_argument(inputPath + " is not a readable file");
  inputPath = inPath; 
  // output path (default to same directory as input but with _lowpoly suffix)
  if (parser.present("--output")) {
    outputPath = parser.get("--output");
  } else {
    outputPath = inputPath;
    size_t i = outputPath.find_last_of('/');
    i = outputPath.find('.', i);
    outputPath.insert(i, "_lowpoly");
  }
  // specify either target-input-width or preproc-scale, priority to former
  if (parser.present("--target-input-width")) {
    int tiw = parser.get<int>("--target-input-width");
    if (tiw < 1)
      throw invalid_argument("Must supply a positive integer width for input");
    targetInputWidth = tiw;
  } else {
    float scale = parser.get<float>("--preproc-scale");
    if (scale <= 0.0f)
      throw invalid_argument("Must supply a positive float for scale");
    preprocScale = scale;
  }
  // specify either target-output-width or postproc-scale, priority to former
  if (parser.present("--target-output-width")) {
    int tow = parser.get<int>("--target-output-width");
    if (tow < 1)
      throw invalid_argument("Must supply a positive integer width for output");
    targetOutputWidth = tow;
  } else {
    float scale = parser.get<float>("--postproc-scale");
    if (scale <= 0.0f)
      throw invalid_argument("Must supply a positive float for scale");
    postprocScale = scale;
  }
  // edge threshold
  float et = parser.get<float>("--edge-threshold");
  if (et < 0.0f || et > 1.0f)
    throw invalid_argument("Edge threshold must be within [0.0, 1.0]");
  edgeThreshold = et;
  // edge aoe
  int aoe = parser.get<int>("--edge-aoe");
  if (aoe < 1)
    throw invalid_argument("Must supply a positive integer for edge AoE");
  edgeAOE = aoe;
  // aNMS kernel range: parse string in the form "min-max"
  string akr = parser.get("--anms-kernel-range");
  invalid_argument anmsExcp("Must supply an positive integer range (e.g. 2-7)");
  size_t dashPos = akr.find('-');
  if (dashPos == string::npos)
    throw anmsExcp;
  size_t lastParsed;
  uint start, end;
  string startStr = akr.substr(0, dashPos);
  start = stoi(startStr, &lastParsed);
  if (lastParsed != dashPos)
    throw anmsExcp;
  string endStr = akr.substr(dashPos+1);
  end = stoi(endStr, &lastParsed);
  if (lastParsed != endStr.length())
    throw anmsExcp;
  if (start < 1 || end < 1 || start > end)
    throw anmsExcp;
  anmsKernelRange = {start, end};
  // salt percent
  float sp = parser.get<float>("--salt-percent");
  if (sp < 0.0f || sp > 1.0f)
    throw invalid_argument("Salt percent value must be within [0.0, 1.0]");
  saltPercent = sp;
}

