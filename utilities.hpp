#pragma once
#include "structures.hpp"
#include <vector>
using namespace std;
// Utility Functions
// Function to calculate the median of x coordinates
// Global Variables
//Layout layout;
//WireUnits wireUnits;
//BufferUnits bufferUnits;
//TSVUnits tsvUnits;
ClockSource clockSource;

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

vector<Point> findPoints(double x1, double y1, double mDist) {
  return {
      {x1 + mDist, y1}, // Right
      {x1 - mDist, y1}, // Left
      {x1, y1 + mDist}, // Up
      {x1, y1 - mDist}  // Down
  };
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

// Function to convert a vector of nodes to a vector of sinks
std::vector<Sink> convertNodesToSinks(const std::vector<Node *> &nodes) {
  std::vector<Sink> allSinks;

  for (const auto &node : nodes) {
    // Add all sinks from the current node to the allSinks vector
    allSinks.insert(allSinks.end(), node->sinks.begin(), node->sinks.end());
  }

  return allSinks;
}

double euclideanDistance(const Point &a, const Point &b) {
  return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}


