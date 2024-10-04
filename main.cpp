#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <vector>
/* TODO
- Unit Conversion at Data read-in
- Split into header files
- Account for Vertical wirelength
- Delay buffering instead of wire elongation (if wire is elongated more
than 1.5x original length)
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

struct Point {
  double x, y;
};

struct Sink {
  int x, y, z;             // Unit: None, coordinates in an unspecified grid
  double inputCapacitance; // Unit: femtofarads (fF)
  string color;            // Unit: None, simply a descriptive string
  // Constructor with default sink color set to gray/uncolored
  Sink(int x = 0, int y = 0, int z = 0, double inputCapacitance = 0,
       string color = "Gray")
      : x(x), y(y), z(z), inputCapacitance(inputCapacitance), color(color) {}
};

struct Node {
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
  Node(const vector<Sink> &sinks, string color = "Gray",
       double capacitance = 0.0, double resistance = 0.0,
       bool isBuffered = false, int id = 0, int x = -1, int y = -1, int z = -1)
      : id(id), sinks(sinks), leftChild(nullptr), rightChild(nullptr),
        color(color), capacitance(capacitance), resistance(resistance),
        isBuffered(isBuffered), x(x), y(y), z(z) {}
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
  // cout<<"zmin:  " << Zmin << endl;
  // cout<<"zmax:  " << Zmax << endl;
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
    // if (!S.empty()) {
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
  cout << indent << "Node ID: " << node->id << " at position: (" << node->x
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

// Function to search for a node by its children's coordinates with verbose output
Node* findNodeByChildren(Node* root, int childX, int childY) {
    if (root == nullptr) {
        cout << "Reached a null node. Returning nullptr." << endl;
        return nullptr;
    }

    cout << "Visiting Node ID: " << root->id << " with coordinates (" << root->x << ", " << root->y << ", " << root->z << ")" << endl;

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
    Node* foundNode = findNodeByChildren(root->leftChild, childX, childY);
    if (foundNode != nullptr) {
        return foundNode;
    }

    // Recursively search in the right subtree
    cout << "Recursively searching right subtree of Node ID: " << root->id << endl;
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
  // depthFirstDelay(node, clockSource.outputResistance);
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

double distance(const Point& a, const Point& b) {
  return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

double radius(const std::vector<Point>& sinks) {
    double max_dist = 0;
    for (size_t i = 0; i < sinks.size(); ++i) {
        for (size_t j = i + 1; j < sinks.size(); ++j) {
            max_dist = std::max(max_dist, distance(sinks[i], sinks[j]));
        }
    }
    return max_dist / 2;
}

Point center(const std::vector<Point>& sinks, double r) {
    double x_sum = 0, y_sum = 0;
    for (const auto& sink : sinks) {
        x_sum += sink.x;
        y_sum += sink.y;
    }
    return {x_sum / sinks.size(), y_sum / sinks.size()};
}

void print_points(const std::string& label, const std::vector<Point>& points) {
    std::cout << label << ": ";
    for (const auto& point : points) {
        std::cout << "(" << point.x << ", " << point.y << ") ";
    }
    std::cout << std::endl;
}

void linear_planar_dme_sub(std::vector<Point>& S_prime, const Point& P_S_prime, std::vector<Point>& tree_points) {
    if (S_prime.size() == 1) return;
    //cout<<"DME SUB"<<endl;
    double r_prime = radius(S_prime);
    Point ms_v = center(S_prime, r_prime);
    // Add node v at ms_v to the tree
    tree_points.push_back(ms_v);
    // Divide S' into S1' and S2'
    std::vector<Point> S1_prime, S2_prime;

    bool identicalPoints = true;  // Flag to detect identical points condition
    // Detect if all points are identical
    Point firstSink = S_prime.front();
    for (const auto& sink : S_prime) {
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
    for (auto& sink : S_prime) {
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
      for (auto& sink : S_prime) {
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

std::vector<Point> linear_planar_dme(std::vector<Point>& sinks, const Point& clk_location = {-1, -1} ) {
    double r = radius(sinks);
    cout<<" "<<endl;
    // Build Manhattan Disks
    Point c_S = center(sinks, r);
    // Print the input sinks
    std::cout << "DME Input Sinks:" << std::endl;
    for (const auto& sink : sinks) {
        std::cout << "(" << sink.x << ", " << sink.y << ")" << std::endl;
    }
    cout<<" "<<endl;
    // Container for tree points
    std::vector<Point> tree_points;

    // Use clock location or center of sinks as starting point
    Point start_point = clk_location.x == -1 && clk_location.y == -1 ? c_S : clk_location;

    tree_points.push_back(start_point);

    // Recursive DME Sub
    linear_planar_dme_sub(sinks, c_S, tree_points);

    // Output cost: sum of edge lengths (can be implemented as needed)
    return tree_points;
}
//findNodeByChildren(root, 67, 30);
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
    solutions.insert(solutions.end(), solutions2.begin(),solutions2.end());
    
    // Print the combined vector
    //for (const Point &point : combined_vector) {
    //  cout << "comb(" << point.x << ", " << point.y << ")\n";
    //}
    /*
    // Sort the solutions1 vector based on x and y coordinates
    sort(solutions1.begin(), solutions1.end(),
         [](const Point &a, const Point &b) { //replace with dme 
           if (a.x == b.x) //replace with dme 
             return a.y < b.y; //replace with dme 
           return a.x < b.x; //replace with dme 
         }); 
    cout << endl; //replace with dme 
    // Return the lowest value from the sorted solutions2
    if (!solutions1.empty()) { //replace with dme 
      cout << "Lowest value in solutions1: (" << solutions1.front().x << ", " //replace with dme 
           << solutions1.front().y << ")\n"; //replace with dme 
    } //replace with dme 

    // Sort the solutions2 vector based on x and y coordinates
    sort(solutions2.begin(), solutions2.end(),
         [](const Point &a, const Point &b) { //replace with dme 
           if (a.x == b.x) //replace with dme 
             return a.y < b.y; //replace with dme 
           return a.x < b.x; //replace with dme 
         }); //replace with dme 

    // Return the lowest value from the sorted solutions2
    if (!solutions2.empty()) { //replace with dme 
      cout << "Lowest value in solutions2: (" << solutions2.front().x << ", "  //replace with dme 
           << solutions2.front().y << ")\n"; //replace with dme 
    } //replace with dme 
    // cout<<findLCA(root,id1,id2)->id<<endl;
    if (!solutions1.empty() && !solutions2.empty()) { //replace with dme 
      parent->x = (solutions1.front().x + solutions2.front().x) / 2; //replace with dme 
      parent->y = (solutions1.front().y + solutions2.front().y) / 2; //replace with dme 
      parent->z = node1->z; //replace with dme 
    } //replace with dme */
    //linear_planar_dme(solutions1);
    //linear_planar_dme(solutions2);
    //cout<<"combined dme";
    //cout<<endl;
    //linear_planar_dme(solutions);
    // Call linear_planar_dme and get the merging point
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
        ancestors[i]->x = mergingPoints[i].x;
        ancestors[i]->y = mergingPoints[i].y;
        ancestors[i]->z = node1->z; // Assuming z-coordinate remains the same
        cout << "Merged At: (" << ancestors[i]->x << ", " << ancestors[i]->y << ") for Node ID: " << ancestors[i]->id << endl;
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
        if (point.x == x2 && point.x >= 0 && point.y >= 0) {
          solutions.push_back(point);
        }
      }
      for (const Point &point : solutions) {
        cout << "(" << point.x << ", " << point.y << ")\n";
      }
      /*
      sort(solutions.begin(), solutions.end(),
           [](const Point &a, const Point &b) { //replace with dme 
             if (a.x == b.x) //replace with dme 
               return a.y < b.y; //replace with dme 
             return a.x < b.x; //replace with dme 
           }); //replace with dme 
      cout << endl;
      // Return the lowest value from the sorted solutions
      if (!solutions.empty()) { //replace with dme 
        cout << "Lowest value in solutions: (" << solutions.front().x << ", " 
             << solutions.front().y << ")\n"; //replace with dme 

        parent->x = solutions.front().x; //replace with dme 
        parent->y = solutions.front().y; //replace with dme 
        parent->z = node1->z; //replace with dme 
      } //replace with dme  */
      //linear_planar_dme(solutions);
      // Call linear_planar_dme and get the merging point
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
          ancestors[i]->x = mergingPoints[i].x;
          ancestors[i]->y = mergingPoints[i].y;
          ancestors[i]->z = node1->z; // Assuming z-coordinate remains the same
          cout << "Merged At: (" << ancestors[i]->x << ", " << ancestors[i]->y << ") for Node ID: " << ancestors[i]->id << endl;
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
      extension = round(lPrime);
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
        if (point.x == x1 && point.x >= 0 && point.y >= 0) {
          solutions.push_back(point);
        }
      }
      for (const Point &point : solutions) {
        cout << "(" << point.x << ", " << point.y << ")\n";
      }
      
      //linear_planar_dme(solutions);
      // Call linear_planar_dme and get the merging point
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
          ancestors[i]->x = mergingPoints[i].x;
          ancestors[i]->y = mergingPoints[i].y;
          ancestors[i]->z = node1->z; // Assuming z-coordinate remains the same
          cout << "Merged At: (" << ancestors[i]->x << ", " << ancestors[i]->y << ") for Node ID: " << ancestors[i]->id << endl;
      }

      /*
      sort(solutions.begin(), solutions.end(),
           [](const Point &a, const Point &b) { //replace with dme 
             if (a.x == b.x) //replace with dme 
               return a.y < b.y; //replace with dme 
             return a.x < b.x; //replace with dme 
           });
      cout << endl;
      // Return the lowest value from the sorted solutions
      if (!solutions.empty()) { //replace with dme 
        cout << "Lowest value in solutions: (" << solutions.front().x << ", "
             << solutions.front().y << ")\n"; //replace with dme 

        parent->x = solutions.front().x; //replace with dme 
        parent->y = solutions.front().y; //replace with dme 
        parent->z = node1->z; //replace with dme 
      } */
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

// Structure to store a point
struct Point2D {
  double x, y;
  Point2D(double x = 0, double y = 0) : x(x), y(y) {}
};

void exportNode(Node *node, ofstream &file) {
  if (!node) {
    return;
  }
  // Export the current node (point)
  file << "P " << node->x << " " << node->y << std::endl;
  // Export the line to the left child
  if (node->leftChild) {
    file << "L " << node->x << " " << node->y << " " << node->leftChild->x
         << " " << node->leftChild->y << std::endl;
    exportNode(node->leftChild, file);
  }
  // Export the line to the right child
  if (node->rightChild) {
    file << "L " << node->x << " " << node->y << " " << node->rightChild->x
         << " " << node->rightChild->y << std::endl;
    exportNode(node->rightChild, file);
  }
}

void exportPointsAndLines(Node *root, const std::string &filename) {
  std::ofstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Error opening file for writing: " << filename << std::endl;
    return;
  }
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


int main() {
  int bound = 10;
  parseInput("benchmark7.txt");
  displayParsedData();
  // New block to separate sinks by their z-coordinate
  map<int, vector<Sink>> sinksByZ;
  for (const auto &sink : sinks) {
    sinksByZ[sink.z].push_back(sink);
  }
  vector<Node *>
      roots; // To store the root of each subtrees created per z-coordinate
  for (const auto &pair : sinksByZ) {
    int z = pair.first;
    const auto &sinksGroup = pair.second;
    // For each z-coordinate, generate the tree and then perform zero skew tree
    Node *root = AbsTreeGen3D(sinksGroup, bound);
    root->z = z; // Assign the z coordinate of the sink group to the root node
    assignPhysicalLocations(root);
    hierarchicalDelay(root);
    exportPointsAndLines(root, "tree_z_" + to_string(z) +
                                   ".txt"); // Export to Points and Lines fil
    // cout<<getNodeDelay(root, 0)<<endl;
    // cout<<getNodeDelay(root, 1)<<endl;
    // cout<<getNodeDelay(root, 2)<<endl;
    //  colorTree(root, dieColors, clockSource);
    cout << "\nProcessing z-coordinate: " << z << endl;
    roots.push_back(zeroSkewTree(
        root)); // Apply zero skew tree operation and store the root
    
  }

  for (const auto &root : roots) {

    
    exportPointsAndLines(
        root, "zeroskew_points_and_lines_z_" + to_string(root->z) +
                  ".txt"); // Export ZeroSkewTree to Points and Lines file
  }

  for (auto root : roots) {
    cout << "\nZero Skew Tree for tier: " << root->z << endl;
    cout << endl;
    printTree(root);
    deleteTree(root); // Clean up each subtree
  }
  return 0;
}
