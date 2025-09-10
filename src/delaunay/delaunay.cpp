#include "delaunay/delaunay.hpp"
#include "delaunay/quadEdgeRef.hpp"
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

namespace Delaunay {

  struct comparePoints {
    bool operator()(const cv::Point &a, const cv::Point &b) const {
      return (a.x == b.x) ? (a.y < b.y) : (a.x < b.x);
    }
  };

  bool isCCW(cv::Point a, cv::Point b, cv::Point c) {
    float xa = a.x, ya = a.y;
    float xb = b.x, yb = b.y;
    float xc = c.x, yc = c.y;
    float data[3][3] = {
      { xa, ya, 1 },
      { xb, yb, 1 },
      { xc, yc, 1 },
    };
    cv::Mat ccwTest(3, 3, CV_32F, data);
    return cv::determinant(ccwTest) > 0;
  }

  bool isLeftOf(cv::Point test, QE::QuadEdgeRef *edge) {
    if (!edge->origCoords.has_value() || !edge->termCoords().has_value())
      throw std::logic_error("Edge not between vertices.");
    return isCCW(test, edge->origCoords.value(), edge->termCoords().value());
  }

  bool isRightOf(cv::Point test, QE::QuadEdgeRef *edge) {
    if (!edge->origCoords.has_value() || !edge->termCoords().has_value())
      throw std::logic_error("Edge not between vertices.");
    return isCCW(test, edge->termCoords().value(), edge->origCoords.value());
  }

  bool isAbove(QE::QuadEdgeRef *test, QE::QuadEdgeRef *baseL) {
    if (!baseL->origCoords.has_value() || !baseL->termCoords().has_value())
      throw std::logic_error("Edge (baseL) not between vertices.");
    if (!test->origCoords.has_value() || !test->termCoords().has_value())
      throw std::logic_error("Edge (test) not between vertices.");
    return isRightOf(test->termCoords().value(), baseL);
  }

  bool inCircle(cv::Point a, cv::Point b, cv::Point c, cv::Point test) {
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
    cv::Mat inCircleMat(4, 4, CV_32F, data);
    double inCircleDet = cv::determinant(inCircleMat);
    return isCCW(a, b, c) ? inCircleDet > 0 : inCircleDet < 0;
  }

  std::pair<QE::QuadEdgeRef*, QE::QuadEdgeRef*> triangulate_recurse(
      const std::vector<cv::Point> &points, uint first, uint last) {
    // Base case: 2 points => single quad-edge
    const uint N = last - first + 1, i = first, j = last;
    if (N < 2) {
      throw std::logic_error("Should never get here! Fewer than 2 points to "
          "triangulate.");
    } else if (N == 2) {
      QE::QuadEdgeRef *edge = QE::makeQuadEdge(points[i], points[j]);
      return { edge, edge->sym() };
    // Base case: 3 points => single triangle
    } else if (N == 3) {
      QE::QuadEdgeRef *ab = QE::makeQuadEdge(points[i], points[i+1]);
      QE::QuadEdgeRef *bc = QE::makeQuadEdge(points[i+1], points[i+2]);
      QE::splice(ab->sym(), bc);
      if (isCCW(points[i], points[i+1], points[i+2])) {
        QE::connect(bc, ab);
        return { ab, bc->sym() };
      } else if (isCCW(points[i], points[i+2], points[i+1])){
        QE::QuadEdgeRef *ca = QE::connect(bc, ab);
        return { ca->sym(), ca };
      } else {
        // Colinear (do not connect into a triangle)
        return { ab, bc->sym() };
      }
    // General case: 4+ points => recurse + merge
    } else {
      // Recurse on L and R -> left + right bounds
      uint middle = (first + last) / 2;
      printf("window between: (%d,%d) and (%d,%d)\n",
          points[i].x,
          points[i].y,
          points[j].x,
          points[j].y);
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
      QE::QuadEdgeRef *baseL = QE::connect(rdi->sym(), ldi);
      if (ldi->origCoords.value() == ldo->origCoords.value())
        ldo = baseL->sym();
      if (rdi->origCoords.value() == rdo->origCoords.value())
        rdo = baseL;
      QE::printEndpoints(ldo, "ldo");
      QE::printEndpoints(ldi, "ldi");
      QE::printEndpoints(rdi, "rdi");
      QE::printEndpoints(rdo, "rdo");
      // Merge L and R
      while (true) {
        // Determine the best L candidate
        QE::QuadEdgeRef *lcand = baseL->sym()->onext;
        if (isAbove(lcand, baseL)) {
          // Walk CCW around convex hull of L until we find a point not inCircle
          while (inCircle(baseL->termCoords().value(), baseL->origCoords.value(),
                lcand->termCoords().value(), lcand->onext->termCoords().value())) {
            QE::QuadEdgeRef *next = lcand->onext;
            QE::sever(lcand);
            lcand = next;
          }
        }
        // Determine the best R candidate
        QE::QuadEdgeRef *rcand = baseL->oprev();
        if (isAbove(rcand, baseL)) {
          // Walk CW around convex hull of R until we find a point not inCircle
          while (inCircle(baseL->termCoords().value(), baseL->origCoords.value(),
                rcand->termCoords().value(), rcand->oprev()->termCoords().value())) {
            QE::QuadEdgeRef *next = rcand->oprev();
            QE::sever(rcand);
            rcand = next;
          }
        }
        QE::printEndpoints(baseL, "baseL");
        QE::printEndpoints(lcand, "lcand");
        QE::printEndpoints(rcand, "rcand");
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
          baseL = QE::connect(rcand, baseL->sym());
        else
          baseL = QE::connect(baseL->sym(), lcand->sym());
      }
      printf("return\n");
      return { ldo, rdo };
    }
  }

  QE::QuadEdgeRef* triangulate(std::vector<cv::Point> points) {
    std::sort(points.begin(), points.end(),
        [](const cv::Point &a, const cv::Point &b) {
          return (a.x == b.x) ? (a.y < b.y) : (a.x < b.x);
        });
    printf("Sort order: ");
    for (const auto &point : points)
      printf("(%d, %d) ", point.x, point.y);
    printf("\n");
    return triangulate_recurse(points, 0, points.size()-1).first;
  }

  struct EdgeHash {
    size_t operator() (const std::pair<cv::Point, cv::Point> &edge) const {
      std::string concat
        = std::to_string(edge.first.x)
        + std::to_string(edge.first.y)
        + std::to_string(edge.second.x)
        + std::to_string(edge.second.y);
      std::hash<std::string> hasher;
      return hasher(concat);
    }
  };

  bool isEdgeMarked(QE::QuadEdgeRef *edge,
      const std::unordered_set<std::pair<cv::Point, cv::Point>, EdgeHash> &seen) {
    return seen.count({edge->origCoords.value(), edge->termCoords().value()}) > 0;
  }

  void markEdge(QE::QuadEdgeRef *edge,
      std::unordered_set<std::pair<cv::Point, cv::Point>, EdgeHash> &seen) {
    seen.insert({edge->origCoords.value(), edge->termCoords().value()});
    seen.insert({edge->termCoords().value(), edge->origCoords.value()});
  }

  void extractTriangles_recurse(
      QE::QuadEdgeRef *edge,
      std::vector<std::vector<cv::Point>> &triangles,
      std::unordered_set<std::pair<cv::Point, cv::Point>, EdgeHash> &seen,
      std::unordered_set<std::pair<cv::Point, cv::Point>, EdgeHash> &triangulated) {
    // Skip seen edges
    if (isEdgeMarked(edge, seen))
      return;
    // Traverse CCW until back at start
    const QE::QuadEdgeRef *firstEdge = edge;
    std::vector<QE::QuadEdgeRef*> polyEdges;
    do {
      polyEdges.push_back(edge);
      edge = edge->lnext();
    } while (edge != firstEdge);
    if (polyEdges.size() != 3) // Ignore the outside face (convex hull)
      return;
    // Extract triangle vertices, recurse along neighbors along the way
    std::vector<cv::Point> triangle;
    for (const auto &e : polyEdges) {
      triangle.push_back(e->origCoords.value());
      markEdge(e, seen);
      extractTriangles_recurse(e->oprev(), triangles, seen, triangulated);
    }
    // Check if all edges along path are already triangulated
    bool allTriangulated = true;
    for (const auto &e : polyEdges) {
      if (!isEdgeMarked(e, triangulated)) {
        allTriangulated = false;
        break;
      }
    }
    // If any edge is not triangulated, add the triangle to the list
    if (!allTriangulated) {
      triangles.push_back(triangle);
      for (const auto &e : polyEdges)
        markEdge(e, triangulated);
    }
  }

  std::vector<std::vector<cv::Point>> extractTriangles(
      QE::QuadEdgeRef *edge) {
    std::vector<std::vector<cv::Point>> simplices;
    std::unordered_set<std::pair<cv::Point, cv::Point>, EdgeHash> seen;
    std::unordered_set<std::pair<cv::Point, cv::Point>, EdgeHash> triangulated;
    extractTriangles_recurse(edge, simplices, seen, triangulated);
    return simplices;
  }

}
