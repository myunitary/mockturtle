import sys

def generate_point_and_tree(n):
    filename = f"point_and_ary_{n}.v"
    with open(filename, 'w') as f:
        # Module header
        f.write(f"module point_and_ary_{n}( " +
                ', '.join([f"a{i}" for i in range(n)] +
                          [f"b{i}" for i in range(n)] +
                          ["d0"]) + " );\n")

        f.write("  input " + ', '.join([f"a{i}" for i in range(n)]) + ";\n")
        f.write("  input " + ', '.join([f"b{i}" for i in range(n)]) + ";\n")
        f.write("  output d0;\n")

        num_total_ands = 2 * (n - 1) - 1
        num_wires = max(0, num_total_ands - 1)  # d0 is final output

        # Wire declarations
        f.write("  wire " + ', '.join([f"e{i}" for i in range(n)]))
        if num_wires > 0:
            f.write(',' + ', '.join([f"c{i}" for i in range(num_wires)]) + ";\n")
        else:
            f.write(";\n")

        # First layer: XORs
        for i in range(n):
            f.write(f"  assign e{i} = a{i} ^ b{i};\n")

        if num_wires > 0:
            # Second layer: pairwise ANDs
            for i in range(n // 2):
                f.write(f"  assign c{i} = e{2*i} & e{2*i + 1};\n")

            wire_idx = n // 2
            layer = [f"c{i}" for i in range(n // 2)]
            if n % 2 == 1:
                # carry forward last e[n-1]
                layer.append(f"e{n-1}")

            while len(layer) > 1:
                next_layer = []
                for i in range(0, len(layer), 2):
                    if i + 1 < len(layer):
                        # last AND in the chain should drive d0
                        lhs = "d0" if len(layer) == 2 else f"c{wire_idx}"
                        f.write(f"  assign {lhs} = {layer[i]} & {layer[i+1]};\n")
                        next_layer.append(lhs)
                        wire_idx += 1
                    else:
                        next_layer.append(layer[i])  # propagate odd one
                layer = next_layer
        else:
            f.write("  assign d0 = e0 & e1;\n")

        f.write("endmodule\n")

# Usage: python generate_point_and_ary_tree.py 4
if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 gen_point_and_ary_tree.py <n>")
    else:
        try:
            n = int(sys.argv[1])
            assert n >= 2
            generate_point_and_tree(n)
            print(f"Verilog file 'point_and_ary_{n}.v' has been generated.")
        except Exception as e:
            print(f"Error: {e}")
