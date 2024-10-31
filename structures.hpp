#pragma once
#include <string>
#include <vector>
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
};