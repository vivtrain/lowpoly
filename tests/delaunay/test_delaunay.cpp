#include <cassert>
#include <cstdio>
#include <iostream>
#include <opencv2/core/types.hpp>
#include <sys/types.h>
#include <vector>
#include "delaunay/delaunay.hpp"
#include "delaunay/quadEdgeRef.hpp"

using namespace std;
using namespace QE;

void printEndpoints(QuadEdgeRef *edge, const char *label) {
  assert(edge->origCoords.has_value());
  assert(edge->termCoords().has_value());
  printf("%s: tailCoords (%d, %d), headCoords (%d, %d)\n",
      label,
      edge->origCoords->x,
      edge->origCoords->y,
      edge->termCoords()->x,
      edge->termCoords()->y);
}

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
  std::vector<cv::Point> points = { {0,0}, {0,2}, {1,1}, {2,0} };
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
  Delaunay d;
  assert(d.inCircle({2,2}, {6,0}, {8,6}, {4,2}));
  assert(d.inCircle({2,2}, {8,6}, {6,0}, {4,2}));
  assert(!d.inCircle({2,2}, {6,0}, {8,6}, {5,8}));
  assert(!d.inCircle({2,2}, {8,6}, {6,0}, {5,8}));
  cout << "✅  Verified circle test" << endl;
}

int main () {
  // testSingleQuadEdge();
  // testTriangle();
  // testPolygon();
  // testConnect();
  // testInCircle();
  // cout << "ALL TESTS PASSED!" << endl;
  Delaunay d;
  int IMG_HEIGHT = 6, IMG_WIDTH = 8;
  std::vector<std::vector<cv::Point>> triangles
    = d.extractSimplices(
        d.triangulate({
          {0,0}, {0,IMG_HEIGHT}, {IMG_WIDTH,0},// {IMG_WIDTH,IMG_HEIGHT},
          // {IMG_WIDTH/2, IMG_HEIGHT/2},
        }).first);
  for (const auto &triangle : triangles) {
    for (const auto &point : triangle) {
      printf("(%d,%d), ", point.x, point.y);
    }
    std::cout << std::endl;
  }
}

