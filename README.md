# CLOCK TREE SYNTHESIS PROGRAM

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
