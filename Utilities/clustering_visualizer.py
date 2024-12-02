import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os


def plot_clusters(df, output_path=None):
    # Ensure cluster is treated as a numeric type
    df['cluster'] = pd.to_numeric(df['cluster'], errors='coerce')

    # Get unique cluster labels
    unique_clusters = df['cluster'].unique()

    # Create a color map with a unique color for each cluster
    color_map = plt.cm.get_cmap('tab20')
    colors = color_map(np.linspace(0, 1, len(unique_clusters)))

    # Create a scatter plot
    plt.figure(figsize=(7.5, 7.5))
    for cluster, color in zip(unique_clusters, colors):
        cluster_points = df[df['cluster'] == cluster]
        plt.scatter(cluster_points['x'],
                    cluster_points['y'],
                    c=[color],
                    label=f'Cluster {cluster}',
                    alpha=0.7)

    # Set labels and title
    plt.xlabel('X coordinate')
    plt.ylabel('Y coordinate')
    plt.title('Clusters colored by their cluster IDs')

    # Add a legend
    plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')

    # Adjust layout to prevent cutting off the legend
    plt.tight_layout()

    # Save or show the plot
    if output_path:
        plt.savefig(output_path)
        plt.close()
    else:
        plt.show()


def main():
    # Prompt the user for the directory
    dir_path = input(
        "Please enter the path to the directory containing CSV files: ")

    if not os.path.isdir(dir_path):
        print("Invalid directory. Exiting...")
        return

    # Create output directory for plots
    output_dir = os.path.join(dir_path, 'cluster_plots')
    os.makedirs(output_dir, exist_ok=True)

    # Process all CSV files in the directory
    csv_files = [f for f in os.listdir(dir_path) if f.endswith('.csv')]

    if not csv_files:
        print("No CSV files found in the selected directory.")
        return

    for csv_file in csv_files:
        try:
            # Read the CSV file
            file_path = os.path.join(dir_path, csv_file)
            df = pd.read_csv(file_path)

            # Create output path for the plot
            output_path = os.path.join(
                output_dir, f'{os.path.splitext(csv_file)[0]}_plot.png')

            # Generate and save the plot
            plot_clusters(df, output_path)
            print(f"Processed {csv_file} -> {output_path}")

        except Exception as e:
            print(f"Error processing {csv_file}: {str(e)}")


if __name__ == "__main__":
    main()
