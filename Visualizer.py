import matplotlib.pyplot as plt


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
                x1, y1, x2, y2 = float(parts[1]), float(parts[2]), float(
                    parts[3]), float(parts[4])
                lines.append(((x1, y1), (x2, y2)))

    return points, lines


def plot_points_and_lines(points, lines):
    fig, ax = plt.subplots()

    # Plot points
    for (x, y) in points:
        ax.plot(x, y, 'bo')  # Blue dot for each point

    # Plot Manhattan-routed lines
    for ((x1, y1), (x2, y2)) in lines:
        if x1 == x2 or y1 == y2:
            # If the segment is already a straight line
            ax.plot([x1, x2], [y1, y2], 'b-')
        else:
            # Draw Manhattan route
            # You can choose different routes. Here we go horizontally, then vertically
            ax.plot([x1, x1, x2], [y1, y2, y2], 'b-')

    ax.set_xlabel('X')
    ax.set_ylabel('Y')
    ax.set_title('Zero Skew Tree Visualization with Manhattan Routing')
    plt.grid(True)
    plt.show()


if __name__ == "__main__":
    z_levels = [1, 2, 3]  # Replace with the actual z-levels you have
    for z in z_levels:
        points, lines = read_points_and_lines(
            f'zeroskew_points_and_lines_z_{z}.txt')
        plot_points_and_lines(points, lines)
