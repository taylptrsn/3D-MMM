#pragma once
#include "structures.hpp"
#include "utilities.hpp"
#include "globals.hpp"
#include <vector>
#include <unordered_map>
using namespace std;


Node *createClockSourceNode() {
  Node *clockNode = new Node({}, "ClockSource", 0.0, 0.0, false, nodeID++);
  clockNode->x = clockSource.x;
  clockNode->y = clockSource.y;
  clockNode->z = clockSource.z;
  return clockNode;
}

bool areCoordsNearlyIdentical(const Sink &s1, const Sink &s2,
                              int threshold = 1) {
  return abs(s1.x - s2.x) <= threshold && abs(s1.y - s2.y) <= threshold &&
         s1.z != s2.z; // Different layers
}
// Add this function to adjust coordinates
vector<Sink> adjustNearlyIdenticalCoords(vector<Sink> sinks) {
  for (size_t i = 0; i < sinks.size(); i++) {
    for (size_t j = i + 1; j < sinks.size(); j++) {
      if (areCoordsNearlyIdentical(sinks[i], sinks[j])) {
        // Offset the second sink by 1 unit in both x and y
        sinks[j].x += 1;
        sinks[j].y += 1;
      }
    }
  }
  return sinks;
}

void extractSinks(Node *node, std::vector<Sink> &sinks) {
  if (!node) {
    return;
  }
  sinks.insert(sinks.end(), node->sinks.begin(), node->sinks.end());
  extractSinks(node->leftChild, sinks);
  extractSinks(node->rightChild, sinks);
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
// Helper function to find a node by its ID
Node *findNodeById(Node *node, int id) {
  if (node == nullptr)
    return nullptr; // Base case: node is null
  if (node->id == id)
    return node;

  // Recursively search in the left subtree
  Node *leftResult = findNodeById(node->leftChild, id);
  if (leftResult != nullptr)
    return leftResult; // Found in the left subtree

  // Recursively search in the right subtree
  return findNodeById(node->rightChild,
                      id); // Could return nullptr if not found
}

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

void assignPhysicalCharacteristics(Node *node) {
  if (!node) {
    return;
  }
  if (node->leftChild == nullptr && node->rightChild == nullptr) {
    if (!node->sinks.empty()) {
      // Assign physical location from first sink
      node->x = node->sinks.front().x;
      node->y = node->sinks.front().y;
      node->z = node->sinks.front().z;
      node->node_type = "Leaf";

      node->capacitance = node->sinks.front().capacitance;
      // Assign delay from sink to node
      node->elmoreDelay = node->sinks.front().delay;
    }
  }
  // Recursive assignment for child nodes
  assignPhysicalCharacteristics(node->leftChild);
  assignPhysicalCharacteristics(node->rightChild);
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

// Core Logic

// Function to perform Z-cut
void Zcut(const vector<Sink> &S, const ClockSource &Zs, vector<Sink> &St,
          vector<Sink> &Sb) {
  cout << endl;
  cout << "Z-cut!" << endl;
  int Zmin = getMinZ(S); // Set your Zmin value
  int Zmax = getMaxZ(S); // Set your Zmax value
  //  If Zs is less than or equal to Zmin
  if (Zs.z <= Zmin) {
    cout << "Zs is less than or equal to Zmin" << endl;
    // if (!S.empty()) {
    for (const auto &sink : S) {
      if (sink.z == Zmin) {
        Sb.push_back(sink);
      } else {
        St.push_back(sink);
      }
    }
    //}
  }
  // If Zs is greater than or equal to Zmax
  else if (Zs.z >= Zmax) {
    cout << "If Zs is greater than or equal to Zmax" << endl;
    for (const auto &sink : S) {
      if (sink.z == Zmax) {
        St.push_back(sink);
      } else {
        Sb.push_back(sink);
      }
    }
    //}
  }
  // If Zs is between Zmin and Zmax
  else {
    cout << "If Zs is between Zmin and Zmax" << endl;
    for (const auto &sink : S) {
      if (sink.z >= Zs.z) {
        St.push_back(sink); // S top
      } else {
        Sb.push_back(sink); // S bottom
      }
    }
  }
  zCutCount++;
}

// Abstract Tree Generation
Node *AbsTreeGen3D(const vector<Sink> &S, int B) {
  // fixes edge case, where points are too close together to partition
  vector<Sink> adjustedSinks = adjustNearlyIdenticalCoords(S);
  int B1 = 0;
  int B2 = 0;
  // int deltaX = getMaxX(S) - getMinX(S);
  // int deltaY = getMaxY(S) - getMinY(S);
  // int deltaZ = getMaxZ(S) - getMinZ(S);
  // fixes edge case, where points are too close together to partition
  int deltaX = getMaxX(adjustedSinks) - getMinX(adjustedSinks);
  int deltaY = getMaxY(adjustedSinks) - getMinY(adjustedSinks);
  int deltaZ = getMaxZ(adjustedSinks) - getMinZ(adjustedSinks);
  vector<Sink> St, Sb;
  ClockSource Zs = clockSource;
  if (adjustedSinks.size() == 1) {
    // Base case: if die span = 1, 2d tree
    return new Node(S, "Gray", 0.0, 0.0, false,
                    nodeID++); // Assign a unique id to the node

    //} else if (deltaZ > 1 && B==1) {
  } else if (deltaX == 0 && deltaY == 0 && deltaZ >= 1) {
    // New condition for the edge case where all x and y are the same, but z
    // differs
    cout << "Special case: All x and y coordinates are the same. Performing "
            "Z-cut based on z coordinates."
         << endl;
    Zcut(adjustedSinks, Zs, St, Sb);
  } else if (deltaZ >= 1 && B == 1) {
    Zcut(adjustedSinks, Zs, St, Sb);
    //  Display St and Sb
    cout << "St (Top most die group):" << endl;
    for (const auto &sink : St) {
      cout << "(" << sink.x << "," << sink.y << "," << sink.z
           << ")(x,y,z), Input Capacitance - " << sink.inputCapacitance << " fF"
           << endl;
    }
    cout << "Sb (Bottom most die group):" << endl;
    for (const auto &sink : Sb) {
      cout << "(" << sink.x << "," << sink.y << "," << sink.z
           << ")(x,y,z), Input Capacitance - " << sink.inputCapacitance << " fF"
           << endl;
    }
    B1 = B2 = 1;

  } else {
    int medianX = calculateMedianX(adjustedSinks);
    int medianY = calculateMedianY(adjustedSinks);
    for (const auto &sink : adjustedSinks) {
      if (deltaX > deltaY) {
        if (sink.x < medianX) {
          St.push_back(sink); // top subset
        } else {
          Sb.push_back(sink); // bottom subset
        }
      } else {
        if (sink.y < medianY) {
          St.push_back(sink); // top subset
        } else {
          Sb.push_back(sink); // bottom subset
        }
      }
    }
    B1 = B / 2;
    B2 = B - B1;
  }
  Node *root = new Node(S, "Gray", 0.0, 0.0, false,
                        nodeID++); // Assign a unique id to the node

  root->leftChild = AbsTreeGen3D(St, B1);
  root->rightChild = AbsTreeGen3D(Sb, B2);
  return root;
}

// Function to delete the tree nodes recursively
void deleteTree(Node *node) {
  if (node) {
    deleteTree(node->leftChild);
    deleteTree(node->rightChild);
    delete node;
  }
}

void printTree(Node *node, int level = 0) {
  if (!node) {
    return; // Base case: if the node is null, return.
  }
  string indent = string(level * 8, ' '); // Adjust indentation for each level.
  // Print the left branch (child) first.
  if (node->leftChild) {
    printTree(node->leftChild, level + 1);
  }
  // Print the current node along with its color, memory address, capacitance,
  // and Elmore delay.
  cout << indent << node->node_type << " Node ID: " << node->id
       << " Cluster ID: " << node->cluster_id << " at position: (" << node->x
       << ", " << node->y << ", " << node->z << ") -  Depth " << level
       << " - Color: " << node->color
       << " - Total Capacitance: " << node->capacitance << " fF"
       << " - Elmore Delay: " << node->elmoreDelay << " fs" << endl;

  // If the node is a leaf, also print its sinks.
  if (!node->leftChild && !node->rightChild) {
    for (const auto &sink : node->sinks) {
      cout << indent << "    Sink: (" << sink.x << ", " << sink.y << ", "
           << sink.z << "), Input Capacitance: " << sink.inputCapacitance
           << " fF, Color: " << sink.color << endl;
    }
  }
  cout << endl;
  if (node->rightChild) {
    printTree(node->rightChild, level + 1);
  }
}

void printLeaves(const Node *node) {
  if (node == nullptr) {
    return;
  }
  if (node->leftChild == nullptr && node->rightChild == nullptr) {
    for (const auto &sink : node->sinks) {
      cout << "Color: " << sink.color
           << " - Sink: ("
              "Leaf Nodes(Sinks):  ("
           << sink.x << ", " << sink.y << ", " << sink.z << ")" << endl;
    }
  } else {
    printLeaves(node->leftChild);
    printLeaves(node->rightChild);
  }
}

void colorTree(Node *node, const vector<string> &dieColors,
               const ClockSource &source, int depth = 0) {
  if (node == nullptr) {
    return;
  }
  // Determine the color of the source based on its z-coordinate
  string sourceColor = dieColors[source.z % dieColors.size()];

  if (depth == 0) { // If the node is the root
    node->color = sourceColor;
  } else {
    int Zmin = getMinZ(node->sinks);
    int Zmax = getMaxZ(node->sinks);
    if (Zmin > source.z) {
      // If Zmin is greater than the z-coordinate of the source, set to the
      // color of Zmin
      node->color = dieColors[Zmin % dieColors.size()];
    } else if (Zmax < source.z) {
      // If Zmax is less than the z-coordinate of the source, set to the color
      // of Zmax
      node->color = dieColors[Zmax % dieColors.size()];
    } else {
      // Otherwise, set to the color of the source
      node->color = sourceColor;
    }
  }
  // Assign colors to the sinks within this node
  for (auto &sink : node->sinks) {
    sink.color = node->color;
  }
  colorTree(node->leftChild, dieColors, source, depth + 1);
  colorTree(node->rightChild, dieColors, source, depth + 1);
}

// Function to search for a node by its children's coordinates with verbose
// output
Node *findNodeByChildren(Node *root, int childX, int childY) {
  if (root == nullptr) {
    cout << "Reached a null node. Returning nullptr." << endl;
    return nullptr;
  }

  cout << "Visiting Node ID: " << root->id << " with coordinates (" << root->x
       << ", " << root->y << ", " << root->z << ")" << endl;

  // Check left child
  if (root->leftChild != nullptr) {
    cout << "Checking left child of Node ID: " << root->id << endl;
    if (root->leftChild->x == childX && root->leftChild->y == childY) {
      cout << "Match found in left child of Node ID: " << root->id << endl;
      return root;
    }
  } else {
    cout << "No left child for Node ID: " << root->id << endl;
  }

  // Check right child
  if (root->rightChild != nullptr) {
    cout << "Checking right child of Node ID: " << root->id << endl;
    if (root->rightChild->x == childX && root->rightChild->y == childY) {
      cout << "Match found in right child of Node ID: " << root->id << endl;
      return root;
    }
  } else {
    cout << "No right child for Node ID: " << root->id << endl;
  }

  // Recursively search in the left subtree
  cout << "Recursively searching left subtree of Node ID: " << root->id << endl;
  Node *foundNode = findNodeByChildren(root->leftChild, childX, childY);
  if (foundNode != nullptr) {
    return foundNode;
  }

  // Recursively search in the right subtree
  cout << "Recursively searching right subtree of Node ID: " << root->id
       << endl;
  return findNodeByChildren(root->rightChild, childX, childY);
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

// Function to convert a vector of nodes to a vector of sinks
std::vector<Sink> convertNodesToSinks(const std::vector<Node *> &nodes) {
  std::vector<Sink> allSinks;

  for (const auto &node : nodes) {
    // Add all sinks from the current node to the allSinks vector
    allSinks.insert(allSinks.end(), node->sinks.begin(), node->sinks.end());
  }

  return allSinks;
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