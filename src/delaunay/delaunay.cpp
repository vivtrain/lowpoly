#include "delaunay/delaunay.h"
#include "delaunay/quad_edge_ref.h"
#include <algorithm>
#include <cstddef>
#include <functional>
#include <opencv2/core.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

namespace delaunay {

  using namespace std;
  using namespace cv;
  using namespace quadedge;

  struct comparePoints {
    bool operator()(const Point &a, const Point &b) const {
      return (a.x == b.x) ? (a.y < b.y) : (a.x < b.x);
    }
  };

  bool isCCW(Point a, Point b, Point c) {
    float xa = a.x, ya = a.y;
    float xb = b.x, yb = b.y;
    float xc = c.x, yc = c.y;
    float data[3][3] = {
      { xa, ya, 1 },
      { xb, yb, 1 },
      { xc, yc, 1 },
    };
    Mat ccwTest(3, 3, CV_32F, data);
    return determinant(ccwTest) > 0;
  }

  bool isLeftOf(Point test, QuadEdgeRef *edge) {
    if (!edge->origCoords.has_value() || !edge->termCoords().has_value())
      throw logic_error("Edge not between vertices.");
    return isCCW(test, edge->origCoords.value(), edge->termCoords().value());
  }

  bool isRightOf(Point test, QuadEdgeRef *edge) {
    if (!edge->origCoords.has_value() || !edge->termCoords().has_value())
      throw logic_error("Edge not between vertices.");
    return isCCW(test, edge->termCoords().value(), edge->origCoords.value());
  }

  bool isAbove(QuadEdgeRef *test, QuadEdgeRef *baseL) {
    if (!baseL->origCoords.has_value() || !baseL->termCoords().has_value())
      throw logic_error("Edge (baseL) not between vertices.");
    if (!test->origCoords.has_value() || !test->termCoords().has_value())
      throw logic_error("Edge (test) not between vertices.");
    return isRightOf(test->termCoords().value(), baseL);
  }

  bool inCircle(Point a, Point b, Point c, Point test) {
    float xa = a.x, ya = a.y, a2 = a.dot(a);
    float xb = b.x, yb = b.y, b2 = b.dot(b);
    float xc = c.x, yc = c.y, c2 = c.dot(c);
    float xt = test.x, yt = test.y, t2 = test.dot(test);
    float data[4][4] = {
      { xa, ya, a2, 1 },
      { xb, yb, b2, 1 },
      { xc, yc, c2, 1 },
      { xt, yt, t2, 1 },
    };
    Mat inCircleMat(4, 4, CV_32F, data);
    double inCircleDet = determinant(inCircleMat);
    return isCCW(a, b, c) ? inCircleDet > 0 : inCircleDet < 0;
  }

  pair<QuadEdgeRef*, QuadEdgeRef*> triangulate_recurse(
      const vector<Point> &points, uint first, uint last) {
    // Base case: 2 points => single quad-edge
    const uint N = last - first + 1, i = first, j = last;
    if (N < 2) {
      throw logic_error("Should never get here! Fewer than 2 points to "
          "triangulate.");
    } else if (N == 2) {
      QuadEdgeRef *edge = makeQuadEdge(points[i], points[j]);
      return { edge, edge->sym() };
    // Base case: 3 points => single triangle
    } else if (N == 3) {
      QuadEdgeRef *ab = makeQuadEdge(points[i], points[i+1]);
      QuadEdgeRef *bc = makeQuadEdge(points[i+1], points[i+2]);
      splice(ab->sym(), bc);
      if (isCCW(points[i], points[i+1], points[i+2])) {
        connect(bc, ab);
        return { ab, bc->sym() };
      } else if (isCCW(points[i], points[i+2], points[i+1])){
        QuadEdgeRef *ca = connect(bc, ab);
        return { ca->sym(), ca };
      } else {
        // Colinear (do not connect into a triangle)
        return { ab, bc->sym() };
      }
    // General case: 4+ points => recurse + merge
    } else {
      // Recurse on L and R -> left + right bounds
      uint middle = (first + last) / 2;
      auto [ldo, ldi] = triangulate_recurse(points, first, middle);
      auto [rdi, rdo] = triangulate_recurse(points, middle+1, last);
      // Create the base cross edge (lower common tangent)
      while(true) {
        if (isLeftOf(rdi->origCoords.value(), ldi))
          ldi = ldi->lnext();
        else if (isRightOf(ldi->origCoords.value(), rdi))
          rdi = rdi->rprev();
        else
          break;
      }
      QuadEdgeRef *baseL = connect(rdi->sym(), ldi);
      if (ldi->origCoords.value() == ldo->origCoords.value())
        ldo = baseL->sym();
      if (rdi->origCoords.value() == rdo->origCoords.value())
        rdo = baseL;
      // Merge L and R
      while (true) {
        // Determine the best L candidate
        QuadEdgeRef *lcand = baseL->sym()->onext;
        if (isAbove(lcand, baseL)) {
          // Walk CCW around convex hull of L until we find a point not inCircle
          while (inCircle(
                baseL->termCoords().value(),
                baseL->origCoords.value(),
                lcand->termCoords().value(),
                lcand->onext->termCoords().value())) {
            QuadEdgeRef *next = lcand->onext;
            sever(lcand);
            lcand = next;
          }
        }
        // Determine the best R candidate
        QuadEdgeRef *rcand = baseL->oprev();
        if (isAbove(rcand, baseL)) {
          // Walk CW around convex hull of R until we find a point not inCircle
          while (inCircle(
                baseL->termCoords().value(),
                baseL->origCoords.value(),
                rcand->termCoords().value(),
                rcand->oprev()->termCoords().value())) {
            QuadEdgeRef *next = rcand->oprev();
            sever(rcand);
            rcand = next;
          }
        }
        // If neither candidate was valid, done (baseL is upper common tangent)
        bool lCandValid = isAbove(lcand, baseL);
        bool rCandValid = isAbove(rcand, baseL);
        if (!lCandValid && !rCandValid) {
          break;
        }
        // Choose the next cross edge to connect
        bool test = inCircle(lcand->termCoords().value(),
                             lcand->origCoords.value(),
                             rcand->origCoords.value(),
                             rcand->termCoords().value());
        if (!lCandValid || (rCandValid && test))
          baseL = connect(rcand, baseL->sym());
        else
          baseL = connect(baseL->sym(), lcand->sym());
      }
      return { ldo, rdo };
    }
  }

  QuadEdgeRef* triangulate(vector<Point> points) {
    sort(points.begin(), points.end(),
        [](const Point &a, const Point &b) {
          return (a.x == b.x) ? (a.y < b.y) : (a.x < b.x);
        });
    return triangulate_recurse(points, 0, points.size()-1).first;
  }

  struct EdgeHash {
    size_t operator() (const pair<Point, Point> &edge) const {
      string concat
        = to_string(edge.first.x)
        + to_string(edge.first.y)
        + to_string(edge.second.x)
        + to_string(edge.second.y);
      hash<string> hasher;
      return hasher(concat);
    }
  };

  bool isEdgeMarked(
      QuadEdgeRef *edge,
      const unordered_set<pair<Point, Point>, EdgeHash> &seen) {
    return seen.count(edge->points()) > 0;
  }

  void markEdge(
      QuadEdgeRef *edge,
      unordered_set<pair<Point, Point>, EdgeHash> &seen) {
    seen.insert(edge->points());
    seen.insert(edge->sym()->points());
  }

  bool isEdgeMapped(
      QuadEdgeRef *edge,
      const unordered_map<pair<Point, Point>, vector<Point>, EdgeHash> &map) {
    return map.count(edge->points()) > 0;
  }

  void extractTriangles_recurse(
      QuadEdgeRef *edge,
      vector<vector<Point>> &triangles,
      unordered_set<pair<Point, Point>, EdgeHash> &seen,
      unordered_map<pair<Point, Point>, vector<Point>*, EdgeHash> &triangulated) {
    // Skip seen edges
    if (isEdgeMarked(edge, seen))
      return;
    // Traverse CCW until back at start
    const QuadEdgeRef *firstEdge = edge;
    vector<QuadEdgeRef*> polyEdges;
    do {
      polyEdges.push_back(edge);
      edge = edge->lnext();
    } while (edge != firstEdge);
    if (polyEdges.size() != 3) // Ignore the outside face (convex hull)
      return;
    // Extract triangle vertices, recurse along neighbors along the way
    vector<Point> triangle;
    for (const auto &e : polyEdges) {
      triangle.push_back(e->origCoords.value());
      markEdge(e, seen);
      extractTriangles_recurse(e->oprev(), triangles, seen, triangulated);
    }
    // Check if all edges along path are already in the same triangle
    bool sameTriangle = true;
    vector<Point> *existingTriangle = nullptr;
    for (const auto &e : polyEdges) {
      const auto found = triangulated.find(e->points());
      if (found == triangulated.end()
          || (existingTriangle && found->second != existingTriangle)) {
        sameTriangle = false;
        break;
      }
      if (found->second)
        existingTriangle = found->second;
    }
    // If any edge is not triangulated or not in the same triangle, add it
    if (!sameTriangle) {
      triangles.push_back(triangle);
      for (const auto &e : polyEdges)
        triangulated[e->points()] = &triangle;
    }
  }

  vector<vector<Point>> extractTriangles(QuadEdgeRef *edge) {
    vector<vector<Point>> simplices;
    unordered_set<pair<Point, Point>, EdgeHash> seen;
    unordered_map< pair<Point, Point>, vector<Point>*, EdgeHash> triangulated;
    extractTriangles_recurse(edge, simplices, seen, triangulated);
    return simplices;
  }

}
