#pragma once
#include "structures.hpp"
#include "utilities.hpp"
#include "tree.hpp"
#include <vector>
#include <set>
#include <fstream>
#include <map>
#include <set>
using namespace std;

vector<Point> findPoints(double x1, double y1, double mDist) {
  return {
      {x1 + mDist, y1}, // Right
      {x1 - mDist, y1}, // Left
      {x1, y1 + mDist}, // Up
      {x1, y1 - mDist}  // Down
  };
}

double radius(const std::vector<Point> &sinks) {
  double max_dist = 0;
  for (size_t i = 0; i < sinks.size(); ++i) {
    for (size_t j = i + 1; j < sinks.size(); ++j) {
      max_dist = std::max(max_dist, distance(sinks[i], sinks[j]));
    }
  }
  return max_dist / 2;
}

Point center(const std::vector<Point> &sinks, double r) {
  if (sinks.empty()) {
    return {0, 0};
  }

  if (sinks.size() == 1) {
    return {sinks[0].x, sinks[0].y};
  }

  // For multiple sinks, find the actual midpoint between them
  int min_x = sinks[0].x, max_x = sinks[0].x;
  int min_y = sinks[0].y, max_y = sinks[0].y;

  for (const auto &sink : sinks) {
    min_x = std::min(min_x, (int)sink.x);
    max_x = std::max(max_x, (int)sink.x);
    min_y = std::min(min_y, (int)sink.y);
    max_y = std::max(max_y, (int)sink.y);
  }

  // Calculate the midpoint
  double midpoint_x = (min_x + max_x) / 2.0;
  double midpoint_y = (min_y + max_y) / 2.0;

  // Round to nearest integer since we're working with a grid
  return {round(midpoint_x), round(midpoint_y)};
}

void linear_planar_dme_sub(std::vector<Point> &S_prime, const Point &P_S_prime,
                           std::vector<Point> &tree_points) {
  if (S_prime.size() == 1)
    return;
  // cout<<"DME SUB"<<endl;
  double r_prime = radius(S_prime);
  Point ms_v = center(S_prime, r_prime);
  ms_v = roundCoordinates(ms_v);
  // Add node v at ms_v to the tree
  tree_points.push_back(ms_v);
  // Divide S' into S1' and S2'
  std::vector<Point> S1_prime, S2_prime;

  bool identicalPoints = true; // Flag to detect identical points condition
  // Detect if all points are identical
  Point firstSink = S_prime.front();
  for (const auto &sink : S_prime) {
    if (sink.x != firstSink.x || sink.y != firstSink.y) {
      identicalPoints = false;
      break;
    }
  }

  if (identicalPoints) {
    // Handle identical points by creating two arbitrary groups
    // Arbitrarily choose the half for the partitioning
    S1_prime.assign(S_prime.begin(), S_prime.begin() + S_prime.size() / 2);
    S2_prime.assign(S_prime.begin() + S_prime.size() / 2, S_prime.end());
  } else {
    for (auto &sink : S_prime) {
      if (sink.x <= ms_v.x) {
        S1_prime.push_back(sink);
      } else if (sink.x > ms_v.x) {
        S2_prime.push_back(sink);
      }
    }

    // Attempt split by y-coordinate if x-coordinate doesn't work
    if (S1_prime.empty() || S2_prime.empty()) {
      S1_prime.clear();
      S2_prime.clear();
      for (auto &sink : S_prime) {
        if (sink.y <= ms_v.y) {
          S1_prime.push_back(sink);
        } else if (sink.y > ms_v.y) {
          S2_prime.push_back(sink);
        }
      }
    }
  }
  if (S1_prime.empty() || S2_prime.empty()) {
    S1_prime.clear();
    S2_prime.clear();
    // First try diagonal-based partitioning
    double diagonal_threshold = ms_v.x + ms_v.y;
    for (const auto &sink : S_prime) {
      if (sink.x + sink.y <= diagonal_threshold) {
        S1_prime.push_back(sink);
      } else {
        S2_prime.push_back(sink);
      }
    }
    // If diagonal partitioning fails, use quadrant-based approach
    if (S1_prime.empty() || S2_prime.empty()) {
      S1_prime.clear();
      S2_prime.clear();

      // Determine quadrants relative to merging point
      for (const auto &sink : S_prime) {
        if ((sink.x <= ms_v.x && sink.y <= ms_v.y) ||
            (sink.x > ms_v.x && sink.y > ms_v.y)) {
          S1_prime.push_back(sink);
        } else {
          S2_prime.push_back(sink);
        }
      }
      // Final fallback: force even split based on x+y coordinates
      if (S1_prime.empty() || S2_prime.empty()) {
        std::vector<Point> sorted_points = S_prime;
        std::sort(sorted_points.begin(), sorted_points.end(),
                  [](const Point &a, const Point &b) {
                    return (a.x + a.y) < (b.x + b.y);
                  });

        S1_prime.clear();
        S2_prime.clear();

        size_t mid = sorted_points.size() / 2;
        S1_prime.assign(sorted_points.begin(), sorted_points.begin() + mid);
        S2_prime.assign(sorted_points.begin() + mid, sorted_points.end());
      }
    }
  }
  print_points("DME Sinks S'", S_prime);
  print_points("DME Sinks S1'", S1_prime);
  print_points("DME Sinks S2'", S2_prime);
  cout << "DME Merging point: (" << ms_v.x << ", " << ms_v.y << ")" << endl;
  // Recursive calls
  linear_planar_dme_sub(S1_prime, ms_v, tree_points);
  linear_planar_dme_sub(S2_prime, ms_v, tree_points);
}

std::vector<Point> linear_planar_dme(std::vector<Point> &sinks,
                                     const Point &clk_location = {-1, -1}) {
  cout << "clock location dme: (" << clk_location.x << ", " << clk_location.y
       << ")" << endl;
  double r = radius(sinks);
  cout << " " << endl;
  // Build Manhattan Disks
  Point c_S = center(sinks, r);
  // Print the input sinks
  std::cout << "DME Input Sinks:" << std::endl;
  for (const auto &sink : sinks) {
    std::cout << "(" << sink.x << ", " << sink.y << ")" << std::endl;
  }
  cout << " " << endl;
  // Container for tree points
  std::vector<Point> tree_points;

  // Use clock location or center of sinks as starting point
  Point start_point =
      clk_location.x == -1 && clk_location.y == -1 ? c_S : clk_location;

  // tree_points.push_back(start_point);
  tree_points.push_back(roundCoordinates(start_point));
  // Recursive DME Sub
  linear_planar_dme_sub(sinks, c_S, tree_points);

  // Round all points in tree_points
  for (auto &point : tree_points) {
    point = roundCoordinates(point);
  }

  return tree_points;
}

double ZeroSkewMerge(Node *root, int id1, int id2) {
  // Use global variables for resistance and capacitance per unit length
  const double resistancePerUnitLength = wireUnits.resistance;
  const double capacitancePerUnitLength = wireUnits.capacitance;
  Node *parent = findLCA(root, id1, id2);
  cout << "Parent: " << parent->id << endl;
  Node *node1 = findNodeById(root, id1);
  Node *node2 = findNodeById(root, id2);
  // double delaySegment1 =
  //    getNodeDelay(root, parent->id) - getNodeDelay(root, id1);
  // double delaySegment2 =
  //    getNodeDelay(root, parent->id) - getNodeDelay(root, id2);
  double delaySegment1 = getNodeDelay(root, id1);
  double delaySegment2 = getNodeDelay(root, id2);
  // cout << delaySegment1 << endl;
  // cout << delaySegment2 << endl;
  double capacitanceSegment1 = getNodeCapacitance(root, id1);
  cout << "CapSeg1: " << capacitanceSegment1 << endl;
  double capacitanceSegment2 = getNodeCapacitance(root, id2);
  cout << "CapSeg2: " << capacitanceSegment2 << endl;
  int lengthOfWire = calculateManhattanDistance(root, id1, id2);
  double delayDifference = abs(delaySegment1 - delaySegment2);
  int sink1_distance, sink2_distance;
  cout << endl;
  double x1 = node1->x;
  double y1 = node1->y;
  double x2 = node2->x;
  double y2 = node2->y;
  cout << "x1: " << x1 << endl;
  cout << "y1: " << y1 << endl;
  cout << "x2: " << x2 << endl;
  cout << "y2: " << y2 << endl;
  cout << endl;
  // Calculate the initial merging point x
  double numerator =
      (delaySegment2 - delaySegment1) +
      resistancePerUnitLength * lengthOfWire *
          (capacitanceSegment2 + (capacitancePerUnitLength * lengthOfWire / 2));
  double denominator = resistancePerUnitLength * lengthOfWire *
                       (capacitancePerUnitLength * lengthOfWire +
                        capacitanceSegment1 + capacitanceSegment2);
  double mergingPointX = numerator / denominator;

  // Assign distances based on which sink should be closer
  if (delaySegment2 > delaySegment1) {
    // If sink2 has more delay, it should be closer to the merging point
    sink2_distance = ceil(mergingPointX * lengthOfWire);
    sink1_distance = floor((1 - mergingPointX) * lengthOfWire);
  } else {
    // If sink1 has more delay, it should be closer to the merging point
    sink1_distance = ceil(mergingPointX * lengthOfWire);
    sink2_distance = floor((1 - mergingPointX) * lengthOfWire);
  }

  if (mergingPointX >= 0 && mergingPointX <= 1) {
    cout << "Tapping point in range, calculating merging point X: "
         << mergingPointX << endl;
    cout << "length of wire: " << lengthOfWire << endl;
    cout << "merge point distance for sink 1 :" << sink1_distance << endl;
    cout << "merge point distance for sink 2 :" << sink2_distance << endl;
    vector<Point> points1 = findPoints(x1, y1, sink1_distance);
    vector<Point> points2 = findPoints(x2, y2, sink2_distance);
    cout << endl;
    cout << "All solutions for point 1 are:\n";
    for (const Point &point : points1) {
      cout << "(" << point.x << ", " << point.y << ")\n";
    }
    cout << endl;

    cout << "All solutions for point 2 are:\n";
    for (const Point &point : points2) {
      cout << "(" << point.x << ", " << point.y << ")\n";
    }
    cout << endl;
    vector<Point> solutions1, solutions2;
    cout << "Valid solutions for point 1 are:\n";
    for (const Point &point : points1) {
      if (point.x == x1 && point.x >= 0 && point.y >= 0) {
        solutions1.push_back(point);
      }
    }
    for (const Point &point : solutions1) {
      cout << "(" << point.x << ", " << point.y << ")\n";
    }
    cout << endl;
    cout << "Valid solutions for point 2 are:\n";
    for (const Point &point : points2) {
      if (point.x == x2 && point.x >= 0 && point.y >= 0) {
        solutions2.push_back(point);
      }
    }

    for (const Point &point : solutions2) {
      cout << "(" << point.x << ", " << point.y << ")\n";
    }

    // test
    std::vector<Point> solutions(solutions1.begin(), solutions1.end());
    // Append elements from the second vector to the new vector
    solutions = filterPoints(solutions, 1.5);
    solutions.insert(solutions.end(), solutions2.begin(), solutions2.end());
    // Call linear_planar_dme and get all merging points
    vector<Point> mergingPoints = linear_planar_dme(solutions);

    // Find the parent node and its ancestors
    Node *parent = findLCA(root, id1, id2);
    vector<Node *> ancestors;
    Node *current = parent;
    while (current != nullptr) {
      ancestors.push_back(current);
      current = findLCA(root, current->id, root->id);
      if (current == root)
        break;
    }

    // Assign merging points to parent and ancestors
    for (size_t i = 0; i < min(mergingPoints.size(), ancestors.size()); ++i) {
      Point roundedPoint = roundCoordinates(mergingPoints[i]);
      // Find nearest free point
      Point freePoint = findNearestFreePoint(root, roundedPoint);
      ancestors[i]->x = freePoint.x;
      ancestors[i]->y = freePoint.y;
      ancestors[i]->z = node1->z;
      ancestors[i]->node_type = "Merging Point";
    }

    return mergingPointX *
           lengthOfWire; // length for sink 1 = l*x, length for sink 2 = l*(1-x)

  } else {
    vector<Point> solutions;
    double lPrime = 0;
    int extension = 0;
    if (mergingPointX > 1) {
      // For x > 1, tapping point exactly on subtree 2
      cout << "Tapping point out of range( > 1), extending from Subtree 2"
           << endl;
      lPrime = (sqrt(pow(resistancePerUnitLength * capacitanceSegment1, 2) +
                     2 * resistancePerUnitLength * capacitancePerUnitLength *
                         (delaySegment2 - delaySegment1)) -
                resistancePerUnitLength * capacitanceSegment1) /
               (resistancePerUnitLength * capacitancePerUnitLength);
      extension = round(lPrime);

      double totalLength = lengthOfWire + extension;
      // Check if buffer is needed (1.5x threshold)
      if (totalLength > (lengthOfWire * 1.5)) {
        cout << "Adding buffer at merging point due to extended length: "
             << totalLength << " (original: " << lengthOfWire << ")" << endl;

        // Calculate required buffer delay to equalize delays
        double requiredBufferDelay = delayDifference;
        // Update parent node to include buffer
        // parent->isBuffered = true;
        parent->bufferDelay = requiredBufferDelay;
        parent->resistance = bufferUnits.outputResistance;
        parent->capacitance = bufferUnits.inputCapacitance;
        //  Add buffer delay to the path
        // parent->elmoreDelay = requiredBufferDelay;
        // parent->elmoreDelay += bufferUnits.intrinsicDelay;
        //  Recalculate delays considering buffer
        // delaySegment1 = getNodeDelay(root, parent->id);
        // delaySegment2 = getNodeDelay(root, parent->id);
        extension = 0;
        lPrime = lengthOfWire;
        cout << "Buffer with delay " << requiredBufferDelay
             << "fs inserted at merging point, length will not be extended"
             << endl;
      }
      cout << "lPrime rounded = " << round(lPrime) << endl;
      cout << "Extending L from " << lengthOfWire << " to "
           << extension + lengthOfWire << endl;
      vector<Point> points = findPoints(x2, y2, (extension + lengthOfWire));
      cout << endl;
      cout << "Unique points satisfying the equation are:\n";
      for (const Point &point : points) {
        cout << "(" << point.x << ", " << point.y << ")\n";
      }
      cout << endl;
      cout << "Valid solution(s) rooted at subtree 2 are:\n";
      for (const Point &point : points) {
        if (point.x == x2 && point.x >= 0 && point.y >= 0 &&
            point.x <= layout.width && point.y <= layout.height) {
          // if (point.x == x2 && point.x >= 0 && point.y >= 0) {
          solutions.push_back(point);
        }
      }
      for (const Point &point : solutions) {
        cout << "(" << point.x << ", " << point.y << ")\n";
      }
      // Call linear_planar_dme and get all merging points
      vector<Point> mergingPoints = linear_planar_dme(solutions);

      // Find the parent node and its ancestors
      Node *parent = findLCA(root, id1, id2);
      vector<Node *> ancestors;
      Node *current = parent;
      while (current != nullptr) {
        ancestors.push_back(current);
        current = findLCA(root, current->id, root->id);
        if (current == root)
          break;
      }

      // Assign merging points to parent and ancestors
      for (size_t i = 0; i < min(mergingPoints.size(), ancestors.size()); ++i) {
        Point roundedPoint = roundCoordinates(mergingPoints[i]);
        // Find nearest free point
        Point freePoint = findNearestFreePoint(root, roundedPoint);
        ancestors[i]->x = freePoint.x;
        ancestors[i]->y = freePoint.y;
        ancestors[i]->z = node1->z;
        ancestors[i]->node_type = "Merging Point";
      }

    } else {
      vector<Point> solutions;
      double lPrime = 0;
      int extension = 0;
      // For x < 0, tapping point on root of subtree 1
      cout << "Tapping point out of range( < 0), extending from Subtree 1"
           << endl;
      lPrime = (sqrt(pow(resistancePerUnitLength * capacitanceSegment2, 2) +
                     2 * resistancePerUnitLength * capacitancePerUnitLength *
                         (delaySegment1 - delaySegment2)) -
                resistancePerUnitLength * capacitanceSegment2) /
               (resistancePerUnitLength * capacitancePerUnitLength);
      extension = round(lPrime);

      double totalLength = lengthOfWire + extension;
      // Check if buffer is needed (1.5x threshold)
      if (totalLength > (lengthOfWire * 1.5)) {
        cout << "Adding buffer at merging point due to extended length: "
             << totalLength << " (original: " << lengthOfWire << ")" << endl;

        // Calculate required buffer delay to equalize delays
        double requiredBufferDelay = delayDifference;
        // Update parent node to include buffer
        // parent->isBuffered = true;
        parent->bufferDelay = requiredBufferDelay;
        parent->resistance = bufferUnits.outputResistance;
        parent->capacitance = bufferUnits.inputCapacitance;
        //  Add buffer delay to the path
        // parent->elmoreDelay = requiredBufferDelay;
        // parent->elmoreDelay += bufferUnits.intrinsicDelay;
        // Recalculate delays considering buffer
        // delaySegment1 = getNodeDelay(root, parent->id);
        // delaySegment2 = getNodeDelay(root, parent->id);
        extension = 0;
        lPrime = lengthOfWire;
        cout << "Buffer with delay " << requiredBufferDelay
             << "fs inserted at merging point, length will not be extended"
             << endl;
      }
      cout << "lPrime rounded = " << round(lPrime) << endl;
      cout << "L from " << lengthOfWire << " to " << extension + lengthOfWire
           << endl;
      vector<Point> points = findPoints(x1, y1, extension + lengthOfWire);
      cout << endl;
      cout << "Unique points satisfying the equation are:\n";
      for (const Point &point : points) {
        cout << "(" << point.x << ", " << point.y << ")\n";
      }
      cout << endl;
      cout << "Valid solution(s) rooted at subtree 1 are:\n";
      for (const Point &point : points) {
        if (point.x == x1 && point.x >= 0 && point.y >= 0 &&
            point.x <= layout.width && point.y <= layout.height) {
          // if (point.x == x1 && point.x >= 0 && point.y >= 0) {
          solutions.push_back(point);
        }
      }
      for (const Point &point : solutions) {
        cout << "(" << point.x << ", " << point.y << ")\n";
      }

      // Call linear_planar_dme and get all merging points
      vector<Point> mergingPoints = linear_planar_dme(solutions);

      // Find the parent node and its ancestors
      Node *parent = findLCA(root, id1, id2);
      vector<Node *> ancestors;
      Node *current = parent;
      while (current != nullptr) {
        ancestors.push_back(current);
        current = findLCA(root, current->id, root->id);
        if (current == root)
          break;
      }

      // Assign merging points to parent and ancestors
      for (size_t i = 0; i < min(mergingPoints.size(), ancestors.size()); ++i) {
        Point roundedPoint = roundCoordinates(mergingPoints[i]);
        // Find nearest free point
        Point freePoint = findNearestFreePoint(root, roundedPoint);
        ancestors[i]->x = freePoint.x;
        ancestors[i]->y = freePoint.y;
        ancestors[i]->z = node1->z;
        ancestors[i]->node_type = "Merging Point";
      }
    }
    cout << endl;
    return lPrime;
  }
}
// // Main recursive function to perform zero skew merging
Node *zeroSkewTree(Node *root) {
  if (!root || (root->leftChild == nullptr && root->rightChild == nullptr)) {
    return root;
  }

  root->leftChild = zeroSkewTree(root->leftChild);
  root->rightChild = zeroSkewTree(root->rightChild);

  if (hasPhysicalLocation(root->leftChild) &&
      hasPhysicalLocation(root->rightChild)) {
    ZeroSkewMerge(root, root->leftChild->id, root->rightChild->id);
    ZeroSkewMerges++;
    cout << "Merged At: (" << root->x << ", " << root->y << ")" << endl;
    cout << "ZeroSkewMerges: " << ZeroSkewMerges << endl;
  }
  return root;
}