#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/shape.hpp>

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

int main()
{
    cv::Mat img = cv::imread("../mrbluesky.jpg", CV_LOAD_IMAGE_COLOR);

    const int NUM_NODES = 20;
    Node* pixelCorners[NUM_NODES][NUM_NODES];

    for (int c = 0; c < NUM_NODES; c++)
        for (int r = 0; r < NUM_NODES; r++)
             pixelCorners[c][r] = new Node(
                                           cv::Point(c*img.cols/(NUM_NODES-1),
                                                     r*img.rows/(NUM_NODES-1)),
                                           img);

    for (int c = 0; c < NUM_NODES - 1; c++)
        for (int r = 0; r < NUM_NODES - 1; r++)
        {
            cv::Point neighbors[4];
            neighbors[0] = pixelCorners[c  ][r  ]->pos_;
            neighbors[1] = pixelCorners[c  ][r+1]->pos_;
            neighbors[2] = pixelCorners[c+1][r+1]->pos_;
            neighbors[3] = pixelCorners[c+1][r  ]->pos_;

            cv::Mat mask = getPolyMask(neighbors, 4, img.rows, img.cols);

            cv::Scalar avgColor = cv::mean(img, mask);
            cv::fillConvexPoly(img, neighbors, 4, avgColor);
        }


    cv::namedWindow("Mr. Blue Sky", CV_WINDOW_NORMAL);
    cv::resizeWindow("Mr. Blue Sky", 1000, 1000);
    cv::imshow("Mr. Blue Sky", img);

    cv::waitKey();
    cv::destroyAllWindows();
}
