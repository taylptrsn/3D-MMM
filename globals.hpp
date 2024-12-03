#pragma once
#include "structures.hpp"
#include <vector>
#include <string>

using namespace std;

// Global Variables
Layout layout;
WireUnits wireUnits;
BufferUnits bufferUnits;
TSVUnits tsvUnits;
ClockSource clockSource;
vector<Sink> sinks;
vector<string> dieColors = {"Gray", "Red",    "Green",
                            "Blue", "Purple", "Lime"}; // Define colors for dies
int numSinks;
int nodeID = 0;         // Global variable to keep track of the next node ID
int zCutCount = 0;      // Keeps track of the number of Z-cuts performed
int ZeroSkewMerges = 0; // Keeps track of the number of ZSMs performed
