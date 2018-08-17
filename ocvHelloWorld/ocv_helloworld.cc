#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/shape.hpp>

using namespace std;
using namespace cv;

int main(int argc, char** argv) {
    Mat img(512, 512, CV_8UC3, Scalar(0));

    putText(img,
        "OpenCV on Jetson TX1",
        Point(10, img.rows / 2),
        FONT_HERSHEY_DUPLEX,
        1.0,
        CV_RGB(118, 185, 0),
        2);

    imshow("Hello World", img);
    waitKey();
}
