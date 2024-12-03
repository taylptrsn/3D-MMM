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


std::vector<int> regionQuery(const std::vector<Point> &points, int P,
                             double eps) {
  std::vector<int> neighbors;
  for (int i = 0; i < points.size(); ++i) {
    if (euclideanDistance(points[P], points[i]) <= eps) {
      neighbors.push_back(i);
    }
  }
  return neighbors;
}
void expandCluster(std::vector<Point> &points, int P,
                   std::vector<int> &neighbors, int clusterId, double eps,
                   int minPts) {
  points[P].clusterId = clusterId;
  for (size_t i = 0; i < neighbors.size(); ++i) {
    int neighborIndex = neighbors[i];
    if (points[neighborIndex].clusterId == -1) {
      points[neighborIndex].clusterId = clusterId;
    }
    if (points[neighborIndex].clusterId == 0) {
      points[neighborIndex].clusterId = clusterId;
      std::vector<int> newNeighbors = regionQuery(points, neighborIndex, eps);
      if (newNeighbors.size() >= minPts) {
        neighbors.insert(neighbors.end(), newNeighbors.begin(),
                         newNeighbors.end());
      }
    }
  }
}

void dbscan(std::vector<Point> &points, double eps, int minPts, int bound) {
  int clusterId = 0;

  // First pass: Regular DBSCAN to find all clusters
  for (int i = 0; i < points.size(); ++i) {
    if (points[i].clusterId != 0) {
      continue;
    }

    std::vector<int> neighbors = regionQuery(points, i, eps);
    if (neighbors.size() < minPts) {
      points[i].clusterId = 0; // Mark as unclustered instead of -1
    } else {
      ++clusterId;
      expandCluster(points, i, neighbors, clusterId, eps, minPts);
    }
  }

  // Count points in each cluster
  std::map<int, int> clusterSizes;
  for (const auto &point : points) {
    if (point.clusterId > 0) {
      clusterSizes[point.clusterId]++;
    }
  }

  // Sort clusters by size
  std::vector<std::pair<int, int>> sortedClusters;
  for (const auto &pair : clusterSizes) {
    sortedClusters.push_back({pair.second, pair.first}); // {size, clusterId}
  }
  std::sort(
      sortedClusters.begin(), sortedClusters.end(),
      std::greater<std::pair<int, int>>()); // Sort by size in descending order

  // Keep track of clusters to keep
  std::set<int> keepClusters;
  for (int i = 0; i < std::min(bound, (int)sortedClusters.size()); ++i) {
    keepClusters.insert(sortedClusters[i].second);
  }

  // Second pass: Mark points in smaller clusters as cluster 0
  for (auto &point : points) {
    if (point.clusterId > 0 &&
        keepClusters.find(point.clusterId) == keepClusters.end()) {
      point.clusterId = 0; // Mark as cluster 0 instead of -1
    }
  }

  // Renumber remaining clusters from 1 to bound
  std::map<int, int> clusterRemap;
  int newClusterId = 1;
  for (const auto &cluster : keepClusters) {
    clusterRemap[cluster] = newClusterId++;
  }

  // Apply new cluster numbering, leaving cluster 0 points unchanged
  for (auto &point : points) {
    if (point.clusterId > 0) {
      point.clusterId = clusterRemap[point.clusterId];
    }
  }
}


// Define a comparator for the Point structure to use in std::set
struct PointComparator {
  bool operator()(const Point &a, const Point &b) const {
    return std::tie(a.x, a.y, a.clusterId) < std::tie(b.x, b.y, b.clusterId);
  }
};
void outputDBSCANResults(const std::vector<Point> &points, int z,
                         const std::string &filename) {
  // Open the file in append mode
  std::ofstream file(filename, std::ios::app);
  if (!file.is_open()) {
    std::cerr << "Error opening file for writing: " << filename << std::endl;
    return;
  }

  // If the file is empty, write the header
  if (file.tellp() == 0) {
    file << "x,y,cluster,z" << std::endl;
  }

  // Use a set to track unique points
  std::set<Point, PointComparator> uniquePoints;

  // Insert points into the set to ensure uniqueness
  for (const auto &point : points) {
    uniquePoints.insert(point);
  }

  // Write unique points to the file
  for (const auto &point : uniquePoints) {
    if (point.clusterId > 0) { // Only include points that are part of a cluster
      file << point.x << "," << point.y << "," << point.clusterId << "," << z
           << std::endl;
    }
  }

  file.close();
}
void assignClusterIdsToLeafNodes(Node *node, const std::vector<Point> &points) {
  if (!node)
    return;

  if (node->leftChild == nullptr &&
      node->rightChild == nullptr) { // Check if it's a leaf node
    for (const auto &sink : node->sinks) {
      for (const auto &point : points) {
        if (sink.x == point.x && sink.y == point.y) {
          node->cluster_id = point.clusterId; // Assign cluster ID
          break;
        }
      }
    }
  }

  assignClusterIdsToLeafNodes(node->leftChild, points);
  assignClusterIdsToLeafNodes(node->rightChild, points);
}

void runDBSCANAndAssignClusters(Node *root, double eps, int minPts, int bound) {
  std::vector<Sink> sinks;
  extractSinks(root, sinks);

  std::vector<Point> points;
  for (const auto &sink : sinks) {
    points.push_back(
        {static_cast<double>(sink.x), static_cast<double>(sink.y), 0});
  }

  dbscan(points, eps, minPts, bound); // Pass the bound parameter

  assignClusterIdsToLeafNodes(root, points);
  outputDBSCANResults(points, root->z,
                      "dbscan_results_z_" + std::to_string(root->z) + ".csv");
}