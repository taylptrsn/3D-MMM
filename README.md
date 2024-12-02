# CLUSTER DME CLOCK TREE SYNTHESIS PROGRAM

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
```bash
make



