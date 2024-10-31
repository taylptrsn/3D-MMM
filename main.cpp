#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>
#include "structures.hpp"
#include "utilities.hpp"
#include "trees.hpp"
#include "clusters.hpp"
/* TODO
- Delay buffering instead of wire elongation (if wire is elongated more
than 1.5x original length insert a multiple of the buffer, add buffer attribute
to node) WIP
- Unit Conversion at Data read-in
- Split into header files
- Account for Vertical wirelength
- Add validity check for merging point solutions (within Die area bound) WIP
- Check delay calculations
*/
using namespace std;

// Global Variables
Layout layout;
WireUnits wireUnits;
BufferUnits bufferUnits;
TSVUnits tsvUnits;
//ClockSource clockSource;

vector<Sink> sinks;
vector<string> dieColors = {"Gray", "Red",    "Green",
                            "Blue", "Purple", "Lime"}; // Define colors for dies



// Core Logic

// Function to calculate the Manhattan distance between two nodes by their IDs
int calculateManhattanDistance(Node *root, int id1, int id2) {
  Node *node1 = findNodeById(root, id1);
  Node *node2 = findNodeById(root, id2);
  if (node1 == nullptr || node2 == nullptr) {
    cout << "One of the nodes could not be found." << endl;
    return -1;
  }

  int x1 = node1->sinks.front().x;
  int y1 = node1->sinks.front().y;
  // int z1 = node1->sinks.front().z;
  int x2 = node2->sinks.front().x;
  int y2 = node2->sinks.front().y;
  // int z2 = node2->sinks.front().z;
  return abs(x2 - x1) + abs(y2 - y1); // Manhattan distance  + abs(z2 - z1)
}
// Function to assign physical locations from the sink objects to their leaf
// nodes
void assignPhysicalLocations(Node *node) {
  if (!node) {
    return;
  }

  if (node->leftChild == nullptr && node->rightChild == nullptr) {
    if (!node->sinks.empty()) {
      // Assign the location of the first sink to the leaf node
      node->x = node->sinks.front().x;
      node->y = node->sinks.front().y;
      node->z = node->sinks.front().z;
      node->node_type = "Leaf";
    }
  }

  // Recursively assign physical locations to child nodes
  assignPhysicalLocations(node->leftChild);
  assignPhysicalLocations(node->rightChild);
}

// Helper function to check if a node has a physical location
bool hasPhysicalLocation(const Node *node) {
  return node != nullptr && !(node->x == -1 && node->y == -1 && node->z == -1);
}

double depthFirstCapacitance(Node *node) {
  if (!node)
    return 0.0; // Base case

  double nodeCapacitance = 0.0;

  // If the current node is a buffer input node, only add its capacitance
  if (node->isBuffered) {
    // Assuming the node's capacitance includes its own and not its children's
    nodeCapacitance += node->capacitance;
  } else {
    // Calculate total capacitance for non-leaf nodes recursively
    if (node->leftChild)
      nodeCapacitance += depthFirstCapacitance(node->leftChild);
    if (node->rightChild)
      nodeCapacitance += depthFirstCapacitance(node->rightChild);

    // Add capacitance of the current node (for leaf nodes, sum the capacitances
    // of all sinks in the node)
    if (!node->leftChild && !node->rightChild) {
      for (const auto &sink : node->sinks) {
        nodeCapacitance += sink.inputCapacitance;
      }
    }
  }
  node->capacitance =
      nodeCapacitance; // Store the calculated capacitance at the current node

  return nodeCapacitance;
}

void depthFirstDelay(Node *node, double accumulatedResistance) {
  if (!node)
    return; // Base case: node is null

  // Calculate the total resistance from the root to this node
  double totalResistance = accumulatedResistance + node->resistance;
  double delay = 0.0; // Initialize delay calculation variable

  // If the node is buffered, add the buffer delay to the node's delay
  // calculation
  if (node->isBuffered) {
    // if buffered, intrinsicdelay=17000 fS
    delay = bufferUnits.intrinsicDelay; //+(bufferUnits.outputResistance *
                                        // node->capacitance);
  }
  // For leaf nodes, calculate delay directly
  if (!node->leftChild && !node->rightChild) {
    node->elmoreDelay = totalResistance * node->capacitance + delay;
  } else {
    // For internal nodes, delay is calculated based on the subtree capacitance
    double subtreeCapacitance = node->capacitance;
    node->elmoreDelay = totalResistance * subtreeCapacitance + delay;
  }
  // Recursively calculate delay for child nodes, passing the total accumulated
  // resistance to each child
  if (node->leftChild)
    depthFirstDelay(node->leftChild, totalResistance);
  if (node->rightChild)
    depthFirstDelay(node->rightChild, totalResistance);
}

// Function to calculate hierarchical delay for the entire tree
void hierarchicalDelay(Node *node) {
  depthFirstCapacitance(node);
  depthFirstDelay(node, clockSource.outputResistance);
}
double getNodeCapacitance(Node *node, int id) {
  if (node == nullptr) {
    return -1.0;
  }
  if (node->id == id) {
    return node->capacitance;
  }
  double leftSearch = getNodeCapacitance(node->leftChild, id);
  if (leftSearch >= 0) {
    return leftSearch;
  }
  return getNodeCapacitance(node->rightChild, id);
}

double getNodeDelay(Node *node, int id) {
  if (node == nullptr) {
    return -1.0;
  }
  if (node->id == id) {
    return node->elmoreDelay;
  }
  double leftSearch = getNodeDelay(node->leftChild, id);
  if (leftSearch >= 0) {
    return leftSearch;
  }
  return getNodeDelay(node->rightChild, id);
}

double distance(const Point &a, const Point &b) {
  return std::abs(a.x - b.x) + std::abs(a.y - b.y);
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
void print_points(const std::string &label, const std::vector<Point> &points) {
  std::cout << label << ": ";
  for (const auto &point : points) {
    std::cout << "(" << point.x << ", " << point.y << ") ";
  }
  std::cout << std::endl;
}

Point roundCoordinates(const Point &p) {
  // Ensure one coordinate is rounded up and the other is rounded down
  if (p.x - floor(p.x) >= 0.5) {
    return {ceil(p.x), floor(p.y)};
  } else {
    return {floor(p.x), ceil(p.y)};
  }
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
  print_points("DME Sinks S'", S_prime);
  print_points("DME Sinks S1'", S1_prime);
  print_points("DME Sinks S2'", S2_prime);
  cout << "DME Merging point: (" << ms_v.x << ", " << ms_v.y << ")" << endl;
  // Recursive calls
  linear_planar_dme_sub(S1_prime, ms_v, tree_points);
  linear_planar_dme_sub(S2_prime, ms_v, tree_points);
}


std::vector<Point> linear_planar_dme(std::vector<Point> sinks,
                                     Point &clk_location) {
  cout << "clock location dme: (" << clk_location.x << ", " << clk_location.y
       << ")" << endl;

  // Store original sink locations
  std::set<pair<double, double>> sink_locations;
  for (const auto &sink : sinks) {
      sink_locations.insert({sink.x, sink.y});
  }
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

Node *findLCA(Node *root, int node1ID, int node2ID) {
  if (root == nullptr || root->id == node1ID || root->id == node2ID) {
    return root;
  }

  Node *leftLCA = findLCA(root->leftChild, node1ID, node2ID);
  Node *rightLCA = findLCA(root->rightChild, node1ID, node2ID);

  if (leftLCA && rightLCA)
    return root;
  return (leftLCA != nullptr) ? leftLCA : rightLCA;
}


// Function to filter points based on distance
vector<Point> filterPoints(const vector<Point> &points, double minDistance) {
  vector<Point> filteredPoints;
  for (const auto &point : points) {
    bool tooClose = false;
    for (const auto &filteredPoint : filteredPoints) {
      if (euclideanDistance(point, filteredPoint) < minDistance) {
        tooClose = true;
        break;
      }
    }
    if (!tooClose) {
      filteredPoints.push_back(point);
    }
  }
  return filteredPoints;
}

double ZeroSkewMerge(Node *root, int id1, int id2, Point clk_location) {
  // Use global variables for resistance and capacitance per unit length
  cout << "clock location ZSM: " << clk_location.x << "," << clk_location.y
       << endl;
  double resistancePerUnitLength = wireUnits.resistance;
  double capacitancePerUnitLength = wireUnits.capacitance;
  Node *parent = findLCA(root, id1, id2);
  cout << "Parent: " << parent->id << endl;
  Node *node1 = findNodeById(root, id1);
  Node *node2 = findNodeById(root, id2);
  //for buffering
  int originalLength = calculateManhattanDistance(root, id1, id2);
  double delaySegment1 =
      getNodeDelay(root, parent->id) - getNodeDelay(root, id1);
  double delaySegment2 =
      getNodeDelay(root, parent->id) - getNodeDelay(root, id2);

  double capacitanceSegment1 = getNodeCapacitance(root, id1);
  cout << "CapSeg1: " << capacitanceSegment1 << endl;
  double capacitanceSegment2 = getNodeCapacitance(root, id2);
  cout << "CapSeg2: " << capacitanceSegment2 << endl;
  int lengthOfWire = calculateManhattanDistance(root, id1, id2);

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

  if (x1 == x2 && y1 == y2) {
    cout << "Points are the same. Merging point set to: (" << x1 << ", " << y1
         << ")" << endl;
    parent->x = x1;
    parent->y = y1;
    parent->z = node1->z; // Assign same z tier as parents
    parent->node_type = "Merging Point";
    return 0; 
  }
  // Calculate the initial merging point x
  double numerator =
      (delaySegment2 - delaySegment1) +
      resistancePerUnitLength * lengthOfWire *
          (capacitanceSegment2 + (capacitancePerUnitLength * lengthOfWire / 2));
  double denominator = resistancePerUnitLength * lengthOfWire *
                       (capacitancePerUnitLength * lengthOfWire +
                        capacitanceSegment1 + capacitanceSegment2);
  double mergingPointX = numerator / denominator;

  if (mergingPointX >= 0 && mergingPointX <= 1) {
    cout << "Tapping point in range, calculating merging point X: "
         << mergingPointX << endl;
    cout << "length of wire: " << lengthOfWire << endl;
    cout << "merge point distance for sink 1 :"
         << ceil(mergingPointX * lengthOfWire) << endl;
    cout << "merge point distance for sink 2 :"
         << floor((1 - mergingPointX) * lengthOfWire) << endl;
    vector<Point> points1 =
        findPoints(x1, y1, ceil(mergingPointX * lengthOfWire));
    vector<Point> points2 =
        findPoints(x2, y2, floor((1 - mergingPointX) * lengthOfWire));
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
      //if (point.x >= 0 && point.y >= 0 && point.x < layout.width && point.y < layout.height) {
      //if (point.x == x1 && point.x >= 0 && point.y >= 0 && point.x < layout.width && point.y < layout.height) {
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
      //if (point.x >= 0 && point.y >= 0 && point.x <= layout.width && point.y <= layout.height) {
      //if (point.x == x2 && point.x >= 0 && point.y >= 0 && point.x <= layout.width && point.y <= layout.height) {
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
    solutions.insert(solutions.end(), solutions2.begin(), solutions2.end());
    solutions = filterPoints(solutions, 1.5);
    vector<Point> mergingPoints = linear_planar_dme(solutions, clk_location);

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
      ancestors[i]->x = roundedPoint.x;
      ancestors[i]->y = roundedPoint.y;
      ancestors[i]->z = node1->z; // Assign same z tier as parents
      ancestors[i]->node_type = "Merging Point";
      cout << "Merged At: (" << ancestors[i]->x << ", " << ancestors[i]->y
           << ") for Node ID: " << ancestors[i]->id << endl;
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
      double totalLength = originalLength + extension;

      // Check if buffer is needed (1.5x threshold)
      if (totalLength > (originalLength * 1.5)) {
          cout << "Adding buffer at merging point due to extended length: " 
               << totalLength << " (original: " << originalLength << ")" << endl;

          // Update parent node to include buffer
          parent->isBuffered = true;
          parent->resistance = bufferUnits.outputResistance;
          parent->capacitance = bufferUnits.inputCapacitance;

          // Add buffer delay to the path
          parent->elmoreDelay += bufferUnits.intrinsicDelay;

          // Recalculate delays considering buffer

          delaySegment1 = getNodeDelay(root, parent->id);
          delaySegment2 = getNodeDelay(root, parent->id);
          extension=0;
          lPrime=originalLength;
          cout << "Buffer inserted, length will not be extended" << endl;
      } 

      cout << "lPrime rounded = " << extension << endl;
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
        if (point.x == x2 && point.x >= 0 && point.y >= 0 && point.x <= layout.width && point.y <= layout.height) {
        //if (point.x == x2 && point.x >= 0 && point.y >= 0) {
          solutions.push_back(point);
        }
      }
      for (const Point &point : solutions) {
        cout << "(" << point.x << ", " << point.y << ")\n";
      }

      // Call linear_planar_dme and get all merging points
      vector<Point> mergingPoints = linear_planar_dme(solutions, clk_location);

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
        ancestors[i]->x = roundedPoint.x;
        ancestors[i]->y = roundedPoint.y;
        ancestors[i]->z = node1->z; // Assign same z tier as parents
        ancestors[i]->node_type = "Merging Point";
        cout << "Merged At: (" << ancestors[i]->x << ", " << ancestors[i]->y
             << ") for Node ID: " << ancestors[i]->id << endl;
      }

    } else {
      // vector<Point> solutions;
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
      //cout<<"grape"<<round(lPrime)<<endl;
      extension = round(lPrime);
      //cout<<"pineappe"<<extension<<endl;
      double totalLength = originalLength + extension;

      // Check if buffer is needed (1.5x threshold)
      if (totalLength > (originalLength * 1.5)) {
          cout << "Adding buffer at merging point due to extended length: " 
               << totalLength << " (original: " << originalLength << ")" << endl;

          // Update parent node to include buffer
          parent->isBuffered = true;
          parent->resistance = bufferUnits.outputResistance;
          parent->capacitance = bufferUnits.inputCapacitance;

          // Add buffer delay to the path
          parent->elmoreDelay += bufferUnits.intrinsicDelay;

          // Recalculate delays considering buffer
          
          delaySegment1 = getNodeDelay(root, parent->id);
          delaySegment2 = getNodeDelay(root, parent->id);
          extension=0;
          lPrime=originalLength;
          cout << "Buffer inserted, length will not be extended" << endl;
      } 
      cout << "lPrime rounded = " << extension << endl;
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
        if (point.x == x1 && point.x >= 0 && point.y >= 0 && point.x <= layout.width && point.y <= layout.height) {
        //if (point.x == x1 && point.x >= 0 && point.y >= 0) {
          solutions.push_back(point);
        }
      }
      for (const Point &point : solutions) {
        cout << "(" << point.x << ", " << point.y << ")\n";
      }

      // Call linear_planar_dme and get the merging point
      vector<Point> mergingPoints = linear_planar_dme(solutions, clk_location);

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
        ancestors[i]->x = roundedPoint.x;
        ancestors[i]->y = roundedPoint.y;
        ancestors[i]->z = node1->z; // Assign same z tier as parents
        ancestors[i]->node_type = "Merging Point";
        cout << "Merged At: (" << ancestors[i]->x << ", " << ancestors[i]->y
             << ") for Node ID: " << ancestors[i]->id << endl;
      }
    }
    cout << endl;
    return lPrime;
  }
}

// Main recursive function to perform zero skew merging
Node *zeroSkewTree(Node *root, const Point &clk_location = {-1, -1}) { //OLD
  cout << "clock location ZST: " << clk_location.x << "," << clk_location.y
       << endl;
  if (!root || (root->leftChild == nullptr && root->rightChild == nullptr)) {
    return root;
  }

  root->leftChild = zeroSkewTree(root->leftChild, clk_location);
  root->rightChild = zeroSkewTree(root->rightChild, clk_location);

  if (hasPhysicalLocation(root->leftChild) &&
      hasPhysicalLocation(root->rightChild)) {
    ZeroSkewMerge(root, root->leftChild->id, root->rightChild->id,
                  clk_location);
    ZeroSkewMerges++;
    cout << "Merged At: (" << root->x << ", " << root->y << ")" << endl;
    cout << "ZeroSkewMerges: " << ZeroSkewMerges << "\n" << endl;
  }
  return root;
} 
// Helper function to collect all sinks from the tree into a vector
void collectSinks(Node *node, std::vector<Sink> &sinkVector) {
  if (node == nullptr) {
    return; // Base case: if the node is null, return.
  }

  // Add sinks from the current node to the vector
  sinkVector.insert(sinkVector.end(), node->sinks.begin(), node->sinks.end());

  // Recursively collect sinks from left and right children
  collectSinks(node->leftChild, sinkVector);
  collectSinks(node->rightChild, sinkVector);
}
std::vector<Sink> treeToSinkVector(Node *root) {
  std::vector<Sink> sinkVector;
  collectSinks(root, sinkVector);
  return sinkVector;
}


// Function to parse input from a file
void parseInput(const string &filename) {
  ifstream inputFile(filename);
  if (!inputFile.is_open()) {
    cerr << "Error opening file!" << endl;
    exit(1); // Exit if file cannot be opened
  }
  // Parse input file
  inputFile >> layout.width >> layout.height >> layout.numDies;
  inputFile.ignore(numeric_limits<streamsize>::max(),
                   '\n'); // Ignore the rest of the line
  inputFile >> wireUnits.resistance >> wireUnits.capacitance;
  inputFile.ignore(numeric_limits<streamsize>::max(), '\n');
  inputFile >> bufferUnits.outputResistance >> bufferUnits.inputCapacitance >>
      bufferUnits.intrinsicDelay;
  inputFile.ignore(numeric_limits<streamsize>::max(), '\n');
  inputFile >> tsvUnits.resistance >> tsvUnits.capacitance;
  inputFile.ignore(numeric_limits<streamsize>::max(), '\n');
  inputFile >> clockSource.x >> clockSource.y >> clockSource.z >>
      clockSource.outputResistance;
  inputFile.ignore(numeric_limits<streamsize>::max(), '\n');
  inputFile >> numSinks;
  inputFile.ignore(numeric_limits<streamsize>::max(), '\n');
  for (int i = 0; i < numSinks; ++i) {
    Sink sink;
    inputFile >> sink.x >> sink.y >> sink.z >> sink.inputCapacitance;
    sinks.push_back(sink);
    inputFile.ignore(numeric_limits<streamsize>::max(), '\n');
  }
  inputFile.close();
}

void displayParsedData() {
  cout << "Layout Area: (" << layout.width << "," << layout.height
       << ")(x,y)(um)" << endl;
  cout << "Number Of Dies: " << layout.numDies << endl;
  cout << "Unit Wire Resistance: " << wireUnits.resistance << "(ohm/um)"
       << endl;
  cout << "Unit Wire Capacitance: " << wireUnits.capacitance << "(fF/um)"
       << endl;
  cout << "Buffer Output Resistance(ohm): " << bufferUnits.outputResistance
       << endl;
  cout << "Buffer Input Capacitance(fF): " << bufferUnits.inputCapacitance
       << endl;
  cout << "Buffer Intrinsic Delay(ps): " << bufferUnits.intrinsicDelay << endl;
  cout << "TSV Resistance(ohm): " << tsvUnits.resistance << endl;
  cout << "TSV Capacitance(fF): " << tsvUnits.capacitance << endl;
  cout << "Clock Source: (" << clockSource.x << "," << clockSource.y << ","
       << clockSource.z << ")(x,y,z) " << endl;
  cout << "Clock Output resistance(ohm): " << clockSource.outputResistance
       << endl;
  cout << endl;
  cout << "Sinks:" << endl;
  for (const auto &sink : sinks) {
    cout << "(" << sink.x << "," << sink.y << "," << sink.z
         << ")(x,y,z), Input Capacitance - " << sink.inputCapacitance << " fF"
         << endl;
  }
  cout << endl;
  cout << "Median of x coordinates: " << calculateMedianX(sinks) << endl;
  cout << "Median of y coordinates: " << calculateMedianY(sinks) << endl;
}

void exportNode(Node *node, std::ofstream &file) {
  if (!node) {
    return;
  }
  // Check if the node is a leaf node or MIV node
  bool isLeaf = node->leftChild == nullptr && node->rightChild == nullptr;
  bool isMIV = node->node_type == "MIV";
  // Export the current node with special notation for leaf nodes and MIVs
  if (isLeaf) {
    file << "P " << node->x << " " << node->y << " (Leaf Node)" << std::endl;
  } else if (isMIV) {
    file << "P " << node->x << " " << node->y << " (MIV Node)" << std::endl;
  } else {
    file << "P " << node->x << " " << node->y << std::endl;
  }
  // Export the lines to children
  if (node->leftChild) {
    file << "L " << node->x << " " << node->y << " " << node->leftChild->x
         << " " << node->leftChild->y << std::endl;
    exportNode(node->leftChild, file);
  }
  if (node->rightChild) {
    file << "L " << node->x << " " << node->y << " " << node->rightChild->x
         << " " << node->rightChild->y << std::endl;
    exportNode(node->rightChild, file);
  }
}

void exportPointsAndLines(Node *root, const std::string &filename) {
  // Open file in append mode
  std::ofstream file(filename, std::ios::app);
  if (!file.is_open()) {
    std::cerr << "Error opening file for writing: " << filename << std::endl;
    return;
  }

  // Add separator between subtrees
  file << "\n# New Subtree\n";
  exportNode(root, file);
  file.close();
}
Node *createClockSourceNode() {
  Node *clockNode = new Node({}, "ClockSource", 0.0, 0.0, false, nodeID++);
  clockNode->x = clockSource.x;
  clockNode->y = clockSource.y;
  clockNode->z = clockSource.z;
  return clockNode;
}

// Function to calculate the Manhattan distance between two sinks
int manhattanDistance(const Sink &a, const Sink &b) {
  return abs(a.x - b.x) + abs(a.y - b.y);
}

// Function to find the minimum spanning tree using Prim's algorithm
int calculateWirelength(const vector<Sink> &sinks) {
  int n = sinks.size();
  if (n == 0)
    return 0;

  vector<bool> inMST(n, false);
  vector<int> minDist(n, numeric_limits<int>::max());
  minDist[0] = 0; // Start from the first sink

  int totalWirelength = 0;

  for (int i = 0; i < n; ++i) {
    // Find the sink with the minimum distance that is not yet included in MST
    int minDistance = numeric_limits<int>::max();
    int u = -1;
    for (int j = 0; j < n; ++j) {
      if (!inMST[j] && minDist[j] < minDistance) {
        minDistance = minDist[j];
        u = j;
      }
    }

    // Add the selected sink to MST
    inMST[u] = true;
    totalWirelength += minDistance;

    // Update the distances for adjacent sinks
    for (int v = 0; v < n; ++v) {
      if (!inMST[v]) {
        int dist = manhattanDistance(sinks[u], sinks[v]);
        if (dist < minDist[v]) {
          minDist[v] = dist;
        }
      }
    }
  }
  return totalWirelength;
}
int calculateZeroSkewTreeWirelength(Node *root) {
  if (!root)
    return 0.0;

  int wirelength = 0.0;
  if (root->leftChild) {
    wirelength +=
        abs(root->x - root->leftChild->x) + abs(root->y - root->leftChild->y);
    wirelength += calculateZeroSkewTreeWirelength(root->leftChild);
  }
  if (root->rightChild) {
    wirelength +=
        abs(root->x - root->rightChild->x) + abs(root->y - root->rightChild->y);
    wirelength += calculateZeroSkewTreeWirelength(root->rightChild);
  }
  return wirelength;
}

int main() {
  
  cleanupPreviousFiles();
  auto start = std::chrono::high_resolution_clock::now();
  // Create log file with timestamp
  auto now = std::chrono::system_clock::now();
  auto time_t_now = std::chrono::system_clock::to_time_t(now);
  std::stringstream timestamp;
  timestamp << std::put_time(std::localtime(&time_t_now), "%Y%m%d_%H%M%S");
  std::ofstream logFile("log_" + timestamp.str() + ".txt");
  if (!logFile.is_open()) {
    std::cerr << "Error opening log file!" << std::endl;
    return 1;
  }

  // Redirect cout to the log file
  std::streambuf *coutBuf = std::cout.rdbuf();
  std::cout.rdbuf(logFile.rdbuf());

  int bound = 10;
  int idealSum = 0;
  int zsmSum = 0;
  int subtreeTotalSum = 0;
  parseInput("benchmark10.txt");
  displayParsedData();
  //Separate sinks by their z-coordinate
  map<int, vector<Sink>> sinksByZ;
  for (const auto &sink : sinks) {
    sinksByZ[sink.z].push_back(sink);
  }

  for (const auto &pair : sinksByZ) {
    int z = pair.first;
    const auto &sinksGroup = pair.second;
    int tierZsmSum = 0;
    // For each z-coordinate, generate the tree and then perform zero skew tree
    Node *root = AbsTreeGen3D(sinksGroup, bound);
    int wireLength = calculateWirelength(sinksGroup);
    root->z = z; // Assign the z coordinate of the sink group to the root node
    cout << "\nProcessing z-coordinate: " << z << endl;
    // Create the tier-specific filename
    string tierFilename =
        "zeroskew_points_and_lines_z_" + to_string(z) + ".txt";
    assignPhysicalLocations(root);
    //depthFirstCapacitance(root);
    double eps = layout.width * .085; // Epsilon distance
    int minPts = 1;                    // Minimum points to form a cluster
    runDBSCANAndAssignClusters(root, eps, minPts);
    clusterMidpoints = calculateClusterMidpoints(root);
    //  Create subtrees for each cluster
    std::vector<Node *> clusterRoots = createClusterSubtrees(root);
    printNodesByClusterId(root);
    calculateClusterMidpoints(root);
    // Print each subtree using printTree function
    for (Node *subtreeRoot : clusterRoots) {
      std::cout << "Printing subtree with Cluster ID: "
                << subtreeRoot->cluster_id << std::endl;
      Point currMidpoint = getMidpointByClusterId(subtreeRoot->cluster_id);
      std::cout << "Cluster " << subtreeRoot->cluster_id << " midpoint ("
                << currMidpoint.x << "," << currMidpoint.y << ")" << endl;
      std::vector<Sink> subtreeSinks = treeToSinkVector(subtreeRoot);
      Node *AbstractSubtree = AbsTreeGen3D(subtreeSinks, bound);
      std::cout << "***********************************" << std::endl;
      assignPhysicalLocations(AbstractSubtree);
      assignClusterIdToTree(AbstractSubtree, subtreeRoot->cluster_id);
      Node *zeroSkewSubtree = zeroSkewTree(AbstractSubtree);
      zeroSkewSubtree->node_type = "MIV";
      // depthFirstCapacitance(zeroSkewSubtree);
      // depthFirstDelay(zeroSkewSubtree, tsvUnits.resistance);
      printTree(zeroSkewSubtree);

      int subtreeZsmWireLength =
          calculateZeroSkewTreeWirelength(AbstractSubtree);
      cout << "~~~Zero Skew Tree Wirelength for cluster "
           << subtreeRoot->cluster_id << ": " << subtreeZsmWireLength << endl;
      std::cout << "---------------------------------" << std::endl;
      tierZsmSum += subtreeZsmWireLength;
      cout << "subtreeZsmSum for z " << z << "=" << tierZsmSum << endl;

      // Export each subtree to the tier-specific file in append mode
      exportPointsAndLines(zeroSkewSubtree, tierFilename);
      cout << "Exported subtree for cluster " << subtreeRoot->cluster_id
           << " to tier " << z << " file" << endl;
      deleteTree(zeroSkewSubtree);  // Clean up after exporting
    }

    cout << "Completed exporting all subtrees for tier " << z << " to "
         << tierFilename << endl;

    int zsmWireLength = calculateZeroSkewTreeWirelength(root);

    idealSum += wireLength;
    zsmSum += zsmWireLength;
    subtreeTotalSum += tierZsmSum;
    cout << "!!! Wirelength for z = " << z << ": " << wireLength << endl;
    cout << "~~~Zero Skew Tree Wirelength for z = " << z << ": "
         << zsmWireLength << endl;
    deleteTree(root);  // Clean up after exporting (Might not be right)
    //roots.push_back(zeroSkewTree(root)); // Apply zero skew tree operation
    // and store the root
  }

  auto end = std::chrono::high_resolution_clock::now();
  // Calculate the duration
  auto duration =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  // Cout to logfile
  cout << "Ideal Wirelength Sum = " << idealSum << endl;
  cout << "Zero Skew Tree Wirelength Sum = " << zsmSum << endl;
  cout << "Cluster ZST total = " << subtreeTotalSum << endl;
  std::cout << "Execution time: " << duration.count() << " microseconds"
            << std::endl;
  // Cout to console
  std::cout.rdbuf(coutBuf);
  cout << "Ideal Wirelength Sum = " << idealSum << endl;
  cout << "Zero Skew Tree Wirelength Sum = " << zsmSum << endl;
  cout << "Cluster ZST total = " << subtreeTotalSum << endl;
  std::cout << "Execution time: " << duration.count() << " microseconds"
            << std::endl;
  return 0;
}
