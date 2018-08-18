#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/shape.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <string>

const int MAX_NEIGHBORS = 6;

class Node
{
  public:
    cv::Point pos_;

    Node(cv::Point pos, cv::Mat img)
      : pos_(pos), img_(img), numNeighbors_(0)
    {
        for (int i = 0; i < MAX_NEIGHBORS; i++)
            neighbors_[i] = nullptr;
        if (pos_.x > img.cols)
            pos_.x = img.cols - 1;
        if (pos_.y > img.rows)
            pos_.y = img.rows - 1;
        cv::circle(img, pos, 1, cv::Scalar(0,0,0), 20);
    }

    bool addAdjacent(Node* neighbor)
    {
        if (numNeighbors_ >= MAX_NEIGHBORS)
            return false;
        neighbors_[numNeighbors_] = neighbor;
        numNeighbors_++;
        cv::line(img_, pos_, neighbor->pos_, cv::Scalar(10,10,10), 10);
        return true;
    }

  private:
    cv::Mat img_;
    Node* neighbors_[MAX_NEIGHBORS];
    int numNeighbors_;
};

cv::Mat getPolyMask(cv::Point* vertices, int npts, int rows, int cols)
{
    cv::Mat mask = cv::Mat::zeros(rows, cols, CV_8U);
    cv::fillConvexPoly(mask, vertices, npts, cv::Scalar(1));
    return mask;
}

int main(int argc, char *argv[])
{
    std::string filename = "../";
    filename+= argv[1];

    cv::RNG rng;

    cv::Mat img = cv::imread(filename, CV_LOAD_IMAGE_COLOR);
    cv::Mat edgy, dilated, eroded, blurry;
    cv::Mat blank = cv::Mat::zeros(img.rows, img.cols, CV_8UC3);

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;

    int blurSize = img.cols/100 - !(img.cols/100 % 2);
    //cv::GaussianBlur(img, blurry, cv::Size(blurSize, blurSize), 0);
    cv::Canny(img, edgy, 100, 200);
    //cv::GaussianBlur(edgy, blurry, cv::Size(3, 3), 0);
    //blurry = (blurry != 0);
    // cv::dilate(edgy, dilated, cv::Mat::ones(4,4,0));
    // cv::erode(dilated, eroded, cv::Mat::ones(2,2,0));
    cv::findContours(edgy, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    for (int c = 0; c < contours.size(); c++)
        if (cv::contourArea(contours[c]) > 10)
            cv::drawContours(blank,
                            contours,
                            c,
                            cv::Scalar(rng.uniform(100, 255),
                                        rng.uniform(100, 255),
                                        rng.uniform(100, 255),1));

    cv::namedWindow("Mr. Blue Sky", CV_WINDOW_NORMAL);
    cv::resizeWindow("Mr. Blue Sky", 500, 500);
    cv::imshow("Mr. Blue Sky", img);

    cv::namedWindow("output", CV_WINDOW_NORMAL);
    cv::resizeWindow("output", 500, 500);
    cv::imshow("output", blank);

    cv::waitKey();
    cv::destroyAllWindows();
}

