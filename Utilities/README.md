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
```
___

## Testcase Generator
### INPUT
These parameters are set within the script:
  - Layout dimensions (layout_x, layout_y)
  - Number of dies (num_dies)
  - Wire properties (wire_resistance, wire_capacitance)
  - Buffer characteristics (buffer_resistance, buffer_input_cap, buffer_delay)
  - TSV properties (tsv_resistance, tsv_capacitance)
  - Clock source information (clock_source_x, clock_source_y, clock_source_z, clock_source_resistance)
  - Number of sinks (num_sinks)

### OUTPUT
- Generates an output.txt file containing:
  - Layout specifications
  - Wire and buffer parameters
  - TSV characteristics
  - Clock source information
  - Randomly generated sink coordinates and capacitance values

### Usage
- Configure the static parameters as well as the randomized parameter ranges in the script
- Then run via `python testcase_generator.py`
___

## Clock Tree Visualizer
### INPUT
The program reads a benchmark file (e.g., `benchmark0.txt`) with the following format:
- **Line 1**: Layout area dimensions and number of dies
- **Line 2**: Wire resistance (ohm/um) and capacitance (fF/um)


### OUTPUT
- Generates a log file with a timestamp containing:
  - Ideal wirelength sum
  - 
### Usage
`python clock_tree_visualizer.py [subdirectory] [interactive]`
___

## Clustering Visualizer
### INPUT
The program reads a benchmark file (e.g., `benchmark0.txt`) with the following format:
- **Line 1**: Layout area dimensions and number of dies
- **Line 2**: Wire resistance (ohm/um) and capacitance (fF/um)


### OUTPUT
- Generates a log file with a timestamp containing:
  - Ideal wirelength sum

### Usage
`python clock_tree_visualizer.py [subdirectory] [interactive]`


