#pragma once
#include "structures.hpp"
#include "globals.hpp"
#include <iostream>     // for cout, cerr
#include <vector>       // for vector<>
#include <string>       // for string
#include <cmath>        // for abs, floor, ceil
#include <algorithm>    // for max_element, min_element
#include <limits>       // for numeric_limits
#include <fstream>      // for file operations
#include <map>
#include <set>
using namespace std;

// Utility Functions
// Function to calculate the Manhattan distance between two sinks
int manhattanDistance(const Sink &a, const Sink &b) {
  return abs(a.x - b.x) + abs(a.y - b.y);
}
double euclideanDistance(const Point &a, const Point &b) {
  return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
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



// Function to calculate the median of x coordinates
int calculateMedianX(const vector<Sink> &sinks) {
  vector<int> xCoordinates;
  xCoordinates.reserve(sinks.size()); // Pre-allocate for efficiency
  for (const auto &sink : sinks) {
    xCoordinates.push_back(sink.x);
  }
  sort(xCoordinates.begin(), xCoordinates.end());
  if (xCoordinates.size() % 2 == 0) {
    // Even number of elements, take the average of the middle two
    size_t middleIndex = xCoordinates.size() / 2;
    return (xCoordinates[middleIndex - 1] + xCoordinates[middleIndex]) / 2;
  } else {
    // Odd number of elements, take the middle element
    return xCoordinates[xCoordinates.size() / 2];
  }
}
// Function to calculate the median of y coordinates
int calculateMedianY(const vector<Sink> &sinks) {
  vector<int> yCoordinates;
  yCoordinates.reserve(sinks.size()); // Pre-allocate for efficiency
  for (const auto &sink : sinks) {
    yCoordinates.push_back(sink.y);
  }
  sort(yCoordinates.begin(), yCoordinates.end());
  if (yCoordinates.size() % 2 == 0) {
    // Even number of elements, take the average of the middle two
    size_t middleIndex = yCoordinates.size() / 2;
    return (yCoordinates[middleIndex - 1] + yCoordinates[middleIndex]) / 2;
  } else {
    // Odd number of elements, take the middle element
    return yCoordinates[yCoordinates.size() / 2];
  }
}

// Function to return the minimum x value in the set of sinks
int getMinX(const vector<Sink> &sinks) {
  if (sinks.empty()) {
    throw runtime_error("No sinks available to determine minimum X.");
  }
  int minX = sinks[0].x;
  for (const auto &sink : sinks) {
    if (sink.x < minX) {
      minX = sink.x;
    }
  }
  return minX;
}

// Function to return the maximum x value in the set of sinks
int getMaxX(const vector<Sink> &sinks) {
  if (sinks.empty()) {
    throw runtime_error("No sinks available to determine maximum X.");
  }
  int maxX = sinks[0].x;
  for (const auto &sink : sinks) {
    if (sink.x > maxX) {
      maxX = sink.x;
    }
  }
  return maxX;
}

// Function to return the minimum y value in the set of sinks
int getMinY(const vector<Sink> &sinks) {
  if (sinks.empty()) {
    throw runtime_error("No sinks available to determine minimum Y.");
  }
  int minY = sinks[0].y;
  for (const auto &sink : sinks) {
    if (sink.y < minY) {
      minY = sink.y;
    }
  }
  return minY;
}

// Function to return the maximum y value in the set of sinks
int getMaxY(const vector<Sink> &sinks) {
  if (sinks.empty()) {
    throw runtime_error("No sinks available to determine maximum Y.");
  }
  int maxY = sinks[0].y;
  for (const auto &sink : sinks) {
    if (sink.y > maxY) {
      maxY = sink.y;
    }
  }
  return maxY;
}

// Function to return the minimum z value in the set of sinks
int getMinZ(const vector<Sink> &sinks) {
  if (sinks.empty()) {
    throw runtime_error("No sinks available to determine minimum Z.");
  }
  int minZ = sinks[0].z;
  for (const auto &sink : sinks) {
    if (sink.z < minZ) {
      minZ = sink.z;
    }
  }
  return minZ;
}

// Function to return the maximum z value in the set of sinks
int getMaxZ(const vector<Sink> &sinks) {
  if (sinks.empty()) {
    throw runtime_error("No sinks available to determine maximum Z.");
  }
  int maxZ = sinks[0].z;
  for (const auto &sink : sinks) {
    if (sink.z > maxZ) {
      maxZ = sink.z;
    }
  }
  return maxZ;
}

void cleanupPreviousFiles() {
  // Delete DBSCAN results files
  for (int z = 1; z <= 10; z++) {
    string dbscan_file = "dbscan_results_z_" + to_string(z) + ".csv";
    string zeroskew_file =
        "zeroskew_points_and_lines_z_" + to_string(z) + ".txt";

    // Remove files if they exist
    if (remove(dbscan_file.c_str()) == 0) {
      cout << "Deleted previous " << dbscan_file << endl;
    }
    if (remove(zeroskew_file.c_str()) == 0) {
      cout << "Deleted previous " << zeroskew_file << endl;
    }
  }
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

bool isCoordinateUsedByLeaf(Node *root, int x, int y) {
  if (!root)
    return false;

  // If this is a leaf node (sink), check coordinates
  if (!root->leftChild && !root->rightChild) {
    if (root->x == x && root->y == y)
      return true;
  }

  // Recursively check children
  return isCoordinateUsedByLeaf(root->leftChild, x, y) ||
         isCoordinateUsedByLeaf(root->rightChild, x, y);
}
Point findNearestFreePoint(Node *root, Point original, int maxDistance = 5) {
  // If original point is free, return it
  if (!isCoordinateUsedByLeaf(root, original.x, original.y)) {
    return original;
  }

  // Search in expanding square pattern
  for (int d = 1; d <= maxDistance; d++) {
    // Check points in a square pattern around the original
    for (int dx = -d; dx <= d; dx++) {
      for (int dy = -d; dy <= d; dy++) {
        Point candidate = {original.x + dx, original.y + dy};
        if (!isCoordinateUsedByLeaf(root, candidate.x, candidate.y)) {
          return candidate;
        }
      }
    }
  }
  return original; // If no free point found, return original
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
