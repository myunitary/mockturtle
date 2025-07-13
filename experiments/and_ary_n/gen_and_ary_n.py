import sys

def generate_and_tree( n ):
  filename = f"and_ary_{n}.v"
  with open( filename, 'w' ) as f:
    f.write( f"module and_ary_{n}( " +
            ', '.join( [f"a{i}" for i in range( n )] + [f"b{i}" for i in range( n )] + ["d0"] ) + " );\n" )

    f.write( "  input " + ', '.join( [f"a{i}" for i in range( n )] ) + ";\n" )
    f.write( "  input " + ', '.join( [f"b{i}" for i in range( n )] ) + ";\n" )
    f.write( "  output d0;\n" )

    num_total_ands = 2 * n - 1
    num_wires = num_total_ands - 1  # d0 is final output
    if num_wires > 0:
      f.write( "  wire " + ', '.join([f"c{i}" for i in range( num_wires )]) + ";\n" )

      # First layer: c0 to c{n-1} = ai & bi
      for i in range( n ):
        f.write( f"  assign c{i} = a{i} & b{i} ;\n" )

      wire_idx = n  # index for next available wire
      layer = [f"c{i}" for i in range( n )]  # outputs of previous layer

      while len( layer ) > 1:
        next_layer = []
        for i in range( 0, len( layer ), 2 ):
          if i + 1 < len( layer ):
            lhs = f"c{wire_idx}" if wire_idx < num_total_ands - 1 else "d0"
            f.write( f"  assign {lhs} = {layer[i]} & {layer[i+1]} ;\n" )
            next_layer.append( lhs )
            wire_idx += 1
          else:
            # Odd one out propagates to next layer
            next_layer.append( layer[i] )
        layer = next_layer
    f.write( "endmodule\n" )

# Usage: python generate_and_ary_tree.py 4
if __name__ == "__main__":
  if len( sys.argv ) != 2:
    print( "Usage: python3 gen_and_ary_tree.py <n>" )
  else:
    try:
      n = int( sys.argv[1] )
      assert n >= 1
      generate_and_tree( n )
      print( f"Verilog file 'and_ary_{n}.v' has been generated." )
    except Exception as e:
      print( f"Error: {e}" )
