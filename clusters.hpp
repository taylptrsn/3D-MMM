
#pragma once
#include "structures.hpp"
#include "utilities.hpp"
using namespace std;

void collectNodesByClusterId(Node *node,
                             std::map<int, std::vector<Node *>> &clusters) {
  if (!node)
    return;

  // Add the node to the corresponding cluster
  clusters[node->cluster_id].push_back(node);

  // Recursively collect nodes from the left and right children
  collectNodesByClusterId(node->leftChild, clusters);
  collectNodesByClusterId(node->rightChild, clusters);
}

void printNodesByClusterId(Node *root) {
  std::map<int, std::vector<Node *>> clusters;
  collectNodesByClusterId(root, clusters);

  for (const auto &cluster : clusters) {
    int clusterId = cluster.first;
    const auto &nodes = cluster.second;

    std::cout << "Cluster ID: " << clusterId << std::endl;
    for (const auto &node : nodes) {
      std::cout << "Node ID: " << node->id << ", Position: (" << node->x << ", "
                << node->y << ", " << node->z << ")" << std::endl;
    }
    std::cout << std::endl;
  }
}

void collectNodesByClusterId(
    Node *node, std::unordered_map<int, std::vector<Node *>> &clusters) {
  if (!node)
    return;
  clusters[node->cluster_id].push_back(node);
  collectNodesByClusterId(node->leftChild, clusters);
  collectNodesByClusterId(node->rightChild, clusters);
}

// Modified function to calculate and store cluster midpoints
std::unordered_map<int, std::pair<double, double>>
calculateClusterMidpoints(Node *root) {
  std::unordered_map<int, std::vector<Node *>> clusters;
  collectNodesByClusterId(root, clusters);

  std::unordered_map<int, std::pair<double, double>> clusterMidpoints;

  for (const auto &cluster : clusters) {
    int clusterId = cluster.first;
    const auto &nodes = cluster.second;

    double sumX = 0.0;
    double sumY = 0.0;
    int count = 0;

    for (const auto &node : nodes) {
      if (node->leftChild == nullptr &&
          node->rightChild == nullptr) { // Only consider leaf nodes (sinks)
        sumX += node->x;
        sumY += node->y;
        count++;
      }
    }

    if (count > 0) {
      double midpointX = sumX / count;
      double midpointY = sumY / count;
      std::cout << "Cluster ID: " << clusterId << ", Midpoint: ("
                << ceil(midpointX) << ", " << floor(midpointY) << ")"
                << std::endl;
      // Store the midpoint in the map
      clusterMidpoints[clusterId] = {ceil(midpointX), floor(midpointY)};
    }
  }

  return clusterMidpoints; // Return the map of midpoints
}
Node *createClusterSubtree(const std::vector<Node *> &clusterNodes) {
  if (clusterNodes.empty()) {
    return nullptr;
  }

  // Create a new root node for this cluster
  Node *clusterRoot = new Node({}, "ClusterRoot", 0.0, 0.0, false, nodeID++);
  clusterRoot->cluster_id = clusterNodes[0]->cluster_id;

  // Calculate the average position for the cluster root
  double sumX = 0, sumY = 0;
  for (const auto &node : clusterNodes) {
    sumX += node->x;
    sumY += node->y;
  }
  clusterRoot->x = sumX / clusterNodes.size();
  clusterRoot->y = sumY / clusterNodes.size();
  clusterRoot->z =
      clusterNodes[0]
          ->z; // Assuming all nodes in a cluster are on the same tier

  // If there's only one node, make it the root
  if (clusterNodes.size() == 1) {
    return clusterNodes[0];
  }

  // Recursively build the subtree
  std::vector<Node *> leftNodes, rightNodes;
  for (size_t i = 0; i < clusterNodes.size(); ++i) {
    if (i < clusterNodes.size() / 2) {
      leftNodes.push_back(clusterNodes[i]);
    } else {
      rightNodes.push_back(clusterNodes[i]);
    }
  }

  clusterRoot->leftChild = createClusterSubtree(leftNodes);
  clusterRoot->rightChild = createClusterSubtree(rightNodes);

  return clusterRoot;
}

std::vector<Node *> createClusterSubtrees(Node *root) {
  std::map<int, std::vector<Node *>> clusters;
  collectNodesByClusterId(root, clusters);

  std::vector<Node *> clusterRoots;
  for (const auto &cluster : clusters) {
    if (cluster.first != -1) { // Ignore noise points (cluster ID -1)
      Node *clusterRoot = createClusterSubtree(cluster.second);
      clusterRoots.push_back(clusterRoot);
    }
  }

  return clusterRoots;
}

void assignClusterIdToTree(Node *node, int clusterId) {
  if (!node)
    return; // Base case: if node is nullptr, return
  // Assign the cluster ID to the current node
  node->cluster_id = clusterId;
  // Recursively assign the cluster ID to the left and right subtrees
  assignClusterIdToTree(node->leftChild, clusterId);
  assignClusterIdToTree(node->rightChild, clusterId);
}
// Function to print a specific midpoint by cluster ID
void printMidpointById(
    const std::unordered_map<int, std::pair<double, double>> &midpoints,
    int clusterId) {
  try {
    const auto &midpoint = midpoints.at(clusterId);
    std::cout << "~Cluster ID: " << clusterId << ", Midpoint: ("
              << midpoint.first << ", " << midpoint.second << ")" << std::endl;
  } catch (const std::out_of_range &) {
    std::cout << "Cluster ID " << clusterId << " not found." << std::endl;
  }
}
// Global variable to store midpoints
std::unordered_map<int, std::pair<double, double>> clusterMidpoints;
Point getMidpointByClusterId(int clusterId) {
  auto it = clusterMidpoints.find(clusterId);
  if (it != clusterMidpoints.end()) {
    return Point{it->second.first, it->second.second};
  }
  // Return a default Point or throw an exception if the cluster ID is not found
  return Point{-1, -1};
}

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
void dbscan(std::vector<Point> &points, double eps, int minPts) {
  int clusterId = 0;
  for (int i = 0; i < points.size(); ++i) {
    if (points[i].clusterId != 0) {
      continue;
    }
    std::vector<int> neighbors = regionQuery(points, i, eps);
    if (neighbors.size() < minPts) {
      points[i].clusterId = -1;
    } else {
      ++clusterId;
      expandCluster(points, i, neighbors, clusterId, eps, minPts);
    }
  }
}
void extractSinks(Node *node, std::vector<Sink> &sinks) {
  if (!node) {
    return;
  }
  sinks.insert(sinks.end(), node->sinks.begin(), node->sinks.end());
  extractSinks(node->leftChild, sinks);
  extractSinks(node->rightChild, sinks);
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

void runDBSCANAndAssignClusters(Node *root, double eps, int minPts) {
  std::vector<Sink> sinks;
  extractSinks(root, sinks);

  std::vector<Point> points;
  for (const auto &sink : sinks) {
    points.push_back(
        {static_cast<double>(sink.x), static_cast<double>(sink.y), 0});
  }

  dbscan(points, eps, minPts);

  // Assign cluster IDs back to leaf nodes
  assignClusterIdsToLeafNodes(root, points);
  outputDBSCANResults(points, root->z,
                      "dbscan_results_z_" + std::to_string(root->z) + ".csv");
} 