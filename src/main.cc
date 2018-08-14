#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>

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

int main(int argc, char** argv)
{
    cv::Mat img = cv::imread("../mrbluesky.jpg", CV_LOAD_IMAGE_COLOR);

    Node* nd1 = new Node(cv::Point(img.cols/2,img.rows/3), img);
    Node* nd2 = new Node(cv::Point(img.cols/3,img.rows/2), img);
    Node* nd3 = new Node(cv::Point(img.cols/4,img.rows/4), img);

    cv::Point lilPolyVert[3] = { nd1->pos_, nd2->pos_, nd3->pos_ };
    cv::fillConvexPoly(img, lilPolyVert, 3, cv::mean(img));
    // nd1->addAdjacent(nd2);
    // nd1->addAdjacent(nd3);
    // nd2->addAdjacent(nd3);

    cv::namedWindow("Mr. Blue Sky", CV_WINDOW_NORMAL);
    cv::resizeWindow("Mr. Blue Sky", 1000, 1000);
    cv::imshow("Mr. Blue Sky", img);

    cv::waitKey();
    cv::destroyAllWindows();
}
