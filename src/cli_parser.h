#ifndef CLI_PARSER_HPP
#define CLI_PARSER_HPP

#include <optional>
#include <string>

struct CliOptions {
  void parse(int argc, char *argv[]);
  std::string inputPath;
  std::string sobelPath;
  std::string vertexPath;
  std::string triangulatedPath;
  std::string outputPath;
  float preprocScale = 1.0f;
  float postprocScale = 1.0f;
  std::optional<uint> targetInputWidth;
  std::optional<uint> targetOutputWidth;
  float edgeThreshold = 0.4f;
  std::pair<uint, uint> anmsKernelRange {2, 7};
  float saltRatio = 0.001f;
  bool silent = false;
  bool interactive = false;
  bool all = false;
};

#endif // !CLI_PARSER_HPP

