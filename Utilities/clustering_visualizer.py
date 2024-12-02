import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import sys
import os
import glob


def read_csv_data(file_path):
    df = pd.read_csv(file_path)
    df['cluster'] = pd.to_numeric(df['cluster'], errors='coerce')
    return df


def plot_clusters(df, interactive=False, output_filename='output_plot.png'):
    unique_clusters = df['cluster'].unique()
    color_map = plt.cm.get_cmap('tab20')
    colors = color_map(np.linspace(0, 1, len(unique_clusters)))
    plt.figure(figsize=(12, 14))  # Increase figure size for more space
    for cluster, color in zip(unique_clusters, colors):
        cluster_points = df[df['cluster'] == cluster]
        plt.scatter(cluster_points['x'],
                    cluster_points['y'],
                    c=[color],
                    label=f'Cluster {cluster}',
                    alpha=0.7)
    plt.xlabel('X coordinate', fontsize=14)
    plt.ylabel('Y coordinate', fontsize=14)
    plt.title('Clusters colored by their cluster IDs', fontsize=16)
    # Adjust legend to be below the plot
    total_clusters = len(unique_clusters)
    ncol = min(total_clusters, 8)
    legend = plt.legend(loc='upper center',
                        fontsize='medium',
                        framealpha=0.75,
                        ncol=ncol,
                        bbox_to_anchor=(0.5, -0.075))
    legend.get_frame().set_facecolor('white')
    plt.tight_layout(pad=3.0)
    plt.subplots_adjust(top=0.85, bottom=0.2)
    if interactive:
        plt.ion()  # Turn on interactive mode
        plt.draw()
        plt.pause(0.001)
        input('Press Enter to continue...')
    else:
        plt.savefig(output_filename,
                    bbox_inches='tight')  # Save with the new filename
        # plt.show() <- Remove this line to prevent opening the plot in non-interactive mode.
        plt.close(
        )  # Close the plot to free up memory and ensure the program exits properly


def get_csv_files(subdir):
    pattern = os.path.join(subdir, '*.csv')  # Adjust the file path as needed
    return glob.glob(pattern)


if __name__ == "__main__":
    subdir = '.'
    interactive = False
    if len(sys.argv) > 1:
        subdir = sys.argv[1]
        if len(sys.argv) > 2:
            interactive = sys.argv[2].lower() == 'true'
    else:
        subdir_input = input(
            "Enter subdirectory name (or press Enter for current dir): "
        ).strip() or '.'
        interactive_input = input(
            "Do you want to view plots interactively? (true/false): ").strip()
        interactive = interactive_input.lower() == 'true'
    csv_files = get_csv_files(subdir)
    if not csv_files:
        print(f"No CSV files found in {subdir}")
        sys.exit(1)
    print(f"Found {len(csv_files)} files to process")
    for file_path in csv_files:
        print(f"Processing: {file_path}")
        df = read_csv_data(file_path)
        # Generate the output file name based on the input file name
        base_filename = os.path.splitext(os.path.basename(file_path))[0]
        output_filename = os.path.join(os.path.dirname(file_path),
                                       f"{base_filename}_output_plot.png")
        plot_clusters(df, interactive,
                      output_filename)  # Pass the output filename
    print("Plots processed successfully.")
