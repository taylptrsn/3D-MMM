import matplotlib.pyplot as plt
import itertools
import os
import glob
import sys


def read_points_and_lines_with_leaf_status(filename):
    points = []
    lines = []
    leaf_points = []
    miv_points = []

    with open(filename, 'r') as file:
        for line in file:
            parts = line.strip().split(' ')
            if parts[0] == 'P':  # Point
                x, y = float(parts[1]), float(parts[2])
                if len(parts) > 3:
                    if parts[3] == '(Leaf':
                        leaf_points.append((x, y))
                    elif parts[3] == '(MIV':
                        miv_points.append((x, y))
                else:
                    points.append((x, y))
            elif parts[0] == 'L':  # Line
                x1, y1, x2, y2 = float(parts[1]), float(parts[2]), float(
                    parts[3]), float(parts[4])
                lines.append(((x1, y1), (x2, y2)))

    return points, lines, leaf_points, miv_points


def plot_points_and_lines_manhattan(points,
                                    lines,
                                    leaf_points,
                                    miv_points,
                                    color='b',
                                    interactive=False):
    fig, ax = plt.subplots()
    # Plot regular points
    for (x, y) in points:
        ax.plot(x, y, color + 'o')  # Colored dot for each point
    # Plot Manhattan-routed lines
    for ((x1, y1), (x2, y2)) in lines:
        if x1 == x2 or y1 == y2:
            ax.plot([x1, x2], [y1, y2], color + '-')  # Straight line
        else:
            ax.plot([x1, x1, x2], [y1, y2, y2], color + '-')  # Manhattan route
        # Plot leaf/sink points in gray color
    for (x, y) in leaf_points:
        ax.plot(x, y, 'o', color='gray')  # Gray dot for each leaf node
        # Plot MIV nodes in red
    for (x, y) in miv_points:
        ax.plot(x, y, 'o', color='red')  # Red dot for each MIV node
        ax.set_xlabel('X')
        ax.set_ylabel('Y')
        ax.set_title('Zero Skew Tree Visualization with Manhattan Routing')
        plt.grid(True)
        plt.tight_layout()  # Arrange subplots neatly
    if interactive:
        plt.show()  # Show the plot interactively if the toggle is set
    return fig


def plot_points_and_lines_direct(points,
                                 lines,
                                 leaf_points,
                                 miv_points,
                                 color='b',
                                 interactive=False):
    fig, ax = plt.subplots()
    # Plot regular points
    for (x, y) in points:
        ax.plot(x, y, color + 'o')  # Colored dot for each point
    # Plot direct lines
    for ((x1, y1), (x2, y2)) in lines:
        ax.plot([x1, x2], [y1, y2], color + '-')  # Direct line
    # Plot leaf/sink points in gray color
    for (x, y) in leaf_points:
        ax.plot(x, y, 'o', color='gray')  # Gray dot for each leaf node
    # Plot MIV nodes in red
    for (x, y) in miv_points:
        ax.plot(x, y, 'o', color='red')  # Red dot for each MIV node
        ax.set_xlabel('X')
        ax.set_ylabel('Y')
        ax.set_title('Zero Skew Tree Visualization with Direct Lines')
        plt.grid(True)
        plt.tight_layout()  # Arrange subplots neatly
    if interactive:
        plt.show()  # Show the plot interactively if the toggle is set
    return fig


def get_z_level_files(miv_dir='MIV', subdir=None):
    # Construct the path including the subdirectory if specified
    if subdir:
        base_path = os.path.join(miv_dir, subdir)
    else:
        base_path = miv_dir

    # Get all zeroskew files in the specified directory
    pattern = os.path.join(base_path, 'zeroskew_points_and_lines_z_*.txt')
    files = glob.glob(pattern)
    return sorted(files)


def save_plot(fig, filename):
    fig.savefig(filename, dpi=500)
    plt.close(fig)


if __name__ == "__main__":
    # Check if command line arguments are provided
    if len(sys.argv) > 2:
        subdir = sys.argv[1]
        interactive = sys.argv[2].lower() == 'true'
    else:
        subdir = input(
            "Enter subdirectory name in MIV folder (or press Enter for all): "
        ).strip()
        interactive_input = input(
            "Do you want to view plots interactively? (true/false): ").strip()
        interactive = interactive_input.lower() == 'true'
    # Get all z-level files from the specified subdirectory
    z_files = get_z_level_files(subdir=subdir)
    if not z_files:
        print(f"No zeroskew files found in MIV/{subdir if subdir else ''}")
        sys.exit(1)
    print(f"Found {len(z_files)} files to process")
    colors = itertools.cycle(['b', 'g', 'c', 'm', 'y', 'k', 'p'])
    for file_path in z_files:
        print(f"Processing: {file_path}")
        points, lines, leaf_points, miv_points = read_points_and_lines_with_leaf_status(
            file_path)
        current_color = next(colors)
        output_directory = os.path.dirname(file_path)
        base_filename = os.path.splitext(
            os.path.basename(file_path))[0]  # Remove .txt extension
        # Manhattan routing - remove the plt.figure() call here
        manhattan_fig = plot_points_and_lines_manhattan(
            points, lines, leaf_points, miv_points, current_color, interactive)
        manhattan_output_file = os.path.join(
            output_directory, f"{subdir}_manhattan_{base_filename}.png")
        save_plot(manhattan_fig, manhattan_output_file)
        # Direct lines - remove the plt.figure() call here
        direct_fig = plot_points_and_lines_direct(points, lines, leaf_points,
                                                  miv_points, current_color,
                                                  interactive)
        direct_output_file = os.path.join(
            output_directory, f"{subdir}_direct_{base_filename}.png")
        save_plot(direct_fig, direct_output_file)

        if interactive:
            plt.show()

    print("Plots processed and saved successfully.")
