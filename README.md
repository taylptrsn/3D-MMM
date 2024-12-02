# CLUSTER DME 3D CLOCK TREE SYNTHESIS PROGRAM

## TODO
- Automatic Unit Conversion on Program Data Input and Output
- Find way to make sure MIV delay/cap from previous tier is not overwritten/is
assigned correctly when calculating merging points of next tier.
- Note: This program is fairly robust, but there are still some edge cases that
may cause it to throw an error or infinite loop!
## FUNCTION
This program implements a clock tree synthesis algorithm for multi-die designs using Zero Skew Merge (ZSM) and Deferred Merge Embedding (DME) techniques. It optimizes clock distribution networks by minimizing wire length while maintaining zero clock skew.

## INPUT
The program reads a benchmark file (e.g., `benchmark0.txt`) with the following format:
- **Line 1**: Layout area dimensions and number of dies
- **Line 2**: Wire resistance (ohm/um) and capacitance (fF/um)
- **Line 3**: Buffer characteristics (resistance, capacitance, delay)
- **Line 4**: TSV/MIV characteristics (resistance, capacitance)
- **Line 5**: Clock source location and output resistance
- **Line 6**: Number of sinks
- **Following lines**: Sink coordinates (x,y,z) and capacitance

## OUTPUT
- Generates a log file with a timestamp containing:
  - Ideal wirelength sum
  - Zero skew tree wirelength sum
  - Cluster ZST total
  - Execution time
- Provides detailed merging point locations and wire routing information
- Intermediate outputs
  - Clustering Visualization Output text for each tier, used by python utility function /Utilities/clustering_visualizer.py
  - Clock Tree Visualization Output text for each tier, used by python utility function /Utilities/clock_tree_visualizer.py

## DEPENDENCIES
- C++ compiler with C++11 support
- Make build system
- Standard C++ libraries:
  - `<iostream>`
  - `<vector>`
  - `<algorithm>`
  - `<chrono>`
  - `<fstream>`
  - `<map>`
  - `<unordered_map>`

## BUILD
To compile the program, use:
`make`

# Sink Generation Script

This script generates random sink data, including coordinates and capacitance values, and writes it to an output file.

## Overview

The script creates a specified number of sinks with unique coordinates in a 2D layout. Each sink also has a randomly generated capacitance value. The output is formatted and saved in a text file.

## Inputs

The script uses the following parameters:

- **layout_x (int)**: The width of the layout (default: 1000).
- **layout_y (int)**: The height of the layout (default: 1000).
- **num_dies (int)**: The number of dies (default: 3).
- **wire_resistance (float)**: Unit wire resistance (ohm/um) (default: 0.1).
- **wire_capacitance (float)**: Unit wire capacitance (fF/um) (default: 0.2).
- **buffer_resistance (int)**: Output resistance of the buffer (ohm) (default: 122).
- **buffer_input_cap (int)**: Input capacitance of the buffer (fF) (default: 24).
- **buffer_delay (int)**: Intrinsic delay of the buffer (ps) (default: 17).
- **tsv_resistance (float)**: Resistance of the TSV (ohm) (default: 0.035).
- **tsv_capacitance (int)**: Capacitance of the TSV (fF) (default: 15).
- **clock_source_x (int)**: X coordinate of the clock source (default: 50).
- **clock_source_y (int)**: Y coordinate of the clock source (default: 0).
- **clock_source_z (int)**: Z coordinate of the clock source (default: 1).
- **clock_source_resistance (int)**: Resistance at the clock source (ohm) (default: 100).
- **num_sinks (int)**: The number of sinks to be generated (default: 800).

## Outputs

The script generates a text file named `output.txt` with the following format:




