#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>
/* TODO
- Unit Conversion at Data read-in
- Split into header files
- Complete Zeroskewtree function
- Make main call zeroskewtree for each die level
- Delay buffering instead of wire elongation
- rework ZeroSkewMerge
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
int nodeID = 0;    // Global variable to keep track of the next node ID
int zCutCount = 0; // Keeps track of the number of Z-cuts performed
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
    // Assuming all sinks in a node have the same color, so assign the node's
    // color
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

vector<Point> findPoints(double x1, double y1, double mDist) {
  return {
      {x1 + mDist, y1}, // Right
      {x1 - mDist, y1}, // Left
      {x1, y1 + mDist}, // Up
      {x1, y1 - mDist}  // Down
  };
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
    // (already updated by depthFirstCapacitance)
    double subtreeCapacitance =
        node->capacitance; // Assuming updated by depthFirstCapacitance
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
// Function to calculate the Zero Skew Merging point and adjust wire length for
// zero skew
double ZeroSkewMerge(Node *root, int id1, int id2) {
  // Use global variables for resistance and capacitance per unit length
  double resistancePerUnitLength = wireUnits.resistance;
  double capacitancePerUnitLength = wireUnits.capacitance;

  Node *node1 = findNodeById(root, id1);
  Node *node2 = findNodeById(root, id2);
  double delaySegment1=getNodeDelay(root,id1);
  double delaySegment2=getNodeDelay(root,id2);
  double capacitanceSegment1=getNodeCapacitance(root,id1);
  double capacitanceSegment2=getNodeCapacitance(root,id2);
  int lengthOfWire=calculateManhattanDistance(root,id1,id2);
  
  cout<<endl;
  double x1 = node1->x;
  double y1 = node1->y;
  double x2 = node2->x;
  double y2 = node2->y;
  cout<<"x1: "<<x1<<endl;
  cout<<"y1: "<<y1<<endl;
  cout<<"x2: "<<x2<<endl;
  cout<<"y2: "<<y2<<endl;
  // Calculate the initial merging point x
  double numerator = (delaySegment2 - delaySegment1) +
                     resistancePerUnitLength * lengthOfWire *
                         (capacitanceSegment2 + (capacitancePerUnitLength / 2));
  double denominator =
      resistancePerUnitLength * lengthOfWire *
      (capacitancePerUnitLength + capacitanceSegment1 + capacitanceSegment2);
  double mergingPointX = numerator / denominator;

  if (mergingPointX >= 0 && mergingPointX <= 1) {
    cout << "Tapping point in range, calculating merging point" << endl;
    cout << "X: " << mergingPointX << endl;
    cout<< "lenth of wire: "<<lengthOfWire<<endl;
    cout << "merge point sink 1 :" <<ceil(mergingPointX*lengthOfWire) << endl;
    cout << "merge point sink 2 :" << floor((1-mergingPointX)*lengthOfWire) << endl;
    vector<Point> points1 = findPoints(x1, y1, ceil(mergingPointX*lengthOfWire));
    vector<Point> points2 = findPoints(x2, y2, floor((1-mergingPointX)*lengthOfWire));
    for (const Point &point : points1) {
      cout << "(" << point.x << ", " << point.y << ")\n";
    }
    
    for (const Point &point : points2) {
      cout << "(" << point.x << ", " << point.y << ")\n";
    }
    return mergingPointX *
           lengthOfWire; // length for sink 1 = l*x, length for sink 2 = l*(1-x)
    
  } else {
    double lPrime = 0;
    if (mergingPointX > 1) {
      // For x > 1, tapping point exactly on subtree 2
      cout << "Tapping point out of range( > 1), extending" << endl;
      lPrime = (sqrt(pow(resistancePerUnitLength * capacitanceSegment1, 2) +
                     2 * resistancePerUnitLength * capacitancePerUnitLength *
                         (delaySegment2 - delaySegment1)) -
                resistancePerUnitLength * capacitanceSegment1) /
               (resistancePerUnitLength * capacitancePerUnitLength);

      vector<Point> points = findPoints(x2, y2, lPrime);
      cout << endl;
      cout << "Unique points satisfying the equation are:\n";
      for (const Point &point : points) {
        cout << "(" << point.x << ", " << point.y << ")\n";
      }
      cout << endl;
      cout << "Valid solutions rooted at subtree 2 are:\n";
      for (const Point &point : points) {
        if (point.x == x2 && point.x >= 0 && point.y >=0) { 
        cout << "(" << point.x << ", " << point.y << ")\n";
        }
      }
    } else {
      // For x < 0, tapping point on root of subtree 1
      cout << "Tapping point out of range( < 0), extending" << endl;
      lPrime = (sqrt(pow(resistancePerUnitLength * capacitanceSegment2, 2) +
                     2 * resistancePerUnitLength * capacitancePerUnitLength *
                         (delaySegment1 - delaySegment2)) -
                resistancePerUnitLength * capacitanceSegment2) /
               (resistancePerUnitLength * capacitancePerUnitLength);

      vector<Point> points = findPoints(x1, y1, lPrime);
      cout << endl;
      cout << "Unique points satisfying the equation are:\n";
      for (const Point &point : points) {
        cout << "(" << point.x << ", " << point.y << ")\n";
      }
      cout << endl;
      cout << "Valid solutions rooted at subtree 1 are:\n";
      for (const Point &point : points) {
        if (point.x == x1 && point.x >= 0 && point.y >=0){ 
        cout << "(" << point.x << ", " << point.y << ")\n";
        }
      }
    }
    
    return lPrime;
  }
}


/* Pseudocode
Node *zeroSkewTree(Node *root) {
  Top down, from clock source / root if subtree has 2 locations in x, y, z,
      zeroskew merge if any node doesnt have a physical location,
      visit children to see if they have 2 locations and perform
          zeroskewmerge if they do
} */

// Helper function to check if a node has a physical location
bool hasPhysicalLocation(const Node *node) {
  return node != nullptr && !(node->x == -1 && node->y == -1 && node->z == -1);
}

// Main recursive function to perform zero skew merging WIP
Node *zeroSkewTree(Node *root) {
  if (!root || (root->leftChild == nullptr && root->rightChild == nullptr)) {
    // Leaf node or empty subtree, no merging required
    return root;
  }

  // First, process the subtrees
  root->leftChild = zeroSkewTree(root->leftChild);
  root->rightChild = zeroSkewTree(root->rightChild);

  // Check if both children have physical locations
  if (hasPhysicalLocation(root->leftChild) &&
      hasPhysicalLocation(root->rightChild)) {

    // Perform ZeroSkewMerge to determine the optimal merging strategy
    // double mergePoint = ZeroSkewMerge(delay1, delay2, distance, cap1, cap2);

    // Update root's location based on the mergePoint
    // root->x = (root->leftChild->x + root->rightChild->x) / 2; // Simplified                  
    // root->y = (root->leftChild->y + root->rightChild->y) / 2; // Simplified
  }
  return root;
}

int main() {
  int bound = 10;
  // Call parseInput to read and parse the input file
  parseInput("benchmark1.txt");
  //  Display parsed data
  displayParsedData();

  // Generate the 3D tree
  Node *root = AbsTreeGen3D(sinks, bound);
  assignPhysicalLocations(root);
  colorTree(root, dieColors, clockSource);
  cout << "Number of Z-cuts performed: " << zCutCount << endl;
  // Print the sinks of generated tree
  cout << endl;
  cout << "Sinks of Abstract Tree:" << endl;
  printLeaves(root);
  // Calculate hierarchical delays for the tree
  hierarchicalDelay(root);
  // Print the generated tree
  cout << endl;
  cout << "Abstract Tree:" << endl;
  printTree(root);
  ZeroSkewMerge(root,1,3);
  /*cout << "ZSM "
   << ZeroSkewMerge(getNodeDelay(root, 3), getNodeDelay(root, 4),
                    calculateManhattanDistance(root, 3, 4),
                    getNodeCapacitance(root, 3),
                    getNodeCapacitance(root, 4))
   << endl; */

  // Clean up memory
  deleteTree(root);
  return 0;
}
