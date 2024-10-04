import matplotlib.pyplot as plt
import itertools

def read_points_and_lines(filename):
    points = []
    lines = []

    with open(filename, 'r') as file:
        for line in file:
            parts = line.strip().split(' ')
            if parts[0] == 'P':  # Point
                x, y = float(parts[1]), float(parts[2])
                points.append((x, y))
            elif parts[0] == 'L':  # Line
                x1, y1, x2, y2 = float(parts[1]), float(parts[2]), float(parts[3]), float(parts[4])
                lines.append(((x1, y1), (x2, y2)))

    return points, lines

def plot_points_and_lines(points, lines, color='b'):
    fig, ax = plt.subplots()

    # Plot points
    for (x, y) in points:
        ax.plot(x, y, color + 'o')  # Colored dot for each point

    # Plot Manhattan-routed lines
    for ((x1, y1), (x2, y2)) in lines:
        if x1 == x2 or y1 == y2:
            ax.plot([x1, x2], [y1, y2], color + '-')  # Straight line
        else:
            ax.plot([x1, x1, x2], [y1, y2, y2], color + '-')  # Manhattan route

    ax.set_xlabel('X')
    ax.set_ylabel('Y')
    ax.set_title('Zero Skew Tree Visualization with Manhattan Routing')
    plt.grid(True)
    plt.show()

def plot_all_files_together(z_levels):
    fig, ax = plt.subplots()
    colors = itertools.cycle(['b', 'g', 'r', 'c', 'm', 'y', 'k'])  # Cycle through colors

    for z in z_levels:
        points, lines = read_points_and_lines(f'zeroskew_points_and_lines_z_{z}.txt')
        color = next(colors)

        # Plot points
        for (x, y) in points:
            ax.plot(x, y, color + 'o')  # Colored dot for each point

        # Plot Manhattan-routed lines
        for ((x1, y1), (x2, y2)) in lines:
            if x1 == x2 or y1 == y2:
                ax.plot([x1, x2], [y1, y2], color + '-')  # Straight line
            else:
                ax.plot([x1, x1, x2], [y1, y2, y2], color + '-')  # Manhattan route

    ax.set_xlabel('X')
    ax.set_ylabel('Y')
    ax.set_title('Combined Zero Skew Tree Visualization with Manhattan Routing')
    plt.grid(True)
    plt.show()

if __name__ == "__main__":
    z_levels = [1, 2, 3, 4, 5]  

    # Plot each file individually with different colors
    colors = itertools.cycle(['b', 'g', 'r', 'c', 'm', 'y', 'k'])  # Cycle through colors
    for z in z_levels:
        points, lines = read_points_and_lines(f'zeroskew_points_and_lines_z_{z}.txt')
        plot_points_and_lines(points, lines, next(colors))

    # Plot all files together
    plot_all_files_together(z_levels)
