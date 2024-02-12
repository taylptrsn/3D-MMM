#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>

using namespace std;

// Structure Definitions
struct Layout {
  double width;
  double height;
  int numDies;
};
struct WireUnits {
  double resistance;
  double capacitance;
};
struct BufferUnits {
  double outputResistance;
  double inputCapacitance;
  double intrinsicDelay;
};
struct TSVUnits {
  double resistance;
  double capacitance;
};
struct ClockSource {
  double x;
  double y;
  int z;
  double outputResistance;
};
struct Sink {
  double x;
  double y;
  int z;
  double inputCapacitance;
  string color;
  // Constructor with default sink color set to gray/uncolored
  Sink(int x = 0, int y = 0, int z = 0, double inputCapacitance = 0,
       string color = "Gray")
      : x(x), y(y), z(z), inputCapacitance(inputCapacitance), color(color) {}
};
struct Node {
  vector<Sink> sinks;
  Node *leftChild;
  Node *rightChild;
  string color;
  int dieIndex; // Added attribute for die index
  Node(const vector<Sink> &sinks, string color = "Gray")
      : sinks(sinks), leftChild(nullptr), rightChild(nullptr), color(color) {}
};

// Global Variables
Layout layout;
WireUnits wireUnits;
BufferUnits bufferUnits;
TSVUnits tsvUnits;
ClockSource clockSource;
int numSinks;
int zCutCount = 0; // Keeps track of the number of Z-cuts performed
vector<Sink> sinks;
vector<string> dieColors = {"Gray",   "Red", "Green", "Blue",
                            "Purple", "Lime"}; //  Define colors for dies

// Utility Functions
// Function to calculate the median of x coordinates
double calculateMedianX(const vector<Sink> &sinks) {
  vector<double> xCoordinates;
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
double calculateMedianY(const vector<Sink> &sinks) {
  vector<double> yCoordinates;
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
    throw std::runtime_error("No sinks available to determine minimum X.");
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
    throw std::runtime_error("No sinks available to determine maximum X.");
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
    throw std::runtime_error("No sinks available to determine minimum Y.");
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
    throw std::runtime_error("No sinks available to determine maximum Y.");
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
    throw std::runtime_error("No sinks available to determine minimum Z.");
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
    throw std::runtime_error("No sinks available to determine maximum Z.");
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
  cout << "B " << B << endl;
  int B1 = 0;
  int B2 = 0;
  int deltaX = getMaxX(S) - getMinX(S);
  int deltaY = getMaxY(S) - getMinY(S);
  int deltaZ = getMaxZ(S) - getMinZ(S);
  vector<Sink> St, Sb;
  ClockSource Zs = clockSource;
  if (S.size() == 1) {
    // Base case: if die span = 1, 2d tree
    return new Node(S);
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
    double medianX = calculateMedianX(S);
    double medianY = calculateMedianY(S);
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
    cout << "B1 " << B1 << endl;
    cout << "B2 " << B2 << endl;
  }
  /* cout << "St (Top most die group):" << endl;
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
  } */
  Node *root = new Node(S);
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

  string indent =
      string(level * 8, ' '); // Increase indentation for each level.

  // Print the left branch (child) first.
  if (node->leftChild) {
    printTree(node->leftChild, level + 1);
  }

  // Then, print the current node along with its color.
  if (node->leftChild || node->rightChild) {
    // For internal nodes, display their color (if applicable).
    cout << indent << "Level " << level
         << " - Internal Node, Color: " << node->color << endl;
  } else {
    // For leaf nodes, list all sinks and their colors.
    cout << indent << "Level " << level << " - Sinks (Leaves):" << endl;
    for (const auto &sink : node->sinks) {
      cout << indent << "Sink: (" << sink.x << ", " << sink.y << ", " << sink.z
           << "), Color: " << sink.color << endl;
    }
  }
  cout << endl;
  // Finally, print the right branch (child).
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

  // Recurse for child nodes with incremented depth
  colorTree(node->leftChild, dieColors, source, depth + 1);
  colorTree(node->rightChild, dieColors, source, depth + 1);
}

bool compareByColor(const Sink &a, const Sink &b) { return a.color < b.color; }
void sortAndPrintSinksByColor(const vector<Sink> &sinks) {
  vector<Sink> sortedSinks = sinks;
  sort(sortedSinks.begin(), sortedSinks.end(), compareByColor);
  cout << "Sinks sorted by color:" << endl;
  for (const auto &sink : sortedSinks) {
    cout << "Color: " << sink.color << " - Sink: (" << sink.x << ", " << sink.y
         << ", " << sink.z << ")" << endl;
  }
}

vector<Sink> getSinksByColor(const vector<Sink> &allSinks,
                             const string &color) {
  vector<Sink> sinksByColor;
  for (const auto &sink : allSinks) {
    if (sink.color == color) {
      sinksByColor.push_back(sink);
    }
  }
  return sinksByColor;
}

void collectSinks(const Node *node, vector<Sink> &allSinks) {
  if (node == nullptr) {
    return;
  }
  // If it's a leaf node, add its sinks to the collection
  if (node->leftChild == nullptr && node->rightChild == nullptr) {
    for (const auto &sink : node->sinks) {
      allSinks.push_back(sink);
    }
  }
  // Recurse on the left and right children
  collectSinks(node->leftChild, allSinks);
  collectSinks(node->rightChild, allSinks);
}

// Function to sort sinks by their z coordinate
void sortSinksByZ(vector<Sink> &sinks) {
  sort(sinks.begin(), sinks.end(),
       [](const Sink &a, const Sink &b) { return a.z < b.z; });
}

void printOneSink(const Node *node) {
  if (node == nullptr) {
    return;
  }
  // Check if current node is a leaf node
  if (node->leftChild == nullptr && node->rightChild == nullptr &&
      !node->sinks.empty()) {
    // Print the first sink in this leaf node
    const auto &sink = node->sinks[0];
    cout << "One Sink: (" << sink.x << ", " << sink.y << ", " << sink.z
         << "), Input Capacitance - " << sink.inputCapacitance
         << " fF, Color - " << sink.color << endl;
    return; // Stop after printing one sink
  } else {
    // Recursively search for a leaf node in the left subtree first, then the
    // right subtree
    if (node->leftChild != nullptr) {
      printOneSink(node->leftChild);
    }
    if (node->rightChild != nullptr) {
      printOneSink(node->rightChild);
    }
  }
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

int main() {
  int bound = 10;
  // Call parseInput to read and parse the input file
  parseInput("benchmark1.txt");
  // sortSinksByZ(sinks);
  //  Display parsed data
  displayParsedData();

  // Generate the 3D tree
  Node *root = AbsTreeGen3D(sinks, bound);
  colorTree(root, dieColors, clockSource);
  cout << "Number of Z-cuts performed: " << zCutCount << endl;
  // Print the sinks of generated tree
  cout << endl;
  cout << "Sinks of Abstract Tree:" << endl;
  printLeaves(root);

  // Print the generated tree
  cout << endl;
  cout << "Abstract Tree:" << endl;
  printTree(root);

  // Assign colors to sinks in the tree
  // colorTree(root, dieColors);
  // Collect all sinks
  vector<Sink> allSinks;
  collectSinks(root, allSinks);
  cout << endl;
  sortAndPrintSinksByColor(allSinks);

  // Clean up memory
  deleteTree(root);
  return 0;
}
