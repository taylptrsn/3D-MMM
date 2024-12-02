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
Language: Python 3.11 or Above

Dependencies
```python
matplotlib
numpy
pandas
```
`pip install matplotlib pandas numpy`

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
  - TSV/MIV characteristics
  - Clock source information
  - Randomly generated sink coordinates and capacitance values

### Usage
- Configure the static parameters as well as the randomized parameter ranges in the script
- Then run via `python testcase_generator.py`
___

## Clock Tree Visualizer
### INPUT
Input files:
  - Text files in the specified MIV subdirectory with the pattern zeroskew_points_and_lines_z_*.txt
    
Optional command-line arguments:
  - subdirectory: Name of the subdirectory in the MIV folder (if not provided, user will be prompted)
  - interactive: Boolean flag(true/false) for interactive plot display (if not provided, user will be prompted) 

### OUTPUT
Generates two types of PNG image files for each input file, located in the same directory as input file:
  - Manhattan routing visualization: `{subdir}_manhattan_zeroskew_points_and_lines_z_*.png`
  - Direct line routing visualization: `{subdir}_direct_zeroskew_points_and_lines_z_*.png`
If interactive mode is enabled, displays plots on screen

### Usage
- Run via `python clock_tree_visualizer.py [subdirectory] [interactive]`
  - if the command line arguments are not provided the user will be prompted to indicate their choice
___

## Clustering Visualizer
### INPUT
Input files:
  - CSV files in the specified directory with columns:
    - 'x': X-coordinate
    - 'y': Y-coordinate
    -'cluster': Cluster ID
    
Optional command-line arguments:
  - subdirectory: Name of the subdirectory in the MIV folder (if not provided, user will be prompted)
  - interactive: Boolean flag(true/false) for interactive plot display (if not provided, user will be prompted)

### OUTPUT
Generates a PNG image files for each input file, located in the same directory as input file:
  - Clustering visualization: `{original_csv_name}_output_plot.png`

If interactive mode is enabled, displays plots on screen

### Usage
- Run via `python clustering_visualizer.py [subdirectory] [interactive]`
  - if the command line arguments are not provided the user will be prompted to indicate their choice


