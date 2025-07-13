import re
from pathlib import Path

def clean_verilog_netlist(input_path, output_path, bitwidth=16):
    with open(input_path, "r") as f:
        lines = f.readlines()

    output_lines = []
    wire_names = set()
    additional_wires = set()
    assignments = []
    used_wires = set()

    # Track known I/O bits
    input_output_bits = {f"in_array[{i}]" for i in range(bitwidth)} | {f"out_array[{i}]" for i in range(bitwidth)}

    in_module = False
    for i, line in enumerate(lines):
        if line.strip().startswith("module bitonic_sort_inc("):
            ports = [f"in_array[{i}]" for i in range(bitwidth)] + [f"out_array[{i}]" for i in range(bitwidth)]
            output_lines.append("module bitonic_sort_inc(" + ", ".join(ports) + ");\n")
            in_module = True
        elif in_module and "input [15:0] in_array;" in line:
            output_lines.append("input " + ", ".join([f"in_array[{i}]" for i in range(bitwidth)]) + ";\n")
        elif in_module and "output [15:0] out_array;" in line:
            output_lines.append("output " + ", ".join([f"out_array[{i}]" for i in range(bitwidth)]) + ";\n")
        elif line.strip().startswith("wire "):
            wire_decls = line.strip()[5:].rstrip(";").split(",")
            wire_names.update(w.strip() for w in wire_decls)
        elif re.match(r'^(IV|AND|XOR|XNOR)\s', line.strip()):
            gate_type = line.strip().split()[0]
            gate_lines = lines[i:i+5]  # Capture the block
            a = b = z = None
            for gl in gate_lines:
                if ".A(" in gl:
                    a = gl.strip().split(".A(")[-1].split(")")[0]
                elif ".B(" in gl:
                    b = gl.strip().split(".B(")[-1].split(")")[0]
                elif ".Z(" in gl:
                    z = gl.strip().split(".Z(")[-1].split(")")[0]
            if gate_type == "IV":
                assignments.append(f"assign {z} = ~{a};")
                used_wires.update([z, a])
            elif gate_type == "AND":
                assignments.append(f"assign {z} = {a} & {b};")
                used_wires.update([z, a, b])
            elif gate_type == "XOR":
                assignments.append(f"assign {z} = {a} ^ {b};")
                used_wires.update([z, a, b])
            elif gate_type == "XNOR":
                temp = f"{z}_neg"
                assignments.append(f"assign {temp} = {a} ^ {b};")
                assignments.append(f"assign {z} = ~{temp};")
                used_wires.update([z, a, b, temp])
                additional_wires.add(temp)
        elif line.strip().startswith("endmodule"):
            # Emit flattened wire declaration
            all_wires = sorted((wire_names | additional_wires | used_wires) - input_output_bits)
            if all_wires:
                output_lines.append("wire " + ", ".join(all_wires) + ";\n")
            output_lines += [a + "\n" for a in assignments]
            output_lines.append("endmodule\n")
            break
        else:
            continue

    with open(output_path, "w") as f:
        f.writelines(output_lines)

# Example usage
if __name__ == "__main__":
    input_file = "bitonic_sort_inc_syn_yos.v"
    output_file = "bitonic_sort_inc_cleaned_final.v"
    clean_verilog_netlist(input_file, output_file)
    print(f"Cleaned Verilog written to: {output_file}")

