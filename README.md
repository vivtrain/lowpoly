# lowpoly
An OpenCV, C++ project that generates an aesthetic "low-poly" rendition of an image using a Delaunay triangulation algorithm.

## Project Dependencies
### OpenCV
[OpenCV](https://opencv.org/about/) is the premier open-source computer vision library. It abstracts and optimizes matrix operations commonly used in image processing and machine learning. For this project, the main modules (e.g. core, imgproc, imgcodecs, and highgui) should be sufficient. [Find Releases here](https://opencv.org/releases/) or search your favorite package manager. All this project cares about is that CMake can ```find_package``` it.

### GUI Features
Though this is primarily a CLI tool, it features GUI interactivity, so if you want that, be sure your environment can produce GUI windows (e.g. you may need to set up something like [X11](https://en.wikipedia.org/wiki/X_Window_System) if you are in a true CLI-only environment).

## Usage and Options
This tool allows for a high degree of customizability through command-line options (shoutout to [p-ranav/argparse](https://github.com/p-ranav/argparse) for the excellent library). For example, it may be desirable to downscale the input for better computational performance while upscaling the output to preserve sharpness and acuity. Other options apply to specific pipeline parameters and are given reasonable defaults. A description of the pipeline can be found below.
```
lowpoly [--help] [--version]
        [--output PATH] [--preproc-scale SCALE]
        [--target-input-width WIDTH] [--postproc-scale SCALE]
        [--target-output-width WIDTH] [--edge-threshold THRESHOLD]
        [--edge-aoe RADIUS] [--anms-kernel-range RANGE]
        [--salt-percent PROBABILITY] [--silent] [--interactive] [--all]
        FILE

Positional arguments:
  FILE                             Path to input image

Optional arguments:
  -h, --help                       shows help message and exits
  -v, --version                    prints version information and exits
  -o, --output PATH                Output image path
  -s, --preproc-scale SCALE        Initial preprocessing scale factor [default: 1]
  -w, --target-input-width WIDTH   Scale the input image to this size before processing (overrides -s)
  -S, --postproc-scale SCALE       Final postprocessing scale factor [default: 1]
  -W, --target-output-width WIDTH  Scale the output image to this size after processing (overrides -S)
  -t, --edge-threshold THRESHOLD   Minimum edge strength on the interval [0.0, 1.0] [default: 0.4]
  -a, --edge-aoe RADIUS            Area of effect of edges in adaptive non-max suppression [default: 5]
  -k, --anms-kernel-range RANGE    Range of adaptive non-max suppression kernel radius [default: "2-7"]
  -p, --salt-percent PROBABILITY   Frequency of random salt added prior to triangulation [default: 0.001]
  --silent                         Suppress normal output
  -i, --interactive                Use GUI to preview and supply an interactive loop
  -a, --all                        Write all intermediate outputs to files
```

## Pipeline
<div align="center">
  <img src="images/bluesky.jpg" alt="Original image" width="400"/>
  <p>Orignal image (photo of the Central Coast in California, USA)</p>
</div>

### Overview
1. Read in image and apply any scaling.
2. Extract edge information via Sobel filter.
3. Extract vertex information from edge data via non-max suppression.
4. Pass vertices to Delaunay triangulation algorithm.
5. Extract triangles from generated graph.
6. Determine average color in each triangle.
7. Scale geometric information for output.
8. Stitch together mosaic of colored triangles for final output :)

### Edge Detection
Why do we need to detect edges in the first place? Well, edges are really good places to put vertices in the image we are trying to create. Placing vertices for the triangulation on edges preserves the shape of objects in the image. In essence, we want a "connect-the-dots" style graph to triangulate.

There are many ways to approach the problem of edge detection, but since getting crisp, contiguous, binary edges (like [Canny](https://en.wikipedia.org/wiki/Canny_edge_detector)) is not the focus of this project, I opted for a very simple 2D filtering technique: [the Sobel operator](https://en.wikipedia.org/wiki/Sobel_operator). Applying the horizontal and vertical filters to our image results in a vector at each pixel, and we only care about the magnitude of this vector. In fact, the command-line option ```--edge-threshold``` directly operates on the magnitude of this Sobel vector, where ```0.0``` corresponds to a completely flat region and ```1.0``` corresponds to the maximum Euclidean distance between pixel vectors (e.g. an edge between black and white regions).

<div align="center">
  <img src="images/bluesky_sobel.jpg" alt="Original image" width="400px"/>
  <p>Orignal image (photo of the Central Coast in California, USA)</p>
</div>

### Adaptive Non-Max Suppression
```// TODO```

### Delaunay Triangulation
```// TODO```

### Color Extraction
```// TODO```























