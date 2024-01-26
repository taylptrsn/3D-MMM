#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>

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
  std::string color; // Add color attribute

  // Constructor with default color set to gray
  Sink(double x = 0, double y = 0, int z = 0, double inputCapacitance = 0,
       std::string color = "Gray")
      : x(x), y(y), z(z), inputCapacitance(inputCapacitance), color(color) {}
};

struct Node {
  std::vector<Sink> sinks;
  Node *leftChild;
  Node *rightChild;

  Node(const std::vector<Sink> &sinks)
      : sinks(sinks), leftChild(nullptr), rightChild(nullptr) {}
};

// Global Variables
Layout layout;
WireUnits wireUnits;
BufferUnits bufferUnits;
TSVUnits tsvUnits;
ClockSource clockSource;
int numSinks;
std::vector<Sink> sinks;

// Function to calculate the median of x coordinates
double calculateMedianX(const std::vector<Sink> &sinks) {
  std::vector<double> xCoordinates;
  xCoordinates.reserve(sinks.size()); // Pre-allocate for efficiency
  for (const auto &sink : sinks) {
    xCoordinates.push_back(sink.x);
  }

  std::sort(xCoordinates.begin(), xCoordinates.end());

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
double calculateMedianY(const std::vector<Sink> &sinks) {
  std::vector<double> yCoordinates;
  yCoordinates.reserve(sinks.size()); // Pre-allocate for efficiency
  for (const auto &sink : sinks) {
    yCoordinates.push_back(sink.y);
  }

  std::sort(yCoordinates.begin(), yCoordinates.end());

  if (yCoordinates.size() % 2 == 0) {
    // Even number of elements, take the average of the middle two
    size_t middleIndex = yCoordinates.size() / 2;
    return (yCoordinates[middleIndex - 1] + yCoordinates[middleIndex]) / 2;
  } else {
    // Odd number of elements, take the middle element
    return yCoordinates[yCoordinates.size() / 2];
  }
}

// Function to calculate the mean of x coordinates
double calculateMeanX(const std::vector<Sink> &sinks) {
  double sumX = 0.0;
  for (const auto &sink : sinks) {
    sumX += sink.x;
  }
  return sumX / sinks.size();
}

// Function to calculate the mean of y coordinates
double calculateMeanY(const std::vector<Sink> &sinks) {
  double sumY = 0.0;
  for (const auto &sink : sinks) {
    sumY += sink.y;
  }
  return sumY / sinks.size();
}

// Function to return the minimum x value in the set of sinks
double getMinX(const std::vector<Sink> &sinks) {
  if (sinks.empty()) {
    return std::numeric_limits<double>::quiet_NaN(); // Return NaN for empty set
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
double getMaxX(const std::vector<Sink> &sinks) {
  if (sinks.empty()) {
    return std::numeric_limits<double>::quiet_NaN(); // Return NaN for empty set
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
double getMinY(const std::vector<Sink> &sinks) {
  if (sinks.empty()) {
    return std::numeric_limits<double>::quiet_NaN(); // Return NaN for empty set
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
double getMaxY(const std::vector<Sink> &sinks) {
  if (sinks.empty()) {
    return std::numeric_limits<double>::quiet_NaN(); // Return NaN for empty set
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
double getMinZ(const std::vector<Sink> &sinks) {
  if (sinks.empty()) {
    return std::numeric_limits<double>::quiet_NaN(); // Return NaN for empty set
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
double getMaxZ(const std::vector<Sink> &sinks) {
  if (sinks.empty()) {
    return std::numeric_limits<double>::quiet_NaN(); // Return NaN for empty set
  }
  double maxZ = sinks[0].z;
  for (const auto &sink : sinks) {
    if (sink.z > maxZ) {
      maxZ = sink.z;
    }
  }
  return maxZ;
}

// Function to calculate the geometric mean of x coordinates
double calculateGeometricMeanX(const std::vector<Sink> &sinks) {
  double productX = 1.0;
  for (const auto &sink : sinks) {
    productX *= sink.x;
  }
  return std::pow(productX, 1.0 / sinks.size());
}

// Function to calculate the geometric mean of y coordinates
double calculateGeometricMeanY(const std::vector<Sink> &sinks) {
  double productY = 1.0;
  for (const auto &sink : sinks) {
    productY *= sink.y;
  }
  return std::pow(productY, 1.0 / sinks.size());
}

// Function to perform Z-cut
void Zcut(const std::vector<Sink> &S, const ClockSource &Zs,
          std::vector<Sink> &St, std::vector<Sink> &Sb) {
  std::cout << "Z-cut!" << std::endl;

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

  std::cout << "ending Z-cut!" << std::endl;
}

Node *AbsTreeGen3D(const std::vector<Sink> &S, int B) {
  int B1 = 0;
  int B2 = 0;
  int deltaX = getMaxX(S) - getMinX(S);
  int deltaY = getMaxY(S) - getMinY(S);

  std::vector<Sink> St, Sb;
  ClockSource Zs = clockSource;

  if (S.size() == 1) {
    // Base case: if die span = 1, 2d tree
    return new Node(S);
    // Case when B = 1, and span>1 perform Z-cut
  } else if (B == 1 && S.size() > 1) {
    Zcut(S, Zs, St, Sb);
    // std::cout << "post Z-cut!" << std::endl;
    //  Display St and Sb
    std::cout << "St (Top most die group):" << std::endl;
    for (const auto &sink : St) {
      std::cout << "(" << sink.x << "," << sink.y << "," << sink.z
                << ")(x,y,z), Input Capacitance - " << sink.inputCapacitance
                << " fF" << std::endl;
    }

    std::cout << "Sb (Bottom most die group):" << std::endl;
    for (const auto &sink : Sb) {
      std::cout << "(" << sink.x << "," << sink.y << "," << sink.z
                << ")(x,y,z), Input Capacitance - " << sink.inputCapacitance
                << " fF" << std::endl;
    }
    B1 = B2 = 1;
  } else {
    // Geometrically divide S into St and Sb using median X, lesser item -> left
    // node, bigger item -> right node
    double medianX = calculateMedianX(S);
    double medianY = calculateMedianY(S);
    for (const auto &sink : S) {
      if (deltaX > deltaY) {
        if (sink.y < medianY) {
          St.push_back(sink); // Left subset
        } else {
          Sb.push_back(sink); // Right subset
        }
      } else {
        if (sink.x < medianX) {
          St.push_back(sink); // Left subset
        } else {
          Sb.push_back(sink); // Right subset
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
    std::cout << std::setw(level * 7) << ""; // Adjust indentation
    std::cout << "Node:" << std::endl;

    if (!node->leftChild && !node->rightChild) {
      std::cout << std::setw((level + 1) * 6) << ""; // Adjust indentation
      std::cout << "Sink(Leaf):" << std::endl;
      for (const auto &sink : node->sinks) {
        std::cout << std::setw((level + 2) * 6) << ""; // Adjust indentation
        std::cout << "(" << sink.x << ", " << sink.y << ", " << sink.z << ")"
                  << std::endl;
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
      std::cout << "Color: " << sink.color
                << " - Sink: ("
                   "Leaf Nodes(Sinks):  ("
                << sink.x << ", " << sink.y << ", " << sink.z << ")"
                << std::endl;
    }
  } else {
    printLeaves(node->leftChild);
    printLeaves(node->rightChild);
  }
}

void colorTree(Node *node, const std::vector<std::string> &dieColors) {
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

void sortAndPrintSinksByColor(const std::vector<Sink> &sinks) {
  std::vector<Sink> sortedSinks = sinks;
  std::sort(sortedSinks.begin(), sortedSinks.end(), compareByColor);

  std::cout << "Sinks sorted by color:" << std::endl;
  for (const auto &sink : sortedSinks) {
    std::cout << "Color: " << sink.color << " - Sink: (" << sink.x << ", "
              << sink.y << ", " << sink.z << ")" << std::endl;
  }
}

std::vector<Sink> getSinksByColor(const std::vector<Sink> &allSinks,
                                  const std::string &color) {
  std::vector<Sink> sinksByColor;
  for (const auto &sink : allSinks) {
    if (sink.color == color) {
      sinksByColor.push_back(sink);
    }
  }
  return sinksByColor;
}

void collectSinks(const Node *node, std::vector<Sink> &allSinks) {
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
void sortSinksByZ(std::vector<Sink> &sinks) {
  std::sort(sinks.begin(), sinks.end(),
            [](const Sink &a, const Sink &b) { return a.z < b.z; });
}

int main() {
  int bound = 1;

  std::ifstream inputFile("edgecase-allsame.txt");

  if (!inputFile.is_open()) {
    std::cerr << "Error opening file!" << std::endl;
    return 1;
  }

  // Parse layout
  inputFile >> layout.width >> layout.height >> layout.numDies;
  inputFile.ignore(std::numeric_limits<std::streamsize>::max(),
                   '\n'); // Ignore the rest of the line

  // Parse unit wire
  inputFile >> wireUnits.resistance >> wireUnits.capacitance;
  inputFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

  // Parse buffer
  inputFile >> bufferUnits.outputResistance >> bufferUnits.inputCapacitance >>
      bufferUnits.intrinsicDelay;
  inputFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

  // Parse TSV
  inputFile >> tsvUnits.resistance >> tsvUnits.capacitance;
  inputFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

  // Parse clock source
  inputFile >> clockSource.x >> clockSource.y >> clockSource.z >>
      clockSource.outputResistance;
  inputFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

  // Parse number of sinks
  inputFile >> numSinks;
  inputFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

  // Parse sinks
  for (int i = 0; i < numSinks; ++i) {
    Sink sink;
    inputFile >> sink.x >> sink.y >> sink.z >> sink.inputCapacitance;
    sinks.push_back(sink);
    inputFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  }

  // Display parsed data
  std::cout << "Layout Area: (" << layout.width << "," << layout.height
            << ")(x,y)(um)" << std::endl;
  std::cout << "Number Of Dies: " << layout.numDies << std::endl;
  std::cout << "Unit Wire Resistance: " << wireUnits.resistance << "(ohm/um)"
            << std::endl;
  std::cout << "Unit Wire Capacitance: " << wireUnits.capacitance << "(fF/um)"
            << std::endl;
  std::cout << "Buffer Output Resistance(ohm): " << bufferUnits.outputResistance
            << std::endl;
  std::cout << "Buffer Input Capacitance(fF): " << bufferUnits.inputCapacitance
            << std::endl;
  std::cout << "Buffer Intrinsic Delay(ps): " << bufferUnits.intrinsicDelay
            << std::endl;
  std::cout << "TSV Resistance(ohm): " << tsvUnits.resistance << std::endl;
  std::cout << "TSV Capacitance(fF): " << tsvUnits.capacitance << std::endl;
  std::cout << "Clock Source: (" << clockSource.x << "," << clockSource.y << ","
            << clockSource.z << ")(x,y,z) " << std::endl;
  std::cout << "Clock Output resistance(ohm): " << clockSource.outputResistance
            << std::endl;

  std::cout << std::endl;
  std::cout << "Sinks:" << std::endl;
  for (const auto &sink : sinks) {
    std::cout << "(" << sink.x << "," << sink.y << "," << sink.z
              << ")(x,y,z), Input Capacitance - " << sink.inputCapacitance
              << " fF" << std::endl;
  }
  inputFile.close();

  std::cout << std::endl;
  std::cout << "Median of x coordinates: " << calculateMedianX(sinks)
            << std::endl;
  std::cout << "Mean of x coordinates: " << calculateMeanX(sinks) << std::endl;
  std::cout << "Geometric mean of x coordinates: "
            << calculateGeometricMeanX(sinks) << std::endl;
  std::cout << "Median of y coordinates: " << calculateMedianY(sinks)
            << std::endl;
  std::cout << "Mean of y coordinates: " << calculateMeanY(sinks) << std::endl;
  std::cout << "Geometric mean of y coordinates: "
            << calculateGeometricMeanY(sinks) << std::endl;
  // sortSinksByZ(sinks);
  Node *root = AbsTreeGen3D(sinks, bound);

  // Print the generated tree
  std::cout << std::endl;
  std::cout << "Sinks of Abstract Tree:" << std::endl;
  printLeaves(root);

  // std::cout << std::endl;
  std::cout << "Abstract Tree:" << std::endl;
  printTree(root);

  // Define colors for dies
  std::vector<std::string> dieColors = {"Red", "Green", "Blue", "Purple",
                                        "Lime"};

  // Assign colors to sinks in the tree
  colorTree(root, dieColors);

  // Collect all sinks
  std::vector<Sink> allSinks;
  collectSinks(root, allSinks);

  // Sort and print sinks by color
  // std::cout << std::endl;
  // sortAndPrintSinksByColor(allSinks);
  std::string selectedColor = "Purple";
  // Get sinks by the selected color
  std::vector<Sink> sinksOfSelectedColor =
      getSinksByColor(allSinks, selectedColor);

  // Print sinks of the selected color
  std::cout << "Sinks with color " << selectedColor << ":" << std::endl;
  for (const auto &sink : sinksOfSelectedColor) {
    std::cout << "(" << sink.x << ", " << sink.y << ", " << sink.z << ")"
              << std::endl;
  }
  // Clean up memory
  deleteTree(root);

  return 0;
}
