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

## Requirements

```python
matplotlib
numpy
pandas
