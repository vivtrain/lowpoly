#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <sys/types.h>
#include <unordered_set>
#include <vector>
#include "delaunay/delaunay.hpp"
#include "delaunay/quadEdgeRef.hpp"

using namespace std;
using namespace QuadEdge;

void testSingleQuadEdge() {
  cout << "Testing a single QuadEdgeRef..." << endl;
  QuadEdgeRef *quadEdge = makeQuadEdge({0,0}, {1,2});

  QuadEdgeRef *self = quadEdge;
  assert(self != nullptr);
  QuadEdgeRef *rot = self->rot;
  assert(rot != nullptr);
  QuadEdgeRef *sym = rot->rot;
  assert(sym != nullptr);
  QuadEdgeRef *tor = sym->rot;
  assert(tor != nullptr);
  assert(tor->rot == self);
  cout << "✅  Verified circularity of rotations" << endl;

  assert(self->rot == rot);
  assert(self->sym() == sym);
  assert(self->rot->sym() == tor);
  assert(self->sym()->rot == tor);
  cout << "✅  Verified basic structure" << endl;

  assert(self->origCoords == cv::Point(0,0));
  assert(self->termCoords() == cv::Point(1,2));
  assert(!rot->origCoords.has_value());
  assert(!rot->termCoords().has_value());
  cout << "✅  Verified coordinates" << endl;

  assert(self->onext == self);
  assert(sym->onext == sym);
  cout << "✅  Verified circularity of real edges CCW" << endl;

  assert(rot->onext == tor);
  assert(tor->onext == rot);
  cout << "✅  Verified circularity of dual edges CCW" << endl;
  freeGraph(quadEdge);
}

void testTriangle() {
  cout << "Testing a triangle..." << endl;
  QuadEdgeRef *e1 = makeTriangle({0,0}, {0,2}, {1,1});
  assert(e1->lnext()->lnext()->lnext() == e1);
  cout << "✅  Verified triangular connnectivity" << endl;

  QuadEdgeRef *e2 = e1->lnext(), *e3 = e2->lnext();
  assert(e1->rot == e2->rot->onext);
  assert(e2->rot == e3->rot->onext);
  assert(e3->rot == e1->rot->onext);
  assert(e1->rot->sym() == e2->rot->sym()->oprev());
  assert(e2->rot->sym() == e3->rot->sym()->oprev());
  assert(e3->rot->sym() == e1->rot->sym()->oprev());
  assert(e1->rot != e1->rot->sym());
  cout << "✅  Verified faces" << endl;

  freeGraph(e1);
}

void testPolygon() {
  cout << "Testing a polygon..." << endl;
  vector<cv::Point> points = { {0,0}, {0,2}, {1,1}, {2,0} };
  QuadEdgeRef *e1 = makePolygon(points), *e = e1;
  for (uint i = 0; i < points.size(); i++)
    e = e->lnext();
  assert(e == e1);
  cout << "✅  Verified polygon connnectivity" << endl;

  QuadEdgeRef *e2 = e1->lnext();
  for (uint i = 0; i < points.size(); i++) {
    assert(e1->rot == e2->rot->onext);
    assert(e1->rot->sym() == e2->rot->sym()->oprev());
    e1 = e2;
    e2 = e2->lnext();
  }
  cout << "✅  Verified faces" << endl;

  freeGraph(e1);
}

void testConnect() {
  cout << "Test connecting a new edge..." << endl;
  QuadEdgeRef *quadrangle = makePolygon({ {0,0}, {0,2}, {1,3}, {2,2}, {2,0} });
  QuadEdgeRef *ab = quadrangle,
              *bc = ab->lnext(),
              *cd = bc->lnext(),
              *de = cd->lnext(),
              *ea = de->lnext();
  QuadEdgeRef *ad = connect(ab->sym(), cd->sym());
  assert(ad->onext == ab);
  assert(ad == ea->sym()->onext);
  assert(ad->sym()->onext == de);
  assert(ad->sym()->oprev() == cd->sym());
  assert(ad->rot == de->rot->onext);
  assert(ad->rot->sym() == ab->rot->onext);
  cout << "✅  Verified connection" << endl;
  freeGraph(quadrangle);
}

void testInCircle() {
  cout << "Testing InCircle..." << endl;
  assert(Delaunay::inCircle({2,2}, {6,0}, {8,6}, {4,2}));
  assert(Delaunay::inCircle({2,2}, {8,6}, {6,0}, {4,2}));
  assert(!Delaunay::inCircle({2,2}, {6,0}, {8,6}, {5,8}));
  assert(!Delaunay::inCircle({2,2}, {8,6}, {6,0}, {5,8}));
  cout << "✅  Verified circle test" << endl;
}

struct PointHash {
  size_t operator()(const cv::Point &p) const {
    return hasher(to_string(p.x) + to_string(p.y));
  }
  hash<string> hasher;
};

int main () {
  testSingleQuadEdge();
  testTriangle();
  testPolygon();
  testConnect();
  testInCircle();
  cout << "ALL TESTS PASSED!" << endl;
  cout << "(r)etry/(q)uit" << endl;
  while (true) {
    const int IMG_HEIGHT = 100, IMG_WIDTH = 100, N_POINTS = 100, SCALE = 10;
    unordered_set<cv::Point, PointHash> pointSet;
    pointSet.insert({1, 1});
    pointSet.insert({IMG_WIDTH-1, 1});
    pointSet.insert({1, IMG_HEIGHT-1});
    pointSet.insert({IMG_WIDTH-1, IMG_HEIGHT-1});
    cv::RNG rng(time(nullptr));
    while(pointSet.size() < N_POINTS + 4) {
      int x = rng.uniform(1, IMG_WIDTH-1);
      int y = rng.uniform(1, IMG_HEIGHT-1);
      pointSet.insert({x, y});
    }
    vector<cv::Point> points(pointSet.begin(), pointSet.end());
    QuadEdgeRef *graph = Delaunay::triangulate(points);
    vector<vector<cv::Point>> triangles
      = Delaunay::extractTriangles(graph);

    // printf("%zu Triangles:\n", triangles.size());
    // for (const auto &s : triangles) {
    //   for (const auto &point : s)
    //     printf("(%d,%d) ", point.x, point.y);
    //   printf("\n");
    // }

    for (auto &s : triangles)
      for (auto &point : s)
        point *= SCALE;
    for (auto &point : points)
      point *= SCALE;

    cv::Mat img(IMG_HEIGHT*SCALE, IMG_WIDTH*SCALE, CV_8UC3, cv::Scalar(100, 100, 100));
    for (const auto &triangle : triangles)
      cv::fillConvexPoly(img, triangle, cv::Scalar(0, 0, 0));
    cv::drawContours(img, triangles, -1, cv::Scalar(255, 100, 100), 2);
    for (const auto &point : points)
      cv::circle(img, point, 3, cv::Scalar(100, 0, 255), cv::FILLED);
    for (const auto &triangle : triangles) {
      cv::Point centroid(0, 0);
      for (const auto &point : triangle)
        centroid += point;
      centroid /= 3;
      cv::circle(img, centroid, 3, cv::Scalar(0, 255, 255));
    }
    cv::flip(img, img, 0);
    cv::imshow("delaunay output", img);

    int key = '_';
    while (true) {
      key = cv::waitKey(50);
      bool doBreak = false;
      switch(key) {
        case 'q':
          cv::destroyAllWindows();
          exit(0);
          break;
        case 'r':
          doBreak = true;
          break;
      }
      if (doBreak)
        break;
    }
  }
}

