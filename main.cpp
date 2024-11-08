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

/* TODO
- Unit Conversion at Data read-in
- Split into header files
- Check delay calculations
- Find way to make sure MIV delay/cap from previous tier is not overwritte/is assigned correctly. 
*/
using namespace std;

// Structure Definitions
struct Layout {
  double width;  // Unit: micrometers (um)
  double height; // Unit: micrometers (um)
  int numDies;   // Unit: None, simply a count
};

struct WireUnits {
  double resistance;  // Unit: ohms per micrometer (ohm/um)
  double capacitance; // Unit: femtofarads per micrometer (fF/um)
};

struct BufferUnits {
  double outputResistance; // Unit: ohms (ohm)
  double inputCapacitance; // Unit: femtofarads (fF)
  double intrinsicDelay;   // Unit: picoseconds (ps)
};

struct TSVUnits {
  double resistance;  // Unit: ohms (ohm)
  double capacitance; // Unit: femtofarads (fF)
};

struct ClockSource {
  int x, y, z;             // Unit: None, coordinates in an unspecified grid
  double outputResistance; // Unit: ohms (ohm)
};

/*struct Point {
  double x, y;
};*/
struct Point {
  double x, y;
  int clusterId; // -1 for noise, 0 for unvisited, >0 for cluster IDs
};

struct Sink {
  int x, y, z;             // Unit: None, coordinates in an unspecified grid
  double inputCapacitance; // Unit: femtofarads (fF)
  double capacitance; // Unit: femtofarads (fF)
  string color;            // Unit: None, simply a descriptive string
  double delay; // Unit: picoseconds (ps)
  int cluster_id; 
  string sink_type;
  // Constructor with default sink color set to gray/uncolored
  Sink(int x = 0, int y = 0, int z = 0, double inputCapacitance = 0, double capacitance = 0,
       string color = "Gray",double delay=0,int cluster_id=-1,string sink_type="sink")
      : x(x), y(y), z(z), inputCapacitance(inputCapacitance), capacitance(capacitance), color(color),delay(delay),cluster_id(cluster_id),sink_type(sink_type) {}
}; 

/*struct Node {
  int id;             // Added id field to store unique identifier for each node
  vector<Sink> sinks; // Contains multiple sinks, each with their own units
  Node *leftChild;
  Node *rightChild;
  string color;
  int dieIndex;       // None, simply a count
  double capacitance; // femtofarads (fF)
  double resistance;  // ohms (ohm), added attribute for resistance from root to
                      // this node
  double elmoreDelay; // picoseconds (ps), existing attribute
  bool isBuffered; // Indicates if the node is a buffered node, default is false
  int x, y, z;     // Unit: None, coordinates in an unspecified grid
  string node_type; // New attribute to store the type of node
  int cluster_id;   // New attribute to store the cluster ID of the node

  Node(const vector<Sink> &sinks, string color = "Gray",
       double capacitance = 0.0, double resistance = 0.0,
       bool isBuffered = false, int id = 0, int x = -1, int y = -1, int z = -1,
       string node_type = "undefined",
       int cluster_id = -1) // Initialize node_type to "undefined"
      : id(id), sinks(sinks), leftChild(nullptr), rightChild(nullptr),
        color(color), capacitance(capacitance), resistance(resistance),
        isBuffered(isBuffered), x(x), y(y), z(z), node_type(node_type),
        cluster_id(cluster_id) {}
}; */
struct Node {
  int id;             
  vector<Sink> sinks; 
  Node *leftChild;
  Node *rightChild;
  string color;
  int dieIndex;       
  double capacitance; 
  double resistance;  
  double elmoreDelay; 
  bool isBuffered;    
  int x, y, z;     
  string node_type; 
  int cluster_id;   
  double bufferDelay; 
  Node(const vector<Sink> &sinks, string color = "Gray",
       double capacitance = 0.0, double resistance = 0.0,
       bool isBuffered = false, int id = 0, int x = -1, int y = -1, int z = -1,
       string node_type = "undefined",
       int cluster_id = -1,
       double bufferDelay = 0.0) 
      : id(id), sinks(sinks), leftChild(nullptr), rightChild(nullptr),
        color(color), capacitance(capacitance), resistance(resistance),
        isBuffered(isBuffered), x(x), y(y), z(z), node_type(node_type),
        cluster_id(cluster_id), bufferDelay(bufferDelay) {}
};
// Global Variables
Layout layout;
WireUnits wireUnits;
BufferUnits bufferUnits;
TSVUnits tsvUnits;
ClockSource clockSource;
int numSinks;
int nodeID = 0;         // Global variable to keep track of the next node ID
int zCutCount = 0;      // Keeps track of the number of Z-cuts performed
int ZeroSkewMerges = 0; // Keeps track of the number of ZSMs performed
vector<Sink> sinks;
vector<string> dieColors = {"Gray", "Red",    "Green",
                            "Blue", "Purple", "Lime"}; // Define colors for dies

// Utility Functions
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

vector<Point> findPoints(double x1, double y1, double mDist) {
  return {
      {x1 + mDist, y1}, // Right
      {x1 - mDist, y1}, // Left
      {x1, y1 + mDist}, // Up
      {x1, y1 - mDist}  // Down
  };
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
  // cout << "B " << B << endl;
  int B1 = 0;
  int B2 = 0;
  int deltaX = getMaxX(S) - getMinX(S);
  int deltaY = getMaxY(S) - getMinY(S);
  int deltaZ = getMaxZ(S) - getMinZ(S);
  vector<Sink> St, Sb;
  ClockSource Zs = clockSource;
  if (S.size() == 1) {
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
    Zcut(S, Zs, St, Sb);
  } else if (deltaZ >= 1 && B == 1) {
    Zcut(S, Zs, St, Sb);
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
    int medianX = calculateMedianX(S);
    int medianY = calculateMedianY(S);
    for (const auto &sink : S) {
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

bool isCoordinateUsedByLeaf(Node* root, int x, int y) {
  if (!root) return false;

  // If this is a leaf node (sink), check coordinates
  if (!root->leftChild && !root->rightChild) {
      if (root->x == x && root->y == y) return true;
  }

  // Recursively check children
  return isCoordinateUsedByLeaf(root->leftChild, x, y) || 
         isCoordinateUsedByLeaf(root->rightChild, x, y);
}
Point findNearestFreePoint(Node* root, Point original, int maxDistance = 5) {
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

std::vector<Point> linear_planar_dme(std::vector<Point>& sinks, const Point& clk_location = {-1, -1} ) {
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

double euclideanDistance(const Point &a, const Point &b) {
  return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
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
double ZeroSkewMerge(Node *root, int id1, int id2) {
  // Use global variables for resistance and capacitance per unit length
  double resistancePerUnitLength = wireUnits.resistance;
  double capacitancePerUnitLength = wireUnits.capacitance;
  Node *parent = findLCA(root, id1, id2);
  cout << "Parent: " << parent->id << endl;
  Node *node1 = findNodeById(root, id1);
  Node *node2 = findNodeById(root, id2);
  double delaySegment1 =
      getNodeDelay(root, parent->id) - getNodeDelay(root, id1);
  double delaySegment2 =
      getNodeDelay(root, parent->id) - getNodeDelay(root, id2);
  //cout << delaySegment1 << endl;
  //cout << delaySegment2 << endl;
  double capacitanceSegment1 = getNodeCapacitance(root, id1);
  cout << "CapSeg1: " << capacitanceSegment1 << endl;
  double capacitanceSegment2 = getNodeCapacitance(root, id2);
  cout << "CapSeg2: " << capacitanceSegment2 << endl;
  int lengthOfWire = calculateManhattanDistance(root, id1, id2);
  double delayDifference = abs(delaySegment1 - delaySegment2);

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

    //test
    std::vector<Point> solutions(solutions1.begin(), solutions1.end());
    // Append elements from the second vector to the new vector
    solutions = filterPoints(solutions, 1.5);
    solutions.insert(solutions.end(), solutions2.begin(),solutions2.end());
    // Call linear_planar_dme and get all merging points
    vector<Point> mergingPoints = linear_planar_dme(solutions);

    // Find the parent node and its ancestors
    Node* parent = findLCA(root, id1, id2);
    vector<Node*> ancestors;
    Node* current = parent;
    while (current != nullptr) {
        ancestors.push_back(current);
        current = findLCA(root, current->id, root->id);
        if (current == root) break;
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


      /*double totalLength = lengthOfWire + extension;
      // Check if buffer is needed (1.5x threshold)
      if (totalLength > (lengthOfWire * 1.5)) {
          cout << "Adding buffer at merging point due to extended length: " 
      << totalLength << " (original: " << lengthOfWire << ")" << endl;
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
          lPrime=lengthOfWire;
          cout << "Buffer inserted, length will not be extended" << endl;
      } */
      
      double totalLength = lengthOfWire + extension;
      // Check if buffer is needed (1.5x threshold)
      if (totalLength > (lengthOfWire * 1.5)) {
          cout << "Adding buffer at merging point due to extended length: " 
      << totalLength << " (original: " << lengthOfWire << ")" << endl;

          // Calculate required buffer delay to equalize delays
          double requiredBufferDelay = delayDifference;
          // Update parent node to include buffer
          //parent->isBuffered = true;
          parent->bufferDelay = requiredBufferDelay;
          parent->resistance = bufferUnits.outputResistance;
          //parent->capacitance = bufferUnits.inputCapacitance;
          // Add buffer delay to the path
          //parent->elmoreDelay += bufferUnits.intrinsicDelay;
          // Recalculate delays considering buffer
          //delaySegment1 = getNodeDelay(root, parent->id);
          //delaySegment2 = getNodeDelay(root, parent->id);
          extension=0;
          lPrime=lengthOfWire;
          cout << "Buffer with delay " << requiredBufferDelay <<"fs inserted at merging point, length will not be extended" << endl;
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
        if (point.x == x2 && point.x >= 0 && point.y >= 0 && point.x < layout.width && point.y < layout.height) {
        //if (point.x == x2 && point.x >= 0 && point.y >= 0) {
          solutions.push_back(point);
        }
      }
      for (const Point &point : solutions) {
        cout << "(" << point.x << ", " << point.y << ")\n";
      }
      // Call linear_planar_dme and get all merging points
      vector<Point> mergingPoints = linear_planar_dme(solutions);

      // Find the parent node and its ancestors
      Node* parent = findLCA(root, id1, id2);
      vector<Node*> ancestors;
      Node* current = parent;
      while (current != nullptr) {
          ancestors.push_back(current);
          current = findLCA(root, current->id, root->id);
          if (current == root) break;
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

      /*double totalLength = lengthOfWire + extension;
      // Check if buffer is needed (1.5x threshold)
      if (totalLength > (lengthOfWire * 1.5)) {
          cout << "Adding buffer at merging point due to extended length: " 
      << totalLength << " (original: " << lengthOfWire << ")" << endl;
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
          lPrime=lengthOfWire;
          cout << "Buffer inserted, length will not be extended" << endl;
      } */

      double totalLength = lengthOfWire + extension;
      // Check if buffer is needed (1.5x threshold)
      if (totalLength > (lengthOfWire * 1.5)) {
          cout << "Adding buffer at merging point due to extended length: " 
      << totalLength << " (original: " << lengthOfWire << ")" << endl;

          // Calculate required buffer delay to equalize delays
          double requiredBufferDelay = delayDifference;
          // Update parent node to include buffer
          //parent->isBuffered = true;
          parent->bufferDelay = requiredBufferDelay;
          parent->resistance = bufferUnits.outputResistance;
          //parent->capacitance = bufferUnits.inputCapacitance;
          // Add buffer delay to the path
          parent->elmoreDelay += bufferUnits.intrinsicDelay;
          // Recalculate delays considering buffer
          //delaySegment1 = getNodeDelay(root, parent->id);
          //delaySegment2 = getNodeDelay(root, parent->id);
          extension=0;
          lPrime=lengthOfWire;
          cout << "Buffer with delay " << requiredBufferDelay <<"fs inserted at merging point, length will not be extended" << endl;
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
        if (point.x == x1 && point.x >= 0 && point.y >= 0 && point.x < layout.width && point.y < layout.height) {
        //if (point.x == x1 && point.x >= 0 && point.y >= 0) {
          solutions.push_back(point);
        }
      }
      for (const Point &point : solutions) {
        cout << "(" << point.x << ", " << point.y << ")\n";
      }

      // Call linear_planar_dme and get all merging points
      vector<Point> mergingPoints = linear_planar_dme(solutions);

      // Find the parent node and its ancestors
      Node* parent = findLCA(root, id1, id2);
      vector<Node*> ancestors;
      Node* current = parent;
      while (current != nullptr) {
          ancestors.push_back(current);
          current = findLCA(root, current->id, root->id);
          if (current == root) break;
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
            points[i].clusterId = 0;  // Mark as unclustered instead of -1
        } else {
            ++clusterId;
            expandCluster(points, i, neighbors, clusterId, eps, minPts);
        }
    }
    
    // Count points in each cluster
    std::map<int, int> clusterSizes;
    for (const auto& point : points) {
        if (point.clusterId > 0) {
            clusterSizes[point.clusterId]++;
        }
    }
    
    // Sort clusters by size
    std::vector<std::pair<int, int>> sortedClusters;
    for (const auto& pair : clusterSizes) {
        sortedClusters.push_back({pair.second, pair.first}); // {size, clusterId}
    }
    std::sort(sortedClusters.begin(), sortedClusters.end(), 
              std::greater<std::pair<int, int>>()); // Sort by size in descending order
    
    // Keep track of clusters to keep
    std::set<int> keepClusters;
    for (int i = 0; i < std::min(bound, (int)sortedClusters.size()); ++i) {
        keepClusters.insert(sortedClusters[i].second);
    }
    
    // Second pass: Mark points in smaller clusters as cluster 0
    for (auto& point : points) {
        if (point.clusterId > 0 && keepClusters.find(point.clusterId) == keepClusters.end()) {
            point.clusterId = 0;  // Mark as cluster 0 instead of -1
        }
    }
    
    // Renumber remaining clusters from 1 to bound
    std::map<int, int> clusterRemap;
    int newClusterId = 1;
    for (const auto& cluster : keepClusters) {
        clusterRemap[cluster] = newClusterId++;
    }
    
    // Apply new cluster numbering, leaving cluster 0 points unchanged
    for (auto& point : points) {
        if (point.clusterId > 0) {
            point.clusterId = clusterRemap[point.clusterId];
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

void runDBSCANAndAssignClusters(Node *root, double eps, int minPts, int bound) {
    std::vector<Sink> sinks;
    extractSinks(root, sinks);

    std::vector<Point> points;
    for (const auto &sink : sinks) {
        points.push_back({static_cast<double>(sink.x), static_cast<double>(sink.y), 0});
    }

    dbscan(points, eps, minPts, bound);  // Pass the bound parameter

    assignClusterIdsToLeafNodes(root, points);
    outputDBSCANResults(points, root->z, "dbscan_results_z_" + std::to_string(root->z) + ".csv");
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

  
  void calculateBottomUpDelay(Node* node) {
    if (!node) return;

    // Process children first
    calculateBottomUpDelay(node->leftChild);
    calculateBottomUpDelay(node->rightChild);

    // For leaf nodes
    if (!node->leftChild && !node->rightChild) {
        // Start with clock source resistance for the first segment
        double totalResistance = clockSource.outputResistance + node->resistance;
        node->elmoreDelay = totalResistance * node->capacitance;

        if (node->isBuffered) {
            node->elmoreDelay += bufferUnits.intrinsicDelay;
        }
        return;
    }

    // For internal nodes
    double maxChildDelay = 0.0;

    // Calculate total downstream capacitance and find max child delay
    double downstreamCap = node->capacitance;
    if (node->leftChild) {
        maxChildDelay = max(maxChildDelay, node->leftChild->elmoreDelay);
        downstreamCap += node->leftChild->capacitance;
    }
    if (node->rightChild) {
        maxChildDelay = max(maxChildDelay, node->rightChild->elmoreDelay);
        downstreamCap += node->rightChild->capacitance;
    }

    // Calculate this node's delay contribution
    double nodeDelay = node->resistance * downstreamCap;

    // Total delay is max child delay plus this node's contribution
    node->elmoreDelay = maxChildDelay + nodeDelay;

    if (node->isBuffered) {
        node->elmoreDelay += bufferUnits.intrinsicDelay;
    }
  }

  void calculateBottomUpElmoreDelay(Node* node) {
    if (!node) return;
    depthFirstCapacitance(node);
    calculateBottomUpDelay(node);
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
  //int testbound = 10;
  int bound = 70; //Inserts Bound+1 MIVs per tier, Bound MIVs + 1 MIV for the unclustered sinks
  int idealSum = 0;
  int zsmSum = 0;
  int subtreeTotalSum = 0;
  parseInput("benchmark10.txt");
  displayParsedData();
  //Separate sinks by their z-coordinate
  map<int, vector<Sink>> sinksByZ;
  //std::map<int, std::vector<SubtreeInfo>> tierSubtreeInfo;
  std::map<int, std::vector<Sink>> tierMIVSinks;
  for (const auto &sink : sinks) {
    sinksByZ[sink.z].push_back(sink);
  }

  for (const auto &pair : sinksByZ) {
    int z = pair.first;
    //const auto &sinksGroup = pair.second;
    vector<Sink> sinksGroup = pair.second;
    int tierZsmSum = 0;

    // Add MIV sinks from lower tier (z-1) if they exist
    if (z > 1 && tierMIVSinks.find(z-1) != tierMIVSinks.end()) {
        const auto& mivSinks = tierMIVSinks[z-1];
        sinksGroup.insert(sinksGroup.end(), mivSinks.begin(), mivSinks.end());
    }
    
    // For each z-coordinate, generate the tree and then perform zero skew tree
    Node *root = AbsTreeGen3D(sinksGroup, bound);
    int wireLength = calculateWirelength(sinksGroup);
    root->z = z; // Assign the z coordinate of the sink group to the root node
    cout << "\nProcessing z-coordinate: " << z << endl;
    // Create the tier-specific filename
    string tierFilename =
        "zeroskew_points_and_lines_z_" + to_string(z) + ".txt";
    assignPhysicalLocations(root);
    //assignPhysicalCharacteristics(root); WIP
    //Baseline case, eps = layout.width, minPts=1 or numSinks
    double eps = layout.width * .085; // Epsilon distance
    int minPts = 4;                    // Minimum points to form a cluster
    runDBSCANAndAssignClusters(root, eps, minPts, bound);
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
      depthFirstCapacitance(AbstractSubtree);
      Node *zeroSkewSubtree = zeroSkewTree(AbstractSubtree);
      zeroSkewSubtree->node_type = "MIV";
      depthFirstCapacitance(zeroSkewSubtree);
      depthFirstDelay(zeroSkewSubtree, tsvUnits.resistance); //WIP
      //calculateBottomUpElmoreDelay(zeroSkewSubtree); WIP

      // Store the subtree information in map
      Sink rootSink;
      rootSink.x = zeroSkewSubtree->x;
      rootSink.y = zeroSkewSubtree->y;
      rootSink.z = z;  
      rootSink.delay = zeroSkewSubtree->elmoreDelay; 
      rootSink.cluster_id = zeroSkewSubtree->cluster_id; 
      rootSink.sink_type = "MIV"; 
      tierMIVSinks[z].push_back(rootSink);
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
  }
  // To print/access the information:
  for (const auto& pair : tierMIVSinks) {
      cout << "Tier " << pair.first << " subtree roots:" << endl;
      for (const auto& sink : pair.second) {
          cout<< "Sink type: " << sink.sink_type <<  "Cluster " << sink.cluster_id 
               << " at (" << sink.x << "," << sink.y 
               << ") with delay " << sink.delay << endl;
      }
  }
  auto end = std::chrono::high_resolution_clock::now();
  // Calculate runtime
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
