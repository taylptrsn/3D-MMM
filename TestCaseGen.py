import random

def generate_unique_coordinates(num_sinks, max_x, max_y):
    x_coordinates = random.sample(range(max_x + 1), num_sinks)
    y_coordinates = random.sample(range(max_y + 1), num_sinks)
    return list(zip(x_coordinates, y_coordinates))

def generate_sinks(num_sinks, max_x, max_y, max_z):
    coordinates = generate_unique_coordinates(num_sinks, max_x, max_y)
    sinks = []
    for x, y in coordinates:
        z = random.randint(1, max_z)
        capacitance = round(random.uniform(0.1, 0.5), 2)
        sinks.append((x, y, z, capacitance))
    return sinks

# Parameters
layout_x = 1000
layout_y = 1000
num_dies = 3
wire_resistance = 0.1
wire_capacitance = 0.2
buffer_resistance = 122
buffer_input_cap = 24
buffer_delay = 17
tsv_resistance = 0.035
tsv_capacitance = 15
clock_source_x = 50
clock_source_y = 0
clock_source_z = 1
clock_source_resistance = 100
num_sinks = 305  # Changed to 101 to ensure it's less than or equal to min(layout_x, layout_y)

# Generate sinks
sinks = generate_sinks(num_sinks, layout_x, layout_y, num_dies)

# Print the output
print(f"{layout_x} {layout_y} {num_dies} // the layout area ((0,0) to ({layout_x}um,{layout_y}um)), # dies ({num_dies})")
print(f"{wire_resistance} {wire_capacitance} // unit wire resistance (ohm/um), unit wire capacitance (fF/um)")
print(f"{buffer_resistance} {buffer_input_cap} {buffer_delay} // Buffer: output resistance (ohm), input cap. (fF), intrinsic delay (ps)")
print(f"{tsv_resistance} {tsv_capacitance} // TSV (or MIV): resistance (ohm), capacitance (fF)")
print(f"{clock_source_x} {clock_source_y} {clock_source_z} {clock_source_resistance} // the location of the clock source and its output resistance (ohm)")
print(f"{num_sinks} // # sinks")
for sink in sinks:
    print(f"{sink[0]} {sink[1]} {sink[2]} {sink[3]} // (x,y,z) and its input capacitance (fF) of sink")
