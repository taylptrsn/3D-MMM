import matplotlib.pyplot as plt
import itertools

def read_points_and_lines_with_leaf_status(filename):
    points = []
    lines = []
    leaf_points = []

    with open(filename, 'r') as file:
        for line in file:
            parts = line.strip().split(' ')
            if parts[0] == 'P':  # Point
                x, y = float(parts[1]), float(parts[2])
                if len(parts) > 3 and parts[3] == '(Leaf':
                    leaf_points.append((x, y))
                else:
                    points.append((x, y))
            elif parts[0] == 'L':  # Line
                x1, y1, x2, y2 = float(parts[1]), float(parts[2]), float(parts[3]), float(parts[4])
                lines.append(((x1, y1), (x2, y2)))

    return points, lines, leaf_points

def plot_points_and_lines(points, lines, leaf_points, color='b'):
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

    ax.set_xlabel('X')
    ax.set_ylabel('Y')
    ax.set_title('Zero Skew Tree Visualization with Manhattan Routing and Leaf Nodes')
    plt.grid(True)
    plt.show()

if __name__ == "__main__":
    z_levels = [1, 2, 3]  # Replace with the actual z-levels you have

    colors = itertools.cycle(['b', 'g', 'r', 'c', 'm', 'y', 'k'])  # Cycle through colors
    for z in z_levels:
        points, lines, leaf_points = read_points_and_lines_with_leaf_status(f'zeroskew_points_and_lines_z_{z}.txt')
        plot_points_and_lines(points, lines, leaf_points, next(colors))
