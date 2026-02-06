import re
from pathlib import Path

def flatten_signal_name(name):
    return re.sub(r'(in_array|out_array)\[(\d+)\]', r'\1_\2', name)

def clean_verilog_netlist(input_path, output_path, bitwidth=16):
    with open(input_path, "r") as f:
        lines = f.readlines()

    output_lines = []
    wire_names = set()
    additional_wires = set()
    assignments = []
    used_wires = set()

    input_bits = [f"in_array_{i}" for i in range(bitwidth)]
    output_bits = [f"out_array_{i}" for i in range(bitwidth)]
    input_output_set = set(input_bits + output_bits)

    in_module = False
    i = 0
    while i < len(lines):
        line = lines[i]
        if line.strip().startswith("module bitonic_sort_inc("):
            ports = input_bits + output_bits
            output_lines.append("module bitonic_sort_inc(" + ", ".join(ports) + ");\n")
            in_module = True
            i += 1
        elif in_module and re.match(r'\s*input\s*\[\s*\d+\s*:\s*\d+\s*\]\s*in_array\s*;', line):
            output_lines.append("input " + ", ".join(input_bits) + ";\n")
            i += 1
        elif in_module and re.match(r'\s*output\s*\[\s*\d+\s*:\s*\d+\s*\]\s*out_array\s*;', line):
            output_lines.append("output " + ", ".join(output_bits) + ";\n")
            i += 1
        elif in_module and re.match(r'\s*wire\s*\[\s*\d+\s*:\s*\d+\s*\]\s*in_array\s*;', line):
            i += 1
        elif in_module and re.match(r'\s*wire\s*\[\s*\d+\s*:\s*\d+\s*\]\s*out_array\s*;', line):
            i += 1    
        elif line.strip().startswith("wire "):
            wire_decls = line.strip()[5:].rstrip(";").split(",")
            wire_names.update(w.strip() for w in wire_decls)
            i += 1
        elif re.match(r'^(IV|AND|OR|XOR|XNOR|NOR)\s', line.strip()):
            gate_lines = []
            while not lines[i].strip().endswith(");"):
                gate_lines.append(lines[i])
                i += 1
            gate_lines.append(lines[i])  # closing line
            i += 1

            gate_type = gate_lines[0].strip().split()[0]
            a = b = z = None
            for gl in gate_lines:
                if ".A(" in gl:
                    raw_a = gl.strip().split(".A(")[-1].split(")")[0]
                    a = flatten_signal_name(raw_a)
                elif ".B(" in gl:
                    raw_b = gl.strip().split(".B(")[-1].split(")")[0]
                    b = flatten_signal_name(raw_b)
                elif ".Z(" in gl:
                    raw_z = gl.strip().split(".Z(")[-1].split(")")[0]
                    z = flatten_signal_name(raw_z)

            if gate_type == "IV":
                assignments.append(f"assign {z} = ~{a};")
                used_wires.update([z, a])
            elif gate_type == "AND":
                assignments.append(f"assign {z} = {a} & {b};")
                used_wires.update([z, a, b])
            elif gate_type == "OR":
                assignments.append(f"assign {z} = {a} | {b};")
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
            elif gate_type == "NOR":
                temp = f"{z}_neg"
                assignments.append(f"assign {temp} = {a} | {b};")
                assignments.append(f"assign {z} = ~{temp};")
                used_wires.update([z, a, b, temp])
                additional_wires.add(temp)
        elif line.strip().startswith("endmodule"):
            all_wires = sorted((wire_names | additional_wires | used_wires) - input_output_set)
            if all_wires:
                output_lines.append("wire " + ", ".join(all_wires) + ";\n")
            output_lines += [a + "\n" for a in assignments]
            output_lines.append("endmodule\n")
            break
        else:
            i += 1

    with open(output_path, "w") as f:
        f.writelines(output_lines)

# Example usage
if __name__ == "__main__":
    input_file = "bitonic_sort_inc_syn_yos.v"
    output_file = "bitonic_sort_inc_cleaned_final.v"
    clean_verilog_netlist(input_file, output_file, 4)
    print(f"Cleaned Verilog written to: {output_file}")

