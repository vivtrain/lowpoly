#include "cli_parser.h"
#include "argparse/argparse.hpp"
#include <cstdio>
#include <exception>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace std;

void CliOptions::parse(int argc, char* argv[]) {
  string programName = argv[0];
  programName = programName.substr(programName.find_last_of('/') + 1);
  argparse::ArgumentParser parser(programName);
  parser.set_usage_max_line_width(80);
  parser.add_usage_newline();
  parser.add_description("Low-poly image generator.");

  parser.add_argument("input")
    .help("Path to input image")
    .metavar("FILE");
  parser.add_argument("-o", "--output")
    .help("Output image path")
    .metavar("PATH").nargs(1);
  parser.add_argument("-s", "--preproc-scale")
    .help("Initial preprocessing scale factor")
    .metavar("SCALE")
    .default_value(preprocScale)
    .scan<'g', float>().nargs(1);
  parser.add_argument("-w", "--target-input-width")
    .help("Scale the input image to this size before processing (overrides -s)")
    .metavar("WIDTH")
    .scan<'i', int>().nargs(1);
  parser.add_argument("-S", "--postproc-scale")
    .help("Final postprocessing scale factor")
    .metavar("SCALE")
    .default_value(postprocScale)
    .scan<'g', float>().nargs(1);
  parser.add_argument("-W", "--target-output-width")
    .help("Scale the output image to this size after processing (overrides -S)")
    .metavar("WIDTH")
    .scan<'i', int>().nargs(1);
  parser.add_argument("-t", "--edge-threshold")
    .help("Minimum edge strength on the interval [0.0, 1.0]")
    .metavar("THRESHOLD")
    .default_value(edgeThreshold)
    .scan<'g', float>().nargs(1);
  parser.add_argument("-a", "--edge-aoe")
    .help("Area of effect of edges in adaptive non-max suppression")
    .metavar("RADIUS")
    .default_value(edgeAOE)
    .scan<'i', int>().nargs(1);
  parser.add_argument("-k", "--anms-kernel-range")
    .help("Range of adaptive non-max suppression kernel radius")
    .metavar("RANGE")
    .default_value(to_string(anmsKernelRange.first)
        + '-' + to_string(anmsKernelRange.second)).nargs(1);
  parser.add_argument("-p", "--salt-percent")
    .help("Frequency of random salt added prior to triangulation")
    .metavar("PROBABILITY")
    .default_value(saltPercent)
    .scan<'g', float>().nargs(1);
  parser.add_argument("--silent")
    .help("Suppress normal output") .flag();
  parser.add_argument("-i", "--interactive")
    .help("Use GUI to preview and supply an interactive loop").flag();
  parser.add_argument("-a", "--all")
    .help("Write all intermediate outputs to files").flag();

  try {
    parser.parse_args(argc, argv);
  } catch (const exception &e) {
    cerr << "\nMust provide an input image" << "\n\n" << parser.usage() << endl;
    exit(1);
  }
  // input path
  string inPath = parser.get("input");
  if (!ifstream(inPath).good())
    throw invalid_argument(inPath + " is not a readable file");
  inputPath = inPath; 
  // output path (default to same directory as input but with _lowpoly suffix)
  if (parser.present("--output")) {
    outputPath = triangulatedPath = vertexPath = sobelPath = parser.get("--output");
    size_t i = outputPath.find_last_of('/');
    i = outputPath.find('.', i);
    sobelPath.insert(i, "_sobel");
    vertexPath.insert(i, "_vertices");
    triangulatedPath.insert(i, "_triangulated");
  } else {
    outputPath = triangulatedPath = vertexPath = sobelPath = inputPath;
    size_t i = outputPath.find_last_of('/');
    i = outputPath.find('.', i);
    sobelPath.insert(i, "_sobel");
    vertexPath.insert(i, "_vertices");
    triangulatedPath.insert(i, "_triangulated");
    outputPath.insert(i, "_lowpoly");
  }
  // specify either target-input-width or preproc-scale, priority to former
  if (parser.present<int>("--target-input-width")) {
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
  if (parser.present<int>("--target-output-width")) {
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
  // silent
  silent = parser.get<bool>("--silent");
  // interactive
  interactive = parser.get<bool>("--interactive");
  // all
  all = parser.get<bool>("--all");
}

