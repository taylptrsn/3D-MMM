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

struct Point {
  double x, y;
  int clusterId; // -1 for noise, 0 for unvisited, >0 for cluster IDs
};

struct Sink {
  int x, y, z;             // Unit: None, coordinates in an unspecified grid
  double inputCapacitance; // Unit: femtofarads (fF)
  double capacitance;      // Unit: femtofarads (fF)
  string color;            // Unit: None, simply a descriptive string
  double delay;            // Unit: picoseconds (ps)
  int cluster_id;
  string sink_type;
  // Constructor with default sink color set to gray/uncolored
  Sink(int x = 0, int y = 0, int z = 0, double inputCapacitance = 0,
       double capacitance = 0, string color = "Gray", double delay = 0,
       int cluster_id = -1, string sink_type = "sink")
      : x(x), y(y), z(z), inputCapacitance(inputCapacitance),
        capacitance(capacitance), color(color), delay(delay),
        cluster_id(cluster_id), sink_type(sink_type) {}
};

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
       string node_type = "undefined", int cluster_id = -1,
       double bufferDelay = 0.0)
      : id(id), sinks(sinks), leftChild(nullptr), rightChild(nullptr),
        color(color), capacitance(capacitance), resistance(resistance),
        isBuffered(isBuffered), x(x), y(y), z(z), node_type(node_type),
        cluster_id(cluster_id), bufferDelay(bufferDelay) {}
};