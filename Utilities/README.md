# Clock Tree Synthesis Visualization Tools

A collection of Python scripts for generating and visualizing clock tree synthesis data.

## Scripts Overview

**testcase_generator.py**
- Generates test cases for clock tree synthesis simulation
- Creates random sink coordinates and capacitance values
- Outputs data in a specific format for clock tree analysis
- Parameters include layout dimensions, wire properties, and buffer characteristics

**clock_tree_visualizer.py**
- Visualizes clock tree structures with both Manhattan and direct routing
- Supports interactive and non-interactive plotting modes
- Features:
  - Displays regular nodes, leaf nodes, and MIV (Metal Inter-layer Via) points
  - Generates both Manhattan-style and direct-line routing visualizations
  - Saves high-resolution output images

**clustering_visualizer.py**
- Creates visual representations of clustered data points
- Generates scatter plots with unique colors for each cluster
- Supports batch processing of multiple CSV files
- Automatically saves plots to a designated output directory

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

## Requirements

```python
matplotlib
numpy
pandas
```