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
  Sink(double x = 0, double y = 0, int z = 0, double inputCapacitance = 0,
       string color = "Gray")
      : x(x), y(y), z(z), inputCapacitance(inputCapacitance), color(color) {}
};
struct Node {
  vector<Sink> sinks;
  Node *leftChild;
  Node *rightChild;
  Node(const vector<Sink> &sinks)
      : sinks(sinks), leftChild(nullptr), rightChild(nullptr) {}
};
// Global Variables
Layout layout;
WireUnits wireUnits;
BufferUnits bufferUnits;
TSVUnits tsvUnits;
ClockSource clockSource;
int numSinks;
vector<Sink> sinks;

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
double getMinX(const vector<Sink> &sinks) {
  if (sinks.empty()) {
    return numeric_limits<double>::quiet_NaN(); // Return NaN for empty set
  }
  double minX = sinks[0].x;
  for (const auto &sink : sinks) {
    if (sink.x < minX) {
      minX = sink.x;
    }
  }
  return minX;
}

// Function to return the maximum x value in the set of sinks
double getMaxX(const vector<Sink> &sinks) {
  if (sinks.empty()) {
    return numeric_limits<double>::quiet_NaN(); // Return NaN for empty set
  }
  double maxX = sinks[0].x;
  for (const auto &sink : sinks) {
    if (sink.x > maxX) {
      maxX = sink.x;
    }
  }
  return maxX;
}

// Function to return the minimum y value in the set of sinks
double getMinY(const vector<Sink> &sinks) {
  if (sinks.empty()) {
    return numeric_limits<double>::quiet_NaN(); // Return NaN for empty set
  }
  double minY = sinks[0].y;
  for (const auto &sink : sinks) {
    if (sink.y < minY) {
      minY = sink.y;
    }
  }
  return minY;
}

// Function to return the maximum y value in the set of sinks
double getMaxY(const vector<Sink> &sinks) {
  if (sinks.empty()) {
    return numeric_limits<double>::quiet_NaN(); // Return NaN for empty set
  }
  double maxY = sinks[0].y;
  for (const auto &sink : sinks) {
    if (sink.y > maxY) {
      maxY = sink.y;
    }
  }
  return maxY;
}

// Function to return the maximum y value in the set of sinks
double getMinZ(const vector<Sink> &sinks) {
  if (sinks.empty()) {
    return numeric_limits<double>::quiet_NaN(); // Return NaN for empty set
  }
  double minZ = sinks[0].z;
  for (const auto &sink : sinks) {
    if (sink.z < minZ) {
      minZ = sink.z;
    }
  }
  return minZ;
}

// Function to return the maximum y value in the set of sinks
double getMaxZ(const vector<Sink> &sinks) {
  if (sinks.empty()) {
    return numeric_limits<double>::quiet_NaN(); // Return NaN for empty set
  }
  double maxZ = sinks[0].z;
  for (const auto &sink : sinks) {
    if (sink.z > maxZ) {
      maxZ = sink.z;
    }
  }
  return maxZ;
}

// Function to perform Z-cut
void Zcut(const vector<Sink> &S, const ClockSource &Zs, vector<Sink> &St,
          vector<Sink> &Sb) {
  cout << endl;
  cout << "Z-cut!" << endl;
  int Zmin = getMinZ(S); // Set your Zmin value
  int Zmax = getMaxZ(S); // Set your Zmax value
  // If Zs is less than or equal to Zmin
  if (Zs.z <= Zmin) {
    if (!S.empty()) {
      Sb.push_back(S[0]); // Assuming the first sink is the bottom-most
      for (size_t i = 1; i < S.size(); ++i) {
        St.push_back(S[i]); // The rest of the sinks
      }
    }
  }
  // If Zs is greater than or equal to Zmax
  else if (Zs.z >= Zmax) {
    if (!S.empty()) {
      St.push_back(S[0]); // Assuming the first sink is the top-most
      for (size_t i = 1; i < S.size(); ++i) {
        Sb.push_back(S[i]); // The rest of the sinks
      }
    }
  }
  // If Zs is between Zmin and Zmax
  else {
    for (const auto &sink : S) {
      if (sink.z >= Zs.z) {
        St.push_back(sink); // S top
      } else {
        Sb.push_back(sink); // S bottom
      }
    }
  }
  //cout << "ending Z-cut!" << endl;
}

Node *AbsTreeGen3D(const vector<Sink> &S, int B) {
  int B1 = 0;
  int B2 = 0;
  int deltaX = getMaxX(S) - getMinX(S);
  int deltaY = getMaxY(S) - getMinY(S);
  vector<Sink> St, Sb;
  ClockSource Zs = clockSource;
  if (S.size() == 1) {
    // Base case: if die span = 1, 2d tree
    return new Node(S);
    // Case when B = 1, and span>1 perform Z-cut
  } else if (B == 1 && S.size() > 1) {
    Zcut(S, Zs, St, Sb);
    // cout << "post Z-cut!" << endl;
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
        if (sink.y < medianY) {
          St.push_back(sink); // top subset
        } else {
          Sb.push_back(sink); // bottom subset
        }
      } else {
        if (sink.x < medianX) {
          St.push_back(sink); // top subset
        } else {
          Sb.push_back(sink); // bottom subset
        }
      }
    }
    B1 = B / 2;
    B2 = B - B1;
  }
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
  if (node) {
    // Print the left branch first (top)
    if (node->leftChild) {
      printTree(node->leftChild, level + 1);
    }
    // Print the current node (center)
    cout << setw(level * 7) << ""; // Adjust indentation
    cout << "Node:" << endl;
    if (!node->leftChild && !node->rightChild) {
      cout << setw((level + 1) * 6) << ""; // Adjust indentation
      cout << "Sink(Leaf):" << endl;
      for (const auto &sink : node->sinks) {
        cout << setw((level + 2) * 6) << ""; // Adjust indentation
        cout << "(" << sink.x << ", " << sink.y << ", " << sink.z << ")"
             << endl;
      }
    }
    // Print the right branch last (bottom)
    if (node->rightChild) {
      printTree(node->rightChild, level + 1);
    }
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

void colorTree(Node *node, const vector<string> &dieColors) {
  if (node == nullptr) {
    return;
  }
  if (node->leftChild == nullptr && node->rightChild == nullptr) {
    for (auto &sink : node->sinks) {
      sink.color = dieColors[sink.z % dieColors.size()];
    }
  } else {
    colorTree(node->leftChild, dieColors);
    colorTree(node->rightChild, dieColors);
  }
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

int main() {
  int bound = 777;
  ifstream inputFile("benchmark0.txt");
  if (!inputFile.is_open()) {
    cerr << "Error opening file!" << endl;
    return 1;
  }
  // Parse layout
  inputFile >> layout.width >> layout.height >> layout.numDies;
  inputFile.ignore(numeric_limits<streamsize>::max(),
                   '\n'); // Ignore the rest of the line
  // Parse unit wire
  inputFile >> wireUnits.resistance >> wireUnits.capacitance;
  inputFile.ignore(numeric_limits<streamsize>::max(), '\n');
  // Parse buffer
  inputFile >> bufferUnits.outputResistance >> bufferUnits.inputCapacitance >>
      bufferUnits.intrinsicDelay;
  inputFile.ignore(numeric_limits<streamsize>::max(), '\n');
  // Parse TSV
  inputFile >> tsvUnits.resistance >> tsvUnits.capacitance;
  inputFile.ignore(numeric_limits<streamsize>::max(), '\n');
  // Parse clock source
  inputFile >> clockSource.x >> clockSource.y >> clockSource.z >>
      clockSource.outputResistance;
  inputFile.ignore(numeric_limits<streamsize>::max(), '\n');
  // Parse number of sinks
  inputFile >> numSinks;
  inputFile.ignore(numeric_limits<streamsize>::max(), '\n');
  // Parse sinks
  for (int i = 0; i < numSinks; ++i) {
    Sink sink;
    inputFile >> sink.x >> sink.y >> sink.z >> sink.inputCapacitance;
    sinks.push_back(sink);
    inputFile.ignore(numeric_limits<streamsize>::max(), '\n');
  }
  // Display parsed data
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
  inputFile.close();
  cout << endl;
  cout << "Median of x coordinates: " << calculateMedianX(sinks) << endl;
  cout << "Median of y coordinates: " << calculateMedianY(sinks) << endl;
  
  // Generate the 3D tree
  Node *root = AbsTreeGen3D(sinks, bound);
  // Print the generated tree
  cout << endl;
  cout << "Sinks of Abstract Tree:" << endl;
  printLeaves(root);
  //cout << "Abstract Tree:" << endl;
  //printTree(root);
  // Define colors for dies
  vector<string> dieColors = {"Red", "Green", "Blue", "Purple", "Lime"};
  // Assign colors to sinks in the tree
  colorTree(root, dieColors);
  // Collect all sinks
  vector<Sink> allSinks;
  collectSinks(root, allSinks);
  //Sort and print sinks by color
  cout << endl;
  sortAndPrintSinksByColor(allSinks);
  /*string selectedColor = "Purple";
  // Get sinks by the selected color
  vector<Sink> sinksOfSelectedColor = getSinksByColor(allSinks, selectedColor);
  // Print sinks of the selected color
  cout << "Sinks with color " << selectedColor << ":" << endl;
  for (const auto &sink : sinksOfSelectedColor) {
    cout << "(" << sink.x << ", " << sink.y << ", " << sink.z << ")" << endl;
  } */
  // Clean up memory
  deleteTree(root);
  return 0;
}
